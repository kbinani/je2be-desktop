#include "TemporaryDirectory.h"

namespace je2be::desktop {

void TemporaryDirectory::QueueDeletingDirectory(juce::File directory) {
  juce::Thread::launch([directory]() {
    juce::Logger::outputDebugString(juce::String("Deleting \"") + directory.getFullPathName() + juce::String("\" ...\n"));
    directory.deleteRecursively();
    juce::Logger::outputDebugString(juce::String("Done: \"") + directory.getFullPathName() + juce::String("\" deleted\n"));
  });
}

} // namespace je2be::desktop
