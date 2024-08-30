#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop {

class LookAndFeel : public juce::LookAndFeel_V4 {
public:
  LookAndFeel() {}

  void drawProgressBar(juce::Graphics &g, juce::ProgressBar &progressBar, int width, int height, double progress, const juce::String &textToShow) override {
    using namespace juce;
    if (width == height) {
      drawCircularProgressBar(g, progressBar, textToShow);
    } else {
      String progressText;
      if (progress >= 0 && progress <= 1.0) {
        progressText << roundToInt(progress * 100.0) << '%';
      }
      String text;
      if (progressText == textToShow) {
        text = progressText;
      } else {
        text = textToShow;
        text << progressText;
      }
      drawLinearProgressBar(g, progressBar, width, height, progress, text);
    }
  }

  void drawLinearProgressBar(juce::Graphics &g, juce::ProgressBar &progressBar,
                             int width, int height,
                             double progress, const juce::String &textToShow) {
    using namespace juce;
    auto background = progressBar.findColour(ProgressBar::backgroundColourId);
    auto foreground = progressBar.findColour(ProgressBar::foregroundColourId);

    auto barBounds = progressBar.getLocalBounds().toFloat();

    g.setColour(background);
    g.fillRoundedRectangle(barBounds, (float)progressBar.getHeight() * 0.5f);

    if (progress >= 0.0f && progress <= 1.0f) {
      Path p;
      p.addRoundedRectangle(barBounds, (float)progressBar.getHeight() * 0.5f);
      g.reduceClipRegion(p);

      barBounds.setWidth(barBounds.getWidth() * (float)progress);
      g.setColour(foreground);
      g.fillRoundedRectangle(barBounds, (float)progressBar.getHeight() * 0.5f);
    } else {
      // spinning bar..
      g.setColour(background);

      auto stripeWidth = height * 2;
      auto position = static_cast<int>(Time::getMillisecondCounter() / 15) % stripeWidth;

      Path p;

      for (auto x = static_cast<float>(-position); x < (float)(width + stripeWidth); x += (float)stripeWidth)
        p.addQuadrilateral(x, 0.0f,
                           x + (float)stripeWidth * 0.5f, 0.0f,
                           x, static_cast<float>(height),
                           x - (float)stripeWidth * 0.5f, static_cast<float>(height));

      Image im(Image::ARGB, width, height, true);

      {
        Graphics g2(im);
        g2.setColour(foreground);
        g2.fillRoundedRectangle(barBounds, (float)progressBar.getHeight() * 0.5f);
      }

      g.setTiledImageFill(im, 0, 0, 0.85f);
      g.fillPath(p);
    }

    if (textToShow.isNotEmpty()) {
      g.setColour(Colour::contrasting(background, foreground));
      g.setFont(15.0f);

      g.drawText(textToShow, 0, 0, width, height, Justification::centred, false);
    }
  }

  void drawCircularProgressBar(juce::Graphics &g, juce::ProgressBar &progressBar, const juce::String &progressText) {
    using namespace juce;
    auto background = progressBar.findColour(ProgressBar::backgroundColourId);
    auto foreground = progressBar.findColour(ProgressBar::foregroundColourId);

    auto barBounds = progressBar.getLocalBounds().reduced(2, 2).toFloat();

    auto rotationInDegrees = static_cast<float>((Time::getMillisecondCounter() / 10) % 360);
    auto normalisedRotation = rotationInDegrees / 360.0f;

    auto rotationOffset = 22.5f;
    auto maxRotation = 315.0f;

    auto startInDegrees = rotationInDegrees;
    auto endInDegrees = startInDegrees + rotationOffset;

    if (normalisedRotation >= 0.25f && normalisedRotation < 0.5f) {
      auto rescaledRotation = (normalisedRotation * 4.0f) - 1.0f;
      endInDegrees = startInDegrees + rotationOffset + (maxRotation * rescaledRotation);
    } else if (normalisedRotation >= 0.5f && normalisedRotation <= 1.0f) {
      endInDegrees = startInDegrees + rotationOffset + maxRotation;
      auto rescaledRotation = 1.0f - ((normalisedRotation * 2.0f) - 1.0f);
      startInDegrees = endInDegrees - rotationOffset - (maxRotation * rescaledRotation);
    }

    g.setColour(background);
    Path arcPath2;
    arcPath2.addCentredArc(barBounds.getCentreX(),
                           barBounds.getCentreY(),
                           barBounds.getWidth() * 0.5f,
                           barBounds.getHeight() * 0.5f, 0.0f,
                           0.0f,
                           MathConstants<float>::twoPi,
                           true);
    g.strokePath(arcPath2, PathStrokeType(4.0f));

    g.setColour(foreground);
    Path arcPath;
    arcPath.addCentredArc(barBounds.getCentreX(),
                          barBounds.getCentreY(),
                          barBounds.getWidth() * 0.5f,
                          barBounds.getHeight() * 0.5f,
                          0.0f,
                          degreesToRadians(startInDegrees),
                          degreesToRadians(endInDegrees),
                          true);

    arcPath.applyTransform(AffineTransform::rotation(normalisedRotation * MathConstants<float>::pi * 2.25f, barBounds.getCentreX(), barBounds.getCentreY()));
    g.strokePath(arcPath, PathStrokeType(4.0f));

    if (progressText.isNotEmpty()) {
      g.setColour(progressBar.findColour(TextButton::textColourOffId));
      g.setFont({12.0f, Font::italic});
      g.drawText(progressText, barBounds, Justification::centred, false);
    }
  }
};

} // namespace je2be::desktop
