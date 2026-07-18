// Matrix-Simulator — Universal Device Inquiry responder for Matrix synths (macOS IAC).

#include <utility>

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "DeviceInquiry.h"

namespace
{
    constexpr const char* kKeyProfileId = "synthProfileId";
    constexpr const char* kKeyFirmware = "firmwareVersion";
    constexpr const char* kKeyMidiToId = "midiToIdentifier";
    constexpr const char* kKeyMidiFromId = "midiFromIdentifier";
    constexpr const char* kKeyMidiPortsLocalReferential = "midiPortsLocalReferential";
    constexpr const char* kKeyEnabledInputs = "enabledInputIds";
    constexpr const char* kKeyEnabledOutputs = "enabledOutputIds";
    constexpr const char* kKeyHasPortFilter = "hasPortFilter";

    enum class DeviceProfile
    {
        kMatrix1000 = 0,
        kMatrix6Provisional = 1
    };

    juce::String profileLabel(DeviceProfile profile)
    {
        switch (profile)
        {
            case DeviceProfile::kMatrix1000:
                return "Matrix-1000";
            case DeviceProfile::kMatrix6Provisional:
                return "Matrix-6/6R (provisional inquiry)";
        }
        return "Unknown";
    }

    juce::uint8 profileMemberLow(DeviceProfile profile) noexcept
    {
        return profile == DeviceProfile::kMatrix6Provisional
                   ? DeviceInquiry::kMatrix6MemberLow
                   : DeviceInquiry::kExpectedMemberLow;
    }

    juce::uint8 profileMemberHigh(DeviceProfile profile) noexcept
    {
        return profile == DeviceProfile::kMatrix6Provisional
                   ? DeviceInquiry::kMatrix6MemberHigh
                   : DeviceInquiry::kExpectedMemberHigh;
    }

    juce::String normalizeFirmwareVersion(const juce::String& raw)
    {
        auto trimmed = raw.trim();
        if (trimmed.isEmpty())
            return "1.11";

        while (trimmed.length() < 4)
            trimmed << ' ';

        return trimmed.substring(0, 4);
    }
}

/** Options panel: checkboxes to enable which MIDI ports appear in the main combos. */
class MidiPortSupportComponent final : public juce::Component
{
public:
    MidiPortSupportComponent(juce::StringArray& enabledInputIds,
                             juce::StringArray& enabledOutputIds,
                             std::function<void()> onChanged)
        : enabledInputIds_(enabledInputIds)
        , enabledOutputIds_(enabledOutputIds)
        , onChanged_(std::move(onChanged))
    {
        inputsHeading_.setText("Active MIDI inputs", juce::dontSendNotification);
        inputsHeading_.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
        addAndMakeVisible(inputsHeading_);

        outputsHeading_.setText("Active MIDI outputs", juce::dontSendNotification);
        outputsHeading_.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
        addAndMakeVisible(outputsHeading_);

        addAndMakeVisible(inputsViewport_);
        addAndMakeVisible(outputsViewport_);
        inputsViewport_.setViewedComponent(&inputsList_, false);
        outputsViewport_.setViewedComponent(&outputsList_, false);

        rebuild();
        setSize(480, 420);
    }

