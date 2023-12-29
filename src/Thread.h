#pragma once

namespace je2be::desktop {

class Thread {
  Thread() = delete;

  struct Worker : public std::enable_shared_from_this<Worker> {
    explicit Worker(std::function<void()> task) : fTask(task) {}

    void run() {
      if (!fTask) {
        return;
      }
      fTask();
      fTask = nullptr;
    }

    std::function<void()> fTask;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Worker)
  };

public:
  static void Launch(std::function<void()> task) {
    if (!task) {
      return;
    }
    auto d = std::make_shared<Worker>(task);
    auto weak = d->weak_from_this();
    auto ok = juce::Thread::launch([d]() {
      d->run();
    });
    Cleanup();
    if (ok) {
      Tasks().push_back(weak);
    }
  }

  static void Wait() {
    while (true) {
      Cleanup();
      if (Tasks().empty()) {
        break;
      }
      juce::Thread::sleep(150);
    }
  }

private:
  static std::vector<std::weak_ptr<Worker>> &Tasks() {
    static std::vector<std::weak_ptr<Worker>> sTasks;
    return sTasks;
  }

  static void Cleanup() {
    auto &tasks = Tasks();
    for (int i = (int)tasks.size() - 1; i >= 0; i--) {
      if (auto t = tasks[i].lock(); !t) {
        tasks.erase(tasks.begin() + i);
      }
    }
  }
};

} // namespace je2be::desktop
