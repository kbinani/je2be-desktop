#include "MainWindow.h"
#include <windows.h>

using namespace juce;

namespace je2be::gui {

std::unique_ptr<FileChooser> MainWindow::sFileChooser;

void MainWindow::QueueDeletingDirectory(File directory) {
    Thread::launch([directory]() {
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
