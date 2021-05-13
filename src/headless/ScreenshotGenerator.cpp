#include "ScreenshotGenerator.h"
#include "../ChowKick.h"

ScreenshotGenerator::ScreenshotGenerator()
{
    this->commandOption = "--screenshots";
    this->argumentDescription = "--screenshots --out=[DIR]";
    this->shortDescription = "Generates screenshots for ChowMatrix documentation";
    this->longDescription = "";
    this->command = std::bind (&ScreenshotGenerator::takeScreenshots, this, std::placeholders::_1);
}

void ScreenshotGenerator::takeScreenshots (const ArgumentList& args)
{
    File outputDir = File::getCurrentWorkingDirectory();
    if (args.containsOption ("--out"))
        outputDir = args.getExistingFolderForOption ("--out");

    std::cout << "Generating screenshots... Saving to " << outputDir.getFullPathName() << std::endl;

    std::unique_ptr<AudioProcessor> plugin (createPluginFilterOfType (AudioProcessor::WrapperType::wrapperType_Standalone));
    // dynamic_cast<ChowKick*> (plugin.get())->getStateManager().getPresetManager().setPreset (3); // set to "crazy" preset
    std::unique_ptr<AudioProcessorEditor> editor (plugin->createEditorIfNeeded());

    AudioBuffer<float> buffer (2, 1024);
    buffer.clear();
    MidiBuffer midi;
    auto midiMessage = MidiMessage::noteOn (1, 64, (uint8) 127);
    MidiBuffer midiBuffer; 
    midiBuffer.addEvent (midiMessage, 0);

    plugin->prepareToPlay (48000.0, 1024);
    plugin->processBlock (buffer, midi);
    plugin->releaseResources();

    screenshotForBounds (editor.get(), editor->getLocalBounds(), outputDir, "full_gui.png");
    screenshotForBounds (editor.get(), { 11, 60, 377, 375 }, outputDir, "ShaperSection.png");
    screenshotForBounds (editor.get(), { 386, 60, 376, 375 }, outputDir, "FilterSection.png");

    plugin->editorBeingDeleted (editor.get());
}

void ScreenshotGenerator::screenshotForBounds (Component* editor, Rectangle<int> bounds, const File& dir, const String& filename)
{
    auto screenshot = editor->createComponentSnapshot (bounds);

    File pngFile = dir.getChildFile (filename);
    pngFile.deleteFile();
    pngFile.create();
    auto pngStream = pngFile.createOutputStream();

    if (pngStream->openedOk())
    {
        PNGImageFormat pngImage;
        pngImage.writeImageToStream (screenshot, *pngStream.get());
    }
}