    void rebuild()
    {
        inputToggles_.clear();
        outputToggles_.clear();

        const auto inputs = juce::MidiInput::getAvailableDevices();
        const auto outputs = juce::MidiOutput::getAvailableDevices();

        int y = 0;
        for (const auto& device : inputs)
        {
            auto* toggle = inputToggles_.add(new juce::ToggleButton(device.name));
            toggle->setToggleState(enabledInputIds_.contains(device.identifier),
                                   juce::dontSendNotification);
            const auto id = device.identifier;
            toggle->onClick = [this, id, toggle]
            {
                if (toggle->getToggleState())
                    enabledInputIds_.addIfNotAlreadyThere(id);
                else
                    enabledInputIds_.removeString(id);
                if (onChanged_ != nullptr)
                    onChanged_();
            };
            inputsList_.addAndMakeVisible(toggle);
            toggle->setBounds(0, y, 440, 24);
            y += 26;
        }
        inputsList_.setSize(440, juce::jmax(y, 24));

        y = 0;
        for (const auto& device : outputs)
        {
            auto* toggle = outputToggles_.add(new juce::ToggleButton(device.name));
            toggle->setToggleState(enabledOutputIds_.contains(device.identifier),
                                   juce::dontSendNotification);
            const auto id = device.identifier;
            toggle->onClick = [this, id, toggle]
            {
                if (toggle->getToggleState())
                    enabledOutputIds_.addIfNotAlreadyThere(id);
                else
                    enabledOutputIds_.removeString(id);
                if (onChanged_ != nullptr)
                    onChanged_();
            };
            outputsList_.addAndMakeVisible(toggle);
            toggle->setBounds(0, y, 440, 24);
            y += 26;
        }
        outputsList_.setSize(440, juce::jmax(y, 24));
    }

    void resized() override
    {
        constexpr int kEm = 14;
        constexpr int kGapBeforeOutputs = 2 * kEm; // 2em

        auto area = getLocalBounds().reduced(12);
        inputsHeading_.setBounds(area.removeFromTop(22));
        area.removeFromTop(4);

        const int outputsHeaderBlock = 22 + 4;
        const int minOutputsView = 80;
        const int reservedBelow = kGapBeforeOutputs + outputsHeaderBlock + minOutputsView;
        const int inputsViewH = juce::jlimit(40,
                                             juce::jmax(40, area.getHeight() - reservedBelow),
                                             inputsList_.getHeight());
        inputsViewport_.setBounds(area.removeFromTop(inputsViewH));
        area.removeFromTop(kGapBeforeOutputs);
        outputsHeading_.setBounds(area.removeFromTop(22));
        area.removeFromTop(4);
        outputsViewport_.setBounds(area);
    }

private:
    juce::StringArray& enabledInputIds_;
    juce::StringArray& enabledOutputIds_;
    std::function<void()> onChanged_;

    juce::Label inputsHeading_;
    juce::Label outputsHeading_;
    juce::Viewport inputsViewport_;
    juce::Viewport outputsViewport_;
    juce::Component inputsList_;
    juce::Component outputsList_;
    juce::OwnedArray<juce::ToggleButton> inputToggles_;
    juce::OwnedArray<juce::ToggleButton> outputToggles_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiPortSupportComponent)
};

