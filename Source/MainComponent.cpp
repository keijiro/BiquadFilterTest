#include "../JuceLibraryCode/JuceHeader.h"
#include "BiquadFilter.h"

class MainContentComponent : public AudioAppComponent
{
public:
    MainContentComponent()
    {
        filterTypeLabel.setText("Filter Type", dontSendNotification);
        filterTypeBox.addItem("Low-pass", 1);
        filterTypeBox.addItem("Band-pass", 2);
        filterTypeBox.addItem("High-pass", 3);
        filterTypeBox.setSelectedId(1);

        addAndMakeVisible(filterTypeLabel);
        addAndMakeVisible(filterTypeBox);

        cutoffFreqLabel.setText("Cutoff Frequency", dontSendNotification);
        cutoffFreqSlider.setRange(10, 4000);
        cutoffFreqSlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 20);

        addAndMakeVisible(cutoffFreqLabel);
        addAndMakeVisible(cutoffFreqSlider);

        qualityLabel.setText("Q", dontSendNotification);
        qualitySlider.setRange(0.1f, 1.5f);
        qualitySlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 20);

        addAndMakeVisible(qualityLabel);
        addAndMakeVisible(qualitySlider);

        setSize(600, 100);
        setAudioChannels(1, 1);
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    void prepareToPlay(int /*samplesPerBlockExpected*/, double /*sampleRate*/) override
    {
    }

    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
    {
        auto device = deviceManager.getCurrentAudioDevice();

        const auto activeInputChannels = device->getActiveInputChannels();
        const auto activeOutputChannels = device->getActiveOutputChannels();
        const auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
        const auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

        if (maxInputChannels == 0 || maxOutputChannels == 0 || !activeInputChannels[0] || !activeOutputChannels[0])
        {
            bufferToFill.clearActiveBufferRegion();
        }
        else
        {
            const auto filterType = filterTypeBox.getSelectedId();
            const auto fc = static_cast<float>(cutoffFreqSlider.getValue() / device->getCurrentSampleRate());
            const auto q = static_cast<float>(qualitySlider.getValue());

            if (filterType == 1)
            {
                filter1.setLowpass(fc, q);
                filter2.setLowpass(fc, q);
            }
            else if (filterType == 2)
            {
                filter1.setBandpass(fc, q);
                filter2.setBandpass(fc, q);
            }
            else
            {
                filter1.setHighpass(fc, q);
                filter2.setHighpass(fc, q);
            }

            const auto inBuffer = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
            auto outBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);

            for (auto i = 0; i < bufferToFill.numSamples; ++i)
            {
                outBuffer[i] = filter2.feedSample(filter1.feedSample(inBuffer[i]));
            }
        }
    }

    void releaseResources() override
    {
    }

    void paint(Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        filterTypeLabel.setBounds(10, 10, 190, 20);
        filterTypeBox.setBounds(200, 10, 300, 20);

        cutoffFreqLabel.setBounds(10, 40, 190, 20);
        cutoffFreqSlider.setBounds(200, 40, getWidth() - 210, 20);

        qualityLabel.setBounds(10, 70, 190, 20);
        qualitySlider.setBounds(200, 70, getWidth() - 210, 20);
    }

private:
    BiquadFilter filter1, filter2;

    ComboBox filterTypeBox;
    Label filterTypeLabel;

    Slider cutoffFreqSlider;
    Label cutoffFreqLabel;

    Slider qualitySlider;
    Label qualityLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};

Component* createMainContentComponent() { return new MainContentComponent(); }
