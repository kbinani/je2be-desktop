#include <juce_gui_extra/juce_gui_extra.h>

#include "component/ConvertProgress.h"

#include "Constants.h"
#include "TaskbarProgress.h"
#include "component/TextButton.h"

namespace je2be::desktop::component {

ConvertProgress::ConvertProgress() {
  setSize(kWindowWidth, kWindowHeight);
}

ConvertProgress::~ConvertProgress() {
  if (fThread) {
    fThread->stopThread(-1);
  }
  fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
}

void ConvertProgress::parentHierarchyChanged() {
  if (fPrepared) {
    return;
  }
  fPrepared = true;

  juce::Rectangle<int> bounds(0, 0, kWindowWidth, kWindowHeight);
  bounds.removeFromTop(kMargin);
  bounds.removeFromLeft(kMargin);
  bounds.removeFromRight(kMargin);

  auto first = getProgressCharacteristics(0);
  fLabel.reset(new juce::Label("", first.fLabel));
  fLabel->setBounds(bounds.removeFromTop(kButtonBaseHeight));
  fLabel->setJustificationType(juce::Justification::topLeft);
  addAndMakeVisible(*fLabel);
  int errorMessageY = bounds.getY();

  int const steps = getProgressSteps();
  fProgressBars.resize(steps);
  fProgresses.resize(steps, 0);
  fProgresses[0] = -1;
  fLast.resize(steps);

  for (int i = 0; i < steps; i++) {
    auto characteristics = getProgressCharacteristics(i);
    auto &bar = fProgressBars[i];
    bounds.removeFromTop(kMargin);
    bar.reset(new juce::ProgressBar(fProgresses[i]));
    bar->setBounds(bounds.removeFromTop(kButtonBaseHeight));
    bar->setTextToDisplay(characteristics.fProgressBarLabel + ": ");
    addAndMakeVisible(*bar);
  }

  fTaskbarProgress.reset(new TaskbarProgress());

  fCancelButton.reset(new component::TextButton(TRANS("Cancel")));
  fCancelButton->setBounds(kMargin, kWindowHeight - kMargin - kButtonBaseHeight, kButtonMinWidth, kButtonBaseHeight);
  fCancelButton->onClick = [this]() {
    fCancelRequested = true;
    if (!fFailure && fThread) {
      fCancelButton->setEnabled(false);
      fThread->signalThreadShouldExit();
      fLabel->setText(TRANS("Waiting for the worker thread to finish"), juce::dontSendNotification);
    }
    onCancelButtonClicked();
  };
  addAndMakeVisible(*fCancelButton);

  fErrorMessage.reset(new juce::TextEditor());
  fErrorMessage->setBounds(kMargin, errorMessageY, kWindowWidth - 2 * kMargin, fCancelButton->getY() - kMargin - errorMessageY);
  fErrorMessage->setEnabled(false);
  fErrorMessage->setMultiLine(true);
  fErrorMessage->setColour(juce::TextEditor::backgroundColourId, findColour(juce::Label::backgroundColourId));
  addChildComponent(*fErrorMessage);

  fTaskbarProgress->setState(TaskbarProgress::State::Normal);

  startThread();
}

void ConvertProgress::handleAsyncUpdate() {
  std::deque<Queue> queue;
  {
    std::lock_guard<std::mutex> lock(fMut);
    queue.swap(fQueue);
  }
  if (fFailure || fFinished) {
    return;
  }
  for (auto const &q : queue) {
    if (std::holds_alternative<ProgressQueue>(q)) {
      auto f = std::get<ProgressQueue>(q);
      fLast[f.fStep] = f.fProgress;
      fStep = std::max(fStep, f.fStep);
    } else if (std::holds_alternative<FinishQueue>(q)) {
      fFinished = true;
    } else if (std::holds_alternative<FailureQueue>(q)) {
      auto f = std::get<FailureQueue>(q);
      if (!fFailure && !f.fStatus.ok()) {
        fFailure = f.fStatus;
      }
    }
  }
  if (fFailure) {
    fLabel->setText(TRANS("The conversion failed."), juce::dontSendNotification);
    fLabel->setColour(juce::Label::textColourId, kErrorTextColor);
    auto error = fFailure->error();
    if (error) {
      juce::String message = juce::String(JUCE_APPLICATION_NAME_STRING) + " version " + JUCE_APPLICATION_VERSION_STRING;
      message += juce::String("\nFailed:\n");
      if (!error->fWhat.empty()) {
        message += juce::String("  what: " + error->fWhat + "\n");
      }
      message += juce::String("  trace: \n");
      for (int i = error->fTrace.size() - 1; i >= 0; i--) {
        auto const &trace = error->fTrace[i];
        message += juce::String("    " + trace.fFile + ":" + std::to_string(trace.fLine) + "\n");
      }
      fErrorMessage->setText(message.trimEnd());
      fErrorMessage->setVisible(true);
    }
    fCancelButton->setButtonText(TRANS("Back"));
    fCancelButton->setEnabled(true);
    for (auto &bar : fProgressBars) {
      bar->setVisible(false);
    }
    fTaskbarProgress->setState(TaskbarProgress::State::Error);
  } else if (fFinished) {
    for (size_t i = 0; i < fProgresses.size(); i++) {
      fProgresses[i] = 1;
    }
    fTaskbarProgress->setState(TaskbarProgress::State::NoProgress);
    onFinish();
  } else if (!fCancelRequested) {
    int steps = getProgressSteps();

    double progress = 0;
    for (int i = 0; i < fStep; i++) {
      fProgresses[i] = 1;
      auto ch = getProgressCharacteristics(i);
      progress += ch.fProgressWeight;
    }
    auto ch = getProgressCharacteristics(fStep);
    fProgresses[fStep] = fLast[fStep].fProgress;
    progress += fLast[fStep].fProgress * ch.fProgressWeight;
    if (fStep + 1 < steps) {
      if (fLast[fStep].fProgress == 1) {
        fProgresses[fStep + 1] = -1;
      } else {
        fProgresses[fStep + 1] = 0;
      }
    }
    for (int i = fStep + 2; i < steps; i++) {
      fProgresses[i] = 0;
    }

    fLabel->setText(ch.fLabel, juce::dontSendNotification);
    for (int i = 0; i < steps; i++) {
      auto c = getProgressCharacteristics(i);
      if (c.fUnit == Characteristics::Unit::Chunk) {
        fProgressBars[i]->setTextToDisplay(juce::String::formatted(c.fProgressBarExtraLabelFormat, fLast[i].fCount));
      }
    }
    fTaskbarProgress->update(progress);
  }
}

void ConvertProgress::notifyProgress(int step, Progress progress) {
  ProgressQueue q;
  q.fStep = step;
  q.fProgress = progress;
  {
    std::lock_guard<std::mutex> lock(fMut);
    fQueue.push_back(q);
  }
  triggerAsyncUpdate();
}

void ConvertProgress::notifyError(Status st) {
  FailureQueue q;
  q.fStatus = st;
  {
    std::lock_guard<std::mutex> lock(fMut);
    fQueue.push_back(q);
  }
  triggerAsyncUpdate();
}

void ConvertProgress::notifyFinished() {
  FinishQueue q;
  {
    std::lock_guard<std::mutex> lock(fMut);
    fQueue.push_back(q);
  }
  triggerAsyncUpdate();
}

} // namespace je2be::desktop::component
