#include "PresetComp.h"

namespace
{
#if JUCE_IOS
constexpr int arrowWidth = 32;
constexpr int arrowPad = 4;
#else
constexpr int arrowWidth = 20;
constexpr int arrowPad = 2;
#endif

const String presetExt = ".chowpreset";
} // namespace

PresetComp::PresetComp (ChowKick& proc, chowdsp::PresetManager& manager) : proc (proc),
                                                                           manager (manager),
                                                                           presetsLeft ("", DrawableButton::ImageOnButtonBackground),
                                                                           presetsRight ("", DrawableButton::ImageOnButtonBackground)
{
    manager.addListener (this);

    presetBox.setName ("Preset Manager");
    presetBox.setTooltip ("Use this menu to save and load plugin presets");

#if JUCE_IOS
    setColour (backgroundColourId, Colour (0x33000000));
#else
    setColour (backgroundColourId, Colour (0xFF595C6B));
#endif
    setColour (textColourId, Colours::white);

    addAndMakeVisible (presetBox);
    presetBox.setColour (ComboBox::backgroundColourId, Colours::transparentWhite);
    presetBox.setColour (ComboBox::outlineColourId, Colours::transparentWhite);
    presetBox.setJustificationType (Justification::centred);
    presetBox.setTextWhenNothingSelected ("No Preset selected...");
    presetListUpdated();

    addChildComponent (presetNameEditor);
    presetNameEditor.setColour (TextEditor::backgroundColourId, Colour (0xFF595C6B));
    presetNameEditor.setColour (TextEditor::textColourId, Colours::white);
    presetNameEditor.setColour (TextEditor::highlightColourId, Colour (0xFF8B3232));
    presetNameEditor.setColour (CaretComponent::caretColourId, Colour (0xFF8B3232));
    presetNameEditor.setFont (Font (16.0f).boldened());
    presetNameEditor.setMultiLine (false, false);
    presetNameEditor.setJustification (Justification::centred);

    auto setupButton = [=, &manager] (DrawableButton& button, Drawable* image, int presetOffset)
    {
        addAndMakeVisible (button);
        button.setImages (image, image, image);
        button.setWantsKeyboardFocus (false);
        button.setColour (ComboBox::outlineColourId, Colours::transparentBlack);
        button.setColour (TextButton::buttonColourId, Colours::transparentBlack);
        button.onClick = [=, &manager]
        {
            auto idx = presetBox.getSelectedId() + presetOffset;
            while (idx <= 0)
                idx += manager.getNumPresets();
            while (idx > manager.getNumPresets())
                idx -= manager.getNumPresets();

            presetNameEditor.setVisible (false);
            presetBox.setSelectedId (idx, sendNotification);
        };
    };

    std::unique_ptr<Drawable> leftImage (Drawable::createFromImageData (BinaryData::LeftArrow_svg, BinaryData::LeftArrow_svgSize));
    std::unique_ptr<Drawable> rightImage (Drawable::createFromImageData (BinaryData::RightArrow_svg, BinaryData::RightArrow_svgSize));

    setupButton (presetsLeft, leftImage.get(), -1);
    setupButton (presetsRight, rightImage.get(), 1);

    updatePresetBoxText();
}

PresetComp::~PresetComp()
{
    manager.removeListener (this);
}

void PresetComp::presetListUpdated()
{
    presetBox.getRootMenu()->clear();

    std::map<String, PopupMenu> presetMapItems;
    int optionID = 0;
    for (const auto& presetIDPair : manager.getPresetMap())
    {
        const auto presetID = presetIDPair.first;
        const auto& preset = presetIDPair.second;

        PopupMenu::Item presetItem { preset.getName() };
        presetItem.itemID = presetID + 1;
        presetItem.action = [=, &preset]
        {
            updatePresetBoxText();
            manager.loadPreset (preset);
        };

        presetMapItems[preset.getVendor()].addItem (presetItem);

        optionID = jmax (optionID, presetItem.itemID);
    }

    for (auto& [vendorName, menu] : presetMapItems)
        presetBox.getRootMenu()->addSubMenu (vendorName, menu);

    addPresetOptions (optionID);
}

