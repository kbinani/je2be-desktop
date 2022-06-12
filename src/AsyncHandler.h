#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace je2be::desktop {

template <class Q>
class AsyncHandler : juce::AsyncUpdater {
public:
  explicit AsyncHandler(std::function<void(Q)> receiver) : fReceiver(receiver) {}

  void trigger(Q q) {
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
    if (!fReceiver) {
      return;
    }
    for (auto const &q : copy) {
      fReceiver(q);
    }
  }

private:
  std::function<void(Q)> fReceiver;
  std::deque<Q> fQueue;
  std::mutex fMut;
};

} // namespace je2be::desktop
