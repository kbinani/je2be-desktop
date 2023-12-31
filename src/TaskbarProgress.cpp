#include "TaskbarProgress.h"

#include <shobjidl_core.h>

#include <optional>

using namespace juce;

namespace je2be::desktop {

namespace {
std::optional<HWND> GetTopLevelWindow() {
  int const num = juce::TopLevelWindow::getNumTopLevelWindows();
  for (int i = 0; i < num; i++) {
    if (auto window = juce::TopLevelWindow::getTopLevelWindow(i); window) {
      return (HWND)window->getWindowHandle();
    }
  }
  return std::nullopt;
}
} // namespace

class TaskbarProgress::Impl {
public:
  Impl()
      : fTaskbar(nullptr), fState(std::nullopt) {
    fTopLevelWindow = GetTopLevelWindow();
    if (!fTopLevelWindow) {
      return;
    }
    ITaskbarList3 *taskbar = nullptr;
    if (FAILED(CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (void **)&taskbar))) {
      return;
    }
    if (!taskbar) {
      return;
    }
    if (FAILED(taskbar->HrInit())) {
      taskbar->Release();
      return;
    }
    fTaskbar = taskbar;
  }

  ~Impl() {
    if (fTaskbar) {
      fTaskbar->Release();
      fTaskbar = nullptr;
    }
  }

  void setState(State state) {
    if (!fTopLevelWindow) {
      return;
    }
    if (!fTaskbar) {
      return;
    }
    if (state == fState) {
      return;
    }
    fState = state;

    TBPFLAG flag = TBPF_NOPROGRESS;
    switch (state) {
    case State::Indeterminate:
      flag = TBPF_INDETERMINATE;
      break;
    case State::NoProgress:
      flag = TBPF_NOPROGRESS;
      break;
    case State::Normal:
      flag = TBPF_NORMAL;
      break;
    case State::Error:
      flag = TBPF_ERROR;
      break;
    }
    fTaskbar->SetProgressState(*fTopLevelWindow, flag);
  }

  void update(double progress) {
    if (!fTopLevelWindow) {
      return;
    }
    if (!fTaskbar) {
      return;
    }
    double v = std::min(std::max(progress, 0.0), 1.0);
    ULONGLONG pos = static_cast<ULONGLONG>(v * std::numeric_limits<uint32_t>::max());
    ULONGLONG constexpr max = static_cast<ULONGLONG>(std::numeric_limits<uint32_t>::max());
    fTaskbar->SetProgressValue(*fTopLevelWindow, pos, max);
  }

private:
  ITaskbarList3 *fTaskbar;
  std::optional<HWND> fTopLevelWindow;
  std::optional<State> fState;
};

TaskbarProgress::TaskbarProgress()
    : fImpl(new Impl()) {
}

TaskbarProgress::~TaskbarProgress() {
}

void TaskbarProgress::setState(State state) {
  fImpl->setState(state);
}

void TaskbarProgress::update(double progress) {
  fImpl->update(progress);
}

} // namespace je2be::desktop