void PresetComp::addPresetOptions (int optionID)
{
    auto menu = presetBox.getRootMenu();
    menu->addSeparator();

    PopupMenu::Item saveItem { "Save Preset" };
    saveItem.itemID = ++optionID;
    saveItem.action = [=]
    {
        updatePresetBoxText();
        saveUserPreset();
    };
    menu->addItem (saveItem);

#if ! JUCE_IOS
    PopupMenu::Item goToFolderItem { "Go to Preset folder..." };
    goToFolderItem.itemID = ++optionID;
    goToFolderItem.action = [=]
    {
        updatePresetBoxText();
        auto folder = manager.getUserPresetPath();
        if (folder.isDirectory())
            folder.startAsProcess();
        else
            chooseUserPresetFolder();
    };
    menu->addItem (goToFolderItem);

    PopupMenu::Item chooseFolderItem { "Choose Preset folder..." };
    chooseFolderItem.itemID = ++optionID;
    chooseFolderItem.action = [=]
    {
        updatePresetBoxText();
        chooseUserPresetFolder();
    };
    menu->addItem (chooseFolderItem);
#endif
}

void PresetComp::chooseUserPresetFolder()
{
    constexpr auto folderChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;
    fileChooser = std::make_shared<FileChooser> ("Choose User Preset Folder");
    MessageManager::callAsync ([=]
                               {
                                   fileChooser->launchAsync (folderChooserFlags, [=] (const FileChooser& chooser)
                                                             {
                                                                 manager.setUserPresetPath (chooser.getResult());
                                                                 waiter.signal();
                                                             });
                               });
}

void PresetComp::saveUserPreset()
{
    presetNameEditor.setVisible (true);
    presetNameEditor.toFront (true);
    presetNameEditor.setText ("MyPreset");
    presetNameEditor.setHighlightedRegion ({ 0, 10 });

    presetNameEditor.onReturnKey = [=]
    {
        presetNameEditor.setVisible (false);

        Thread::launch ([=]
                        {
                            auto presetName = presetNameEditor.getText();
                            auto presetPath = manager.getUserPresetPath();
                            if (presetPath == File() || ! presetPath.isDirectory())
                            {
                                presetPath.deleteRecursively();
                                chooseUserPresetFolder();
                                waiter.wait();

                                presetPath = manager.getUserPresetPath();
                            }

                            if (presetPath == File() || ! presetPath.isDirectory())
                                return;

                            manager.saveUserPreset (presetPath.getChildFile (presetName + presetExt));
                        });
    };
}

void PresetComp::updatePresetBoxText()
{
    auto* currentPreset = manager.getCurrentPreset();
    auto name = currentPreset == nullptr ? String() : manager.getCurrentPreset()->getName();
    if (manager.getIsDirty())
        name += "*";

    MessageManagerLock mml;
    presetBox.setText (name, dontSendNotification);
}

void PresetComp::paint (Graphics& g)
{
    constexpr auto cornerSize = 5.0f;

    presetBox.setColour (PopupMenu::ColourIds::backgroundColourId, findColour (backgroundColourId));
    g.setColour (findColour (backgroundColourId));

    g.fillRoundedRectangle (getLocalBounds().reduced (arrowWidth + arrowPad, 0).toFloat(), cornerSize);
}

void PresetComp::resized()
{
    auto b = getLocalBounds();
    presetsLeft.setBounds (b.removeFromLeft (arrowWidth));
    presetsRight.setBounds (b.removeFromRight (arrowWidth));

    Rectangle<int> presetsBound (b.reduced (arrowPad, 0));
    presetBox.setBounds (presetsBound);
    presetNameEditor.setBounds (presetsBound);
    repaint();
}
