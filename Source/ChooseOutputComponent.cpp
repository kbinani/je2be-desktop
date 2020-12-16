#include "ChooseOutputComponent.h"
#include "CommandID.h"
#include "Constants.h"
#include <JuceHeader.h>

static std::string ChunkDataVersionToString(uint32_t v) {
  using namespace std;
  static map<uint32_t, string> const versionToString = {
      {2681, "20w45a"},
      {2584, "1.16.4"},
      {2583, "1.16.4 Release Candidate 1"},
      {2582, "1.16.4 Pre-release 2"},
      {2581, "1.16.4 Pre-release 1"},
      {2580, "1.16.3"},
      {2579, "1.16.3 Release Candidate 1"},
      {2578, "1.16.2"},
      {2577, "1.16.2 Release Candidate 2"},
      {2576, "1.16.2 Release Candidate 1"},
      {2575, "1.16.2 Pre-release 3"},
      {2574, "1.16.2 Pre-release 2"},
      {2573, "1.16.2 Pre-release 1"},
      {2572, "20w30a"},
      {2571, "20w29a"},
      {2570, "20w28a"},
      {2569, "20w27a"},
      {2567, "1.16.1"},
      {2566, "1.16"},
      {2565, "1.16 Release Candidate 1"},
      {2564, "1.16 Pre-release 8"},
      {2563, "1.16 Pre-release 7"},
      {2562, "1.16 Pre-release 6"},
      {2561, "1.16 Pre-release 5"},
      {2560, "1.16 Pre-release 4"},
      {2559, "1.16 Pre-release 3"},
      {2557, "1.16 Pre-release 2"},
      {2556, "1.16 Pre-release 1"},
      {2555, "20w22a"},
      {2554, "20w21a"},
      {2537, "20w20b"},
      {2536, "20w20a"},
      {2534, "20w19a"},
      {2532, "20w18a"},
      {2529, "20w17a"},
      {2526, "20w16a"},
      {2525, "20w15a"},
      {2524, "20w14a"},
      {2521, "20w13b"},
      {2520, "20w13a"},
      {2515, "20w12a"},
      {2513, "20w11a"},
      {2512, "20w10a"},
      {2510, "20w09a"},
      {2507, "20w08a"},
      {2506, "20w07a"},
      {2504, "20w06a"},
      {2230, "1.15.2"},
      {2229, "1.15.2 Pre-release 2"},
      {2228, "1.15.2 Pre-release 1"},
      {2227, "1.15.1"},
      {2226, "1.15.1 Pre-release 1"},
      {2225, "1.15"},
      {2224, "1.15 Pre-release 7"},
      {2223, "1.15 Pre-release 6"},
      {2222, "1.15 Pre-release 5"},
      {2221, "1.15 Pre-release 4"},
      {2220, "1.15 Pre-release 3"},
      {2219, "1.15 Pre-release 2"},
      {2218, "1.15 Pre-release 1"},
      {2217, "19w46b"},
      {2216, "19w46a"},
      {2215, "19w45b"},
      {2214, "19w45a"},
      {2213, "19w44a"},
      {2212, "19w42a"},
      {2210, "19w41a"},
      {2208, "19w40a"},
      {2207, "19w39a"},
      {2206, "19w38b"},
      {2205, "19w38a"},
      {2204, "19w37a"},
      {2203, "19w36a"},
      {2201, "19w35a"},
      {2200, "19w34a"},
      {1976, "1.14.4"},
  };
  auto found = versionToString.find(v);
  if (found == versionToString.end()) {
    return std::to_string(v);
  } else {
    return found->second;
  }
}

static String StatToString(ConvertStatistics stat) {
  String msg;
  msg += "Chunk data version:\n";
  for (auto const &it : stat.fChunkDataVersions) {
    msg += "    Version " + String(ChunkDataVersionToString(it.first)) + ": " +
           String(it.second) + "\n";
  }
  msg += "Number of Chunks: " + String(stat.fNumChunks) + "\n";
  msg += "Number of BlockEntities: " + String(stat.fNumBlockEntities) + "\n";
  msg += "Number of Entities: " + String(stat.fNumEntities);
  return msg;
}

static File BedrockSaveDirectory() {
  return File::getSpecialLocation(File::userApplicationDataDirectory)
      .getParentDirectory()
      .getChildFile("Local")
      .getChildFile("Packages")
      .getChildFile("Microsoft.MinecraftUWP_8wekyb3d8bbwe")
      .getChildFile("LocalState")
      .getChildFile("games")
      .getChildFile("com.mojang")
      .getChildFile("minecraftWorlds");
}

