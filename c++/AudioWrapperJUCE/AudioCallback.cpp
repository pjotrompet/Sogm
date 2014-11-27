/*
  ==============================================================================

    AudioCallback.cpp
    Created: 9 Nov 2014 3:37:44pm
    Author:  Jerre van der Hulst

  ==============================================================================
*/

#include "AudioCallback.h"

AudioCallback::AudioCallback()
{
	running = false;
	
	sourcePlayer.setSource (this);
	deviceManager.initialise (2, 2, nullptr, true);
}

AudioCallback::~AudioCallback()
{
	sourcePlayer.setSource(nullptr);
	deviceManager.removeAudioCallback (&sourcePlayer);
	running = false;
}

void AudioCallback::startAudioCallback()
{
	if ( ! running)
	{
		deviceManager.addAudioCallback(&sourcePlayer);
		running = true;
	}
}

void AudioCallback::stopAudioCallback()
{
	if (running)
	{
		deviceManager.removeAudioCallback (&sourcePlayer);
		running = false;
	}
}


void AudioCallback::getNextAudioBlock (const AudioSourceChannelInfo &bufferToFill)
{
	audioCallback (bufferToFill.buffer->getArrayOfWritePointers(),
				   bufferToFill.buffer->getNumChannels(),
				   bufferToFill.buffer->getNumSamples());
}


float AudioCallback::getSampleRate() const
{
	return deviceManager.getCurrentAudioDevice()->getCurrentSampleRate();
}
