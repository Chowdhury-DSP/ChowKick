#include "SettingsButton.h"
#include "../ChowKick.h"
#include "CustomLNFs.h"

namespace
{
const Colour onColour = Colour (0xFFEAA92C);
const Colour offColour = Colours::white;
} // namespace

SettingsButton::SettingsButton (ChowKick& processor,
                                chowdsp::OpenGLHelper* oglHelper) : DrawableButton ("Settings", DrawableButton::ImageFitted),
                                                                    proc (processor),
                                                                    openGLHelper (oglHelper)
{
#if JUCE_MODULE_AVAILABLE_juce_opengl
    Logger::writeToLog ("Checking OpenGL availability...");
    const auto shouldUseOpenGLByDefault = openGLHelper != nullptr && openGLHelper->isOpenGLAvailable();
    Logger::writeToLog ("OpenGL is available on this system: " + String (shouldUseOpenGLByDefault ? "TRUE" : "FALSE"));
#else
    static constexpr auto shouldUseOpenGLByDefault = false;
    Logger::writeToLog ("Skipping OpenGL checks: Plugin was compiled without linking to OpenGL!");
#endif
    pluginSettings->addProperties<&SettingsButton::globalSettingChanged> ({ { openglID, shouldUseOpenGLByDefault } }, *this);
    globalSettingChanged (openglID);

    auto cog = Drawable::createFromImageData (BinaryData::cogsolid_svg, BinaryData::cogsolid_svgSize);
    setImages (cog.get());

    onClick = [this]
    { showSettingsMenu(); };
}

SettingsButton::~SettingsButton() = default;

void SettingsButton::globalSettingChanged (SettingID settingID)
{
    if (settingID != openglID)
        return;

    if (openGLHelper != nullptr)
    {
        const auto shouldUseOpenGL = pluginSettings->getProperty<bool> (openglID);
        if (shouldUseOpenGL == openGLHelper->isAttached())
            return; // no change

        Logger::writeToLog ("Using OpenGL: " + String (shouldUseOpenGL ? "TRUE" : "FALSE"));
        shouldUseOpenGL ? openGLHelper->attach() : openGLHelper->detach();
    }
}

void SettingsButton::showSettingsMenu()
{
    PopupMenu menu;

    openGLMenu (menu, 100);

    menu.addSeparator();
    menu.addItem ("View Source Code", []
                  { URL ("https://github.com/Chowdhury-DSP/ChowKick").launchInDefaultBrowser(); });
    menu.addItem ("Copy Diagnostic Info", [this]
                  { copyDiagnosticInfo(); });
    menu.addItem ("View User Manual", []
                  { URL ("https://chowdsp.com/manuals/ChowKickManual.pdf").launchInDefaultBrowser(); });

    // get top level component that is big enough
    Component* parentComp = this;
    while (true)
    {
        if (parentComp->getWidth() > 80 && parentComp->getHeight() > 100)
            break;

        parentComp = parentComp->getParentComponent();

        if (parentComp == nullptr)
        {
            jassertfalse; // unable to find component large enough!
            return;
        }
    }

    auto options = PopupMenu::Options()
                       .withParentComponent (parentComp)
                       .withPreferredPopupDirection (PopupMenu::Options::PopupDirection::upwards)
                       .withStandardItemHeight (27);
    menu.setLookAndFeel (lnfAllocator->getLookAndFeel<ComboBoxLNF>());
    menu.showMenuAsync (options);
}

void SettingsButton::openGLMenu (PopupMenu& menu, int itemID)
{
    if (openGLHelper == nullptr || ! openGLHelper->isOpenGLAvailable())
        return;

    const auto isCurrentlyOn = pluginSettings->getProperty<bool> (openglID);

    PopupMenu::Item item;
    item.itemID = ++itemID;
    item.text = "Use OpenGL";
    item.action = [this, isCurrentlyOn]
    { pluginSettings->setProperty (openglID, ! isCurrentlyOn); };
    item.colour = isCurrentlyOn ? onColour : offColour;

    menu.addItem (item);
}

void SettingsButton::copyDiagnosticInfo()
{
    Logger::writeToLog ("Copying diagnostic info...");
    SystemClipboard::copyTextToClipboard (chowdsp::PluginDiagnosticInfo::getDiagnosticsString (proc));
}

//=====================================================
SettingsButtonItem::SettingsButtonItem (foleys::MagicGUIBuilder& builder, const ValueTree& node) : foleys::GuiItem (builder, node)
{
    auto* plugin = dynamic_cast<ChowKick*> (builder.getMagicState().getProcessor());
    button = std::make_unique<SettingsButton> (*plugin, plugin->getOpenGLHelper());

    addAndMakeVisible (button.get());
}