static File DecideDefaultOutputDirectory(ConvertState const &s) {
  File root = BedrockSaveDirectory();
  String name = s.fConfigState.fInputState.fInputDirectory->getFileName();
  File candidate = root.getChildFile(name);
  int count = 0;
  while (candidate.exists()) {
    count++;
    candidate = root.getChildFile(name + "-" + String(count));
  }
  return candidate;
}

static String WillBeSavedMessage(File outputDir) {
  return TRANS("The conversion result will be saved in the following folder.") +
         "\n\n" + outputDir.getFullPathName() + "\n\n" +
         TRANS("If you want to change the destination, please select it with "
               "the button below.");
}

ChooseOutputComponent::ChooseOutputComponent(ConvertState const &convertState)
    : fState(convertState) {
  auto width = kWindowWidth;
  auto height = kWindowHeight;
  setSize(width, height);

  File root = BedrockSaveDirectory();
  File outputDir = DecideDefaultOutputDirectory(convertState);

  {
    fSaveButton.reset(new TextButton(TRANS("Save")));
    fSaveButton->setBounds(width - kMargin - kButtonMinWidth,
                           height - kMargin - kButtonBaseHeight,
                           kButtonMinWidth, kButtonBaseHeight);
    fSaveButton->onClick = [this]() { onSaveButtonClicked(); };
    addAndMakeVisible(*fSaveButton);
  }

  int y = kMargin;
  {
    int h = 96;
    String message;
    if (root.exists()) {
      fState.fCopyDestination = outputDir;
      fState.fFormat = OutputFormat::Directory;
      message = WillBeSavedMessage(outputDir);
      fSaveButton->setEnabled(true);
    } else {
      message = TRANS("Select a folder to save in.\rChoose an empty "
                      "folder to protect your existing data.");
      fSaveButton->setEnabled(false);
    }
    fMessage.reset(new Label("", message));
    fMessage->setBounds(kMargin, y, width - 2 * kMargin, h);
    fMessage->setJustificationType(Justification::topLeft);
    fMessage->setMinimumHorizontalScale(1);
    addAndMakeVisible(*fMessage);
    y += h + kMargin;
  }
  {
    fStat.reset(new TextEditor());
    fStat->setBounds(kMargin, y, width - 2 * kMargin,
                     height - 2 * kMargin - kButtonBaseHeight - y);
    fStat->setEnabled(false);
    fStat->setMultiLine(true);
    fStat->setText(StatToString(convertState.fStat));
    addAndMakeVisible(*fStat);
  }
  {
    int w = 160;
    fBackButton.reset(new TextButton(TRANS("Back to the beginning")));
    fBackButton->setBounds(kMargin, height - kMargin - kButtonBaseHeight, w,
                           kButtonBaseHeight);
    fBackButton->onClick = [this]() { onBackButtonClicked(); };
    addAndMakeVisible(*fBackButton);
  }
  {
    int w = 160;
    fBrowseButton.reset(new TextButton(TRANS("Change the destination")));
    fBrowseButton->setBounds(width - kMargin - kButtonMinWidth - kMargin - w,
                             height - kMargin - kButtonBaseHeight, w,
                             kButtonBaseHeight);
    fBrowseButton->onClick = [this]() { onBrowseButtonClicked(); };
    addAndMakeVisible(*fBrowseButton);

    fBrowseButton->setExplicitFocusOrder(1);
    fBackButton->setExplicitFocusOrder(2);
  }
}

ChooseOutputComponent::~ChooseOutputComponent() {}

void ChooseOutputComponent::onBrowseButtonClicked() {
  static File lastDir = BedrockSaveDirectory();

  FileChooser chooser(TRANS("Select an empty folder to save in"), lastDir);
  bool ok = chooser.browseForDirectory();
  if (!ok) {
    return;
  }
  File dest = chooser.getResult();
  RangedDirectoryIterator it(dest, false);
  bool containsSomething = false;
  for (auto const &e : it) {
    containsSomething = true;
    break;
  }
  if (containsSomething) {
    NativeMessageBox::showMessageBox(
        AlertWindow::AlertIconType::WarningIcon, TRANS("Error"),
        TRANS("There are files and folders in the directory.\rPlease select an "
              "empty folder"));
  } else {
    fState.fCopyDestination = dest;
    fState.fFormat = OutputFormat::Directory;
    String message = WillBeSavedMessage(dest);
    fMessage->setText(message, dontSendNotification);
    fSaveButton->setEnabled(true);
  }
}

void ChooseOutputComponent::paint(juce::Graphics &g) {}

void ChooseOutputComponent::onBackButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toChooseInput, true);
}

void ChooseOutputComponent::onSaveButtonClicked() {
  JUCEApplication::getInstance()->invoke(gui::toCopy, true);
}