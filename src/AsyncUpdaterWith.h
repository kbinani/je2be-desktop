#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop {

template <class Q>
class AsyncUpdaterWith : juce::AsyncUpdater {
public:
  void triggerAsyncUpdateWith(Q q) {
    {
      std::lock_guard<std::mutex> lk(fMut);
      fQueue.push_back(q);
    }
    triggerAsyncUpdate();
  }

  void handleAsyncUpdate() override {
    std::deque<Q> copy;
    {
      std::lock_guard<std::mutex> lk(fMut);
      copy.swap(fQueue);
    }
    for (auto const &q : copy) {
      handleAsyncUpdateWith(q);
    }
  }

  virtual void handleAsyncUpdateWith(Q q) = 0;

private:
  std::deque<Q> fQueue;
  std::mutex fMut;
};

} // namespace je2be::desktop
