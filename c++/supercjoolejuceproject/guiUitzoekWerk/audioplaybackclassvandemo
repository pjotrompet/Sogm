class AudioPlaybackDemo  : public Component,
                           private FileBrowserListener,
                           private Button::Listener,
                           private Slider::Listener,
                           private ChangeListener
{
public:
    AudioPlaybackDemo()
        : deviceManager (MainAppWindow::getSharedAudioDeviceManager()),
          thread ("audio file preview"),
          directoryList (nullptr, thread),
          fileTreeComp (directoryList)
    {
        addAndMakeVisible (zoomLabel);
        zoomLabel.setText ("zoom:", dontSendNotification);
        zoomLabel.setFont (Font (15.00f, Font::plain));
        zoomLabel.setJustificationType (Justification::centredRight);
        zoomLabel.setEditable (false, false, false);
        zoomLabel.setColour (TextEditor::textColourId, Colours::black);
        zoomLabel.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (followTransportButton);
        followTransportButton.setButtonText ("Follow Transport");
        followTransportButton.addListener (this);

        addAndMakeVisible (explanation);
        explanation.setText ("Select an audio file in the treeview above, and this page will display its waveform, and let you play it..", dontSendNotification);
        explanation.setFont (Font (14.00f, Font::plain));
        explanation.setJustificationType (Justification::bottomRight);
        explanation.setEditable (false, false, false);
        explanation.setColour (TextEditor::textColourId, Colours::black);
        explanation.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (zoomSlider);
        zoomSlider.setRange (0, 1, 0);
        zoomSlider.setSliderStyle (Slider::LinearHorizontal);
        zoomSlider.setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
        zoomSlider.addListener (this);
        zoomSlider.setSkewFactor (2);

        addAndMakeVisible (thumbnail = new DemoThumbnailComp (formatManager, transportSource, zoomSlider));
        thumbnail->addChangeListener (this);

        addAndMakeVisible (startStopButton);
        startStopButton.setButtonText ("Play/Stop");
        startStopButton.addListener (this);
        startStopButton.setColour (TextButton::buttonColourId, Colour (0xff79ed7f));

        addAndMakeVisible (fileTreeComp);

        // audio setup
        formatManager.registerBasicFormats();

        directoryList.setDirectory (File::getSpecialLocation (File::userHomeDirectory), true, true);
        thread.startThread (3);

        fileTreeComp.setColour (FileTreeComponent::backgroundColourId, Colours::lightgrey.withAlpha (0.6f));
        fileTreeComp.addListener (this);

        deviceManager.addAudioCallback (&audioSourcePlayer);
        audioSourcePlayer.setSource (&transportSource);

        setOpaque (true);
    }

    ~AudioPlaybackDemo()
    {
        transportSource.setSource (nullptr);
        audioSourcePlayer.setSource (nullptr);

        deviceManager.removeAudioCallback (&audioSourcePlayer);
        fileTreeComp.removeListener (this);
        thumbnail->removeChangeListener (this);
        followTransportButton.removeListener (this);
        zoomSlider.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        fillTiledBackground (g);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (4));

        Rectangle<int> controls (r.removeFromBottom (90));

        explanation.setBounds (controls.removeFromRight (controls.getWidth() / 3));
        Rectangle<int> zoom (controls.removeFromTop (25));
        zoomLabel.setBounds (zoom.removeFromLeft (50));
        zoomSlider.setBounds (zoom);
        followTransportButton.setBounds (controls.removeFromTop (25));
        startStopButton.setBounds (controls);

        r.removeFromBottom (6);
        thumbnail->setBounds (r.removeFromBottom (140));
        r.removeFromBottom (6);
        fileTreeComp.setBounds (r);
    }

private:
    AudioDeviceManager& deviceManager;
    AudioFormatManager formatManager;
    TimeSliceThread thread;
    DirectoryContentsList directoryList;

    AudioSourcePlayer audioSourcePlayer;
    AudioTransportSource transportSource;
    ScopedPointer<AudioFormatReaderSource> currentAudioFileSource;

    ScopedPointer<DemoThumbnailComp> thumbnail;
    Label zoomLabel, explanation;
    Slider zoomSlider;
    ToggleButton followTransportButton;
    TextButton startStopButton;
    FileTreeComponent fileTreeComp;