class SimulatorMainComponent final : public juce::Component,
                                     private juce::MidiInputCallback,
                                     private juce::ComboBox::Listener,
                                     private juce::TextEditor::Listener,
                                     private juce::Timer
{
public:
    explicit SimulatorMainComponent(juce::ApplicationProperties& appProperties)
        : appProperties_(appProperties)
    {
        profileLabel_.setText("Synth Profile", juce::dontSendNotification);
        profileLabel_.attachToComponent(&profileBox_, false);
        addAndMakeVisible(profileLabel_);

        profileBox_.addItem("Matrix-1000", 1);
        profileBox_.addItem("Matrix-6/6R (provisional inquiry)", 2);
        profileBox_.setSelectedId(1, juce::dontSendNotification);
        profileBox_.addListener(this);
        addAndMakeVisible(profileBox_);

        versionLabel_.setText("Firmware (4 chars)", juce::dontSendNotification);
        versionLabel_.attachToComponent(&versionEditor_, false);
        addAndMakeVisible(versionLabel_);

        versionEditor_.setText("1.11", juce::dontSendNotification);
        versionEditor_.setInputRestrictions(4);
        versionEditor_.addListener(this);
        addAndMakeVisible(versionEditor_);

        midiFromLabel_.setText("MIDI From", juce::dontSendNotification);
        midiFromLabel_.attachToComponent(&midiFromBox_, false);
        addAndMakeVisible(midiFromLabel_);
        midiFromBox_.addListener(this);
        addAndMakeVisible(midiFromBox_);

        midiToLabel_.setText("MIDI To", juce::dontSendNotification);
        midiToLabel_.attachToComponent(&midiToBox_, false);
        addAndMakeVisible(midiToLabel_);
        midiToBox_.addListener(this);
        addAndMakeVisible(midiToBox_);

        refreshButton_.setButtonText("Refresh Ports");
        refreshButton_.onClick = [this] { refreshPortLists(); };
        addAndMakeVisible(refreshButton_);

        clearLogsButton_.setButtonText("Clear Logs");
        clearLogsButton_.onClick = [this] { logEditor_.clear(); };
        addAndMakeVisible(clearLogsButton_);

        logLabel_.setText("Status", juce::dontSendNotification);
        addAndMakeVisible(logLabel_);

        logEditor_.setMultiLine(true);
        logEditor_.setReadOnly(true);
        logEditor_.setCaretVisible(false);
        addAndMakeVisible(logEditor_);

        loadSettings();
        refreshPortLists(true);
        appendLog("Ready.");
        startTimer(2000);
    }

    ~SimulatorMainComponent() override
    {
        stopTimer();
        saveSettings();
        closePorts();
    }

    void showMidiPortFilter()
    {
        auto* panel = new MidiPortSupportComponent(
            enabledInputIds_,
            enabledOutputIds_,
            [safeThis = juce::Component::SafePointer<SimulatorMainComponent>(this)]
            {
                if (safeThis == nullptr)
                    return;
                safeThis->hasPortFilter_ = true;
                safeThis->refreshPortLists(true);
                safeThis->saveSettings();
            });

        juce::DialogWindow::LaunchOptions options;
        options.content.setOwned(panel);
        options.dialogTitle = "Active MIDI Ports";
        options.dialogBackgroundColour = findColour(juce::ResizableWindow::backgroundColourId);
        options.escapeKeyTriggersCloseButton = true;
        options.useNativeTitleBar = false;
        options.resizable = true;
        options.launchAsync();
    }

private:
    juce::PropertiesFile* settings() const
    {
        return appProperties_.getUserSettings();
    }

    void loadSettings()
    {
        auto* props = settings();
        if (props == nullptr)
            return;

        const int profileId = props->getIntValue(kKeyProfileId, 1);
        profileBox_.setSelectedId(profileId == 2 ? 2 : 1, juce::dontSendNotification);
        profile_ = profileId == 2 ? DeviceProfile::kMatrix6Provisional
                                  : DeviceProfile::kMatrix1000;

        const auto firmware = normalizeFirmwareVersion(
            props->getValue(kKeyFirmware, "1.11"));
        versionEditor_.setText(firmware.trim(), juce::dontSendNotification);
        firmwareVersion_ = firmware;

        savedMidiToId_ = props->getValue(kKeyMidiToId);
        savedMidiFromId_ = props->getValue(kKeyMidiFromId);

        // Legacy prefs stored synth-centric IDs (From=output, To=input). Swap once to
        // the app-local referential (From=input, To=output).
        if (! props->getBoolValue(kKeyMidiPortsLocalReferential, false))
        {
            std::swap(savedMidiFromId_, savedMidiToId_);
            props->setValue(kKeyMidiPortsLocalReferential, true);
            props->setValue(kKeyMidiToId, savedMidiToId_);
            props->setValue(kKeyMidiFromId, savedMidiFromId_);
            props->saveIfNeeded();
        }

        hasPortFilter_ = props->getBoolValue(kKeyHasPortFilter, false);
        if (hasPortFilter_)
        {
            enabledInputIds_ = juce::StringArray::fromTokens(
                props->getValue(kKeyEnabledInputs), "\n", "");
            enabledOutputIds_ = juce::StringArray::fromTokens(
                props->getValue(kKeyEnabledOutputs), "\n", "");
            enabledInputIds_.removeEmptyStrings();
            enabledOutputIds_.removeEmptyStrings();
        }
        else
        {
            for (const auto& d : juce::MidiInput::getAvailableDevices())
                enabledInputIds_.addIfNotAlreadyThere(d.identifier);
            for (const auto& d : juce::MidiOutput::getAvailableDevices())
                enabledOutputIds_.addIfNotAlreadyThere(d.identifier);
        }

        for (const auto& d : juce::MidiInput::getAvailableDevices())
            knownInputIds_.addIfNotAlreadyThere(d.identifier);
        for (const auto& d : juce::MidiOutput::getAvailableDevices())
            knownOutputIds_.addIfNotAlreadyThere(d.identifier);
    }

    void saveSettings()
    {
        auto* props = settings();
        if (props == nullptr)
            return;

        props->setValue(kKeyProfileId, profileBox_.getSelectedId());
        props->setValue(kKeyFirmware, normalizeFirmwareVersion(versionEditor_.getText()).trim());
        props->setValue(kKeyMidiToId, savedMidiToId_);
        props->setValue(kKeyMidiFromId, savedMidiFromId_);
        props->setValue(kKeyMidiPortsLocalReferential, true);
        props->setValue(kKeyHasPortFilter, hasPortFilter_);
        props->setValue(kKeyEnabledInputs, enabledInputIds_.joinIntoString("\n"));
        props->setValue(kKeyEnabledOutputs, enabledOutputIds_.joinIntoString("\n"));
        props->saveIfNeeded();
    }

    void timerCallback() override
    {
        refreshPortLists(false);
    }

    void resized() override
    {
        constexpr int kLabelH = 18;
        constexpr int kFieldH = 28;
        constexpr int kGapUnderField = 14;

        auto area = getLocalBounds().reduced(12);
        area.removeFromTop(kLabelH);
        profileBox_.setBounds(area.removeFromTop(kFieldH));
        area.removeFromTop(kGapUnderField + kLabelH);
        versionEditor_.setBounds(area.removeFromTop(kFieldH));
        area.removeFromTop(kGapUnderField + kLabelH);
        midiFromBox_.setBounds(area.removeFromTop(kFieldH));
        area.removeFromTop(kGapUnderField + kLabelH);
        midiToBox_.setBounds(area.removeFromTop(kFieldH));
        area.removeFromTop(kGapUnderField);
        {
            auto row = area.removeFromTop(kFieldH);
            refreshButton_.setBounds(row.removeFromLeft(140));
            row.removeFromLeft(8);
            clearLogsButton_.setBounds(row.removeFromLeft(120));
        }
        area.removeFromTop(12);
        logLabel_.setBounds(area.removeFromTop(20));
        logEditor_.setBounds(area);
    }

    void comboBoxChanged(juce::ComboBox* box) override
    {
        if (box == &profileBox_)
        {
            const auto profile = profileBox_.getSelectedId() == 2
                                     ? DeviceProfile::kMatrix6Provisional
                                     : DeviceProfile::kMatrix1000;
            {
                const juce::ScopedLock lock(stateLock_);
                profile_ = profile;
            }
            appendLog("Synth Profile set to " + profileLabel(profile));
            saveSettings();
            return;
        }

        if (box == &midiToBox_ || box == &midiFromBox_)
        {
            openSelectedPorts();
            saveSettings();
        }
    }

    void textEditorTextChanged(juce::TextEditor& editor) override
    {
        if (&editor != &versionEditor_)
            return;

        const auto version = normalizeFirmwareVersion(versionEditor_.getText());
        const juce::ScopedLock lock(stateLock_);
        firmwareVersion_ = version;
    }

    void textEditorReturnKeyPressed(juce::TextEditor& editor) override
    {
        textEditorFocusLost(editor);
    }

    void textEditorFocusLost(juce::TextEditor& editor) override
    {
        if (&editor != &versionEditor_)
            return;

        const auto version = normalizeFirmwareVersion(versionEditor_.getText());
        versionEditor_.setText(version, juce::dontSendNotification);
        {
            const juce::ScopedLock lock(stateLock_);
            firmwareVersion_ = version;
        }
        appendLog("Firmware version set to \"" + version.trim() + "\"");
        saveSettings();
    }

    void refreshPortLists(bool reopen = true)
    {
        midiToBox_.clear(juce::dontSendNotification);
        midiFromBox_.clear(juce::dontSendNotification);
        midiToBox_.addItem("(none)", 1);
        midiFromBox_.addItem("(none)", 1);

        allInputs_ = juce::MidiInput::getAvailableDevices();
        allOutputs_ = juce::MidiOutput::getAvailableDevices();

        for (const auto& d : allInputs_)
            if (! knownInputIds_.contains(d.identifier))
            {
                knownInputIds_.add(d.identifier);
                if (! hasPortFilter_)
                    enabledInputIds_.addIfNotAlreadyThere(d.identifier);
            }
        for (const auto& d : allOutputs_)
            if (! knownOutputIds_.contains(d.identifier))
            {
                knownOutputIds_.add(d.identifier);
                if (! hasPortFilter_)
                    enabledOutputIds_.addIfNotAlreadyThere(d.identifier);
            }

        int nextFromId = 2;
        filteredInputs_.clear();
        for (const auto& device : allInputs_)
        {
            if (! enabledInputIds_.contains(device.identifier))
                continue;
            filteredInputs_.add(device);
            midiFromBox_.addItem(device.name, nextFromId++);
        }

        int nextToId = 2;
        filteredOutputs_.clear();
        for (const auto& device : allOutputs_)
        {
            if (! enabledOutputIds_.contains(device.identifier))
                continue;
            filteredOutputs_.add(device);
            midiToBox_.addItem(device.name, nextToId++);
        }

        selectByIdentifier(midiFromBox_, filteredInputs_, savedMidiFromId_);
        selectByIdentifier(midiToBox_, filteredOutputs_, savedMidiToId_);

        if (reopen)
        {
            openSelectedPorts();
            return;
        }

        // Device disappeared from the OS list: UI shows (none) but ports may still be open.
        const bool uiWantsInput = midiFromBox_.getSelectedId() > 1;
        const bool uiWantsOutput = midiToBox_.getSelectedId() > 1;
        const bool inputOpen = midiInput_ != nullptr;
        const bool outputOpen = [&]
        {
            const juce::ScopedLock lock(stateLock_);
            return midiOutput_ != nullptr;
        }();

        if (inputOpen != uiWantsInput || outputOpen != uiWantsOutput)
            openSelectedPorts();
    }

    static void selectByIdentifier(juce::ComboBox& box,
                                   const juce::Array<juce::MidiDeviceInfo>& filtered,
                                   const juce::String& identifier)
    {
        if (identifier.isEmpty())
        {
            box.setSelectedId(1, juce::dontSendNotification);
            return;
        }

        for (int i = 0; i < filtered.size(); ++i)
        {
            if (filtered.getReference(i).identifier == identifier)
            {
                box.setSelectedItemIndex(i + 1, juce::dontSendNotification);
                return;
            }
        }

        box.setSelectedId(1, juce::dontSendNotification);
    }

    void openSelectedPorts()
    {
        closePorts();

        const int fromIndex = midiFromBox_.getSelectedItemIndex() - 1;
        const int toIndex = midiToBox_.getSelectedItemIndex() - 1;

        savedMidiFromId_.clear();
        savedMidiToId_.clear();

        if (fromIndex >= 0 && fromIndex < filteredInputs_.size())
        {
            const auto& device = filteredInputs_.getReference(fromIndex);
            savedMidiFromId_ = device.identifier;
            midiInput_ = juce::MidiInput::openDevice(device.identifier, this);
            if (midiInput_ != nullptr)
            {
                midiInput_->start();
                appendLog("Listening on MIDI From: " + device.name);
            }
            else
            {
                appendLog("Failed to open MIDI From: " + device.name);
            }
        }

        if (toIndex >= 0 && toIndex < filteredOutputs_.size())
        {
            const auto& device = filteredOutputs_.getReference(toIndex);
            savedMidiToId_ = device.identifier;
            auto output = juce::MidiOutput::openDevice(device.identifier);
            if (output != nullptr)
            {
                {
                    const juce::ScopedLock lock(stateLock_);
                    midiOutput_ = std::move(output);
                }
                appendLog("Replying on MIDI To: " + device.name);
            }
            else
            {
                appendLog("Failed to open MIDI To: " + device.name);
            }
        }

        const bool sameName = midiToBox_.getText() == midiFromBox_.getText()
                              && midiFromBox_.getSelectedId() > 1;
        const bool bothOpen = midiInput_ != nullptr && [&]
        {
            const juce::ScopedLock lock(stateLock_);
            return midiOutput_ != nullptr;
        }();

        if (bothOpen && sameName)
            appendLog("Warning: same display name for To and From - prefer distinct IAC endpoints.");
    }

    void closePorts()
    {
        if (midiInput_ != nullptr)
        {
            midiInput_->stop();
            midiInput_.reset();
        }

        const juce::ScopedLock lock(stateLock_);
        midiOutput_.reset();
    }

    void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage& message) override
    {
        if (!message.isSysEx())
            return;

        const juce::MemoryBlock framed(message.getRawData(),
                                       static_cast<size_t>(message.getRawDataSize()));

        const juce::Component::SafePointer<SimulatorMainComponent> safeThis(this);

        if (!DeviceInquiry::isDeviceInquiryRequest(framed))
        {
            juce::MessageManager::callAsync([safeThis, size = static_cast<int>(framed.getSize())]
            {
                if (safeThis != nullptr)
                    safeThis->appendLog("Ignored non-inquiry SysEx (" + juce::String(size) + " bytes)");
            });
            return;
        }

        DeviceProfile profile;
        juce::String version;
        bool sent = false;
        {
            const juce::ScopedLock lock(stateLock_);
            profile = profile_;
            version = firmwareVersion_;

            if (midiOutput_ == nullptr)
            {
                juce::MessageManager::callAsync([safeThis]
                {
                    if (safeThis != nullptr)
                        safeThis->appendLog("Inquiry received but MIDI To is not open");
                });
                return;
            }

            const auto reply = DeviceInquiry::encodeReply(
                profileMemberLow(profile),
                profileMemberHigh(profile),
                version);

            midiOutput_->sendMessageNow(juce::MidiMessage(reply.getData(),
                                                          static_cast<int>(reply.getSize())));
            sent = true;
        }

        if (! sent)
            return;

        const auto label = profileLabel(profile);
        juce::MessageManager::callAsync([safeThis, label]
        {
            if (safeThis != nullptr)
                safeThis->appendLog("Inquiry received -> replied as " + label);
        });
    }

    void appendLog(const juce::String& line)
    {
        const auto stamp = juce::Time::getCurrentTime().toString(false, true, true, true);
        logEditor_.moveCaretToEnd();
        logEditor_.insertTextAtCaret("[" + stamp + "] " + line + "\n");
        logEditor_.moveCaretToEnd();
    }

    juce::ApplicationProperties& appProperties_;

    juce::Label profileLabel_;
    juce::ComboBox profileBox_;
    juce::Label versionLabel_;
    juce::TextEditor versionEditor_;
    juce::Label midiToLabel_;
    juce::ComboBox midiToBox_;
    juce::Label midiFromLabel_;
    juce::ComboBox midiFromBox_;
    juce::TextButton refreshButton_;
    juce::TextButton clearLogsButton_;
    juce::Label logLabel_;
    juce::TextEditor logEditor_;

    juce::Array<juce::MidiDeviceInfo> allInputs_;
    juce::Array<juce::MidiDeviceInfo> allOutputs_;
    juce::Array<juce::MidiDeviceInfo> filteredInputs_;
    juce::Array<juce::MidiDeviceInfo> filteredOutputs_;
    juce::StringArray enabledInputIds_;
    juce::StringArray enabledOutputIds_;
    juce::StringArray knownInputIds_;
    juce::StringArray knownOutputIds_;
    juce::String savedMidiToId_;
    juce::String savedMidiFromId_;
    bool hasPortFilter_ = false;
    std::unique_ptr<juce::MidiInput> midiInput_;

    juce::CriticalSection stateLock_;
    DeviceProfile profile_ { DeviceProfile::kMatrix1000 };
    juce::String firmwareVersion_ { "1.11" };
    std::unique_ptr<juce::MidiOutput> midiOutput_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimulatorMainComponent)
};

