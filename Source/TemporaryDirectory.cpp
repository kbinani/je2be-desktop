#include "TemporaryDirectory.h"
#include <windows.h>

namespace je2be::gui {

void TemporaryDirectory::QueueDeletingDirectory(juce::File directory) {
  juce::Thread::launch([directory]() {
    OutputDebugStringA("Deleting \"");
    OutputDebugStringW(directory.getFullPathName().toWideCharPointer());
    OutputDebugStringA("\" ...\n");
    directory.deleteRecursively();
    OutputDebugStringA("Done: \"");
    OutputDebugStringW(directory.getFullPathName().toWideCharPointer());
    OutputDebugStringA("\" deleted\n");
  });
}

} // namespace je2be::gui
