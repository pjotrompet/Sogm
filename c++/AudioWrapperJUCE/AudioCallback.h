/*
  ==============================================================================

    AudioCallback.h
    Created: 9 Nov 2014 3:37:44pm
    Author:  Jerre van der Hulst

  ==============================================================================
*/

#ifndef AUDIOCALLBACK_H_INCLUDED
#define AUDIOCALLBACK_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class AudioCallback : private AudioSource
{
public:
			AudioCallback();
	virtual ~AudioCallback();
	
	void startAudioCallback();
	void stopAudioCallback();
	
	
	// This needs to be implemented by the class inheriting this.
	virtual void audioCallback (float** buffer, int channels, int frames) = 0;
	
	
	
	bool isRunning() const {return running;};
	AudioDeviceManager& getDeviceManager () {return deviceManager;};
	float getSampleRate() const;
	
	
	
	
	
	
	
	
	
	
	
	
private:
	void prepareToPlay (int samplesPerBlockExpected, double sampleRate) {};
	void releaseResources () {};
	void getNextAudioBlock (const AudioSourceChannelInfo &bufferToFill);
	
	bool running;
    AudioDeviceManager deviceManager;
	AudioSourcePlayer sourcePlayer;
	
};



#endif  // AUDIOCALLBACK_H_INCLUDED
