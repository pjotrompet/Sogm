#include "ampclass.h"

void Amplifier::setAmpfactor(float newAmpfactor)
{
	AmpFactor=newAmpfactor;
}

float Amplifier::getAmpfactor()
{
	return AmpFactor;
}

void Amplifier::process(float audioin[bufsize])
{
	Amplifier Amp;
	for(int i = 0; i<bufsize; i++)
	{
		audioin[i] = audioin[i]*Amp.getAmpfactor();

	};
};