class SimulatorApplication final : public juce::JUCEApplication
{
public:
    SimulatorApplication()
    {
        juce::PropertiesFile::Options options;
        options.applicationName = "Matrix-Simulator";
        options.filenameSuffix = ".settings";
        options.osxLibrarySubFolder = "Application Support";
        options.folderName = "Ten Square Software";
        appProperties_.setStorageParameters(options);
    }

    const juce::String getApplicationName() override { return "Matrix-Simulator"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow_ = std::make_unique<MainWindow>(getApplicationName(), appProperties_);
    }

    void shutdown() override
    {
        mainWindow_.reset();
        if (auto* props = appProperties_.getUserSettings())
            props->saveIfNeeded();
    }

    void systemRequestedQuit() override { quit(); }

private:
    class MainWindow final : public juce::DocumentWindow,
                             private juce::MenuBarModel
    {
    public:
        MainWindow(const juce::String& name, juce::ApplicationProperties& appProperties)
            : DocumentWindow(name,
                             juce::Desktop::getInstance().getDefaultLookAndFeel()
                                 .findColour(juce::ResizableWindow::backgroundColourId),
                             DocumentWindow::closeButton | DocumentWindow::minimiseButton)
        {
            constexpr int kWindowW = 500;
            constexpr int kWindowMinH = 524;
            constexpr int kWindowMaxH = 4096;

            setUsingNativeTitleBar(false);
            setMenuBar(this);

            auto* content = new SimulatorMainComponent(appProperties);
            contentComponent_ = content;
            setContentOwned(content, true);
            // Corner grip + equal min/max width → vertical resize only; Status log fills leftover height.
            setResizable(true, true);
            setResizeLimits(kWindowW, kWindowMinH, kWindowW, kWindowMaxH);
            setSize(kWindowW, kWindowMinH);
            centreWithSize(kWindowW, kWindowMinH);
            setVisible(true);
            toFront(true);
        }

        ~MainWindow() override { setMenuBar(nullptr); }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        juce::StringArray getMenuBarNames() override { return { "Options" }; }

        juce::PopupMenu getMenuForIndex(int menuIndex, const juce::String&) override
        {
            juce::PopupMenu menu;
            if (menuIndex == 0)
                menu.addItem(1, "Active MIDI Ports...");
            return menu;
        }

        void menuItemSelected(int menuItemID, int) override
        {
            if (menuItemID == 1 && contentComponent_ != nullptr)
                contentComponent_->showMidiPortFilter();
        }

        SimulatorMainComponent* contentComponent_ = nullptr;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    juce::ApplicationProperties appProperties_;
    std::unique_ptr<MainWindow> mainWindow_;
};

START_JUCE_APPLICATION(SimulatorApplication)
