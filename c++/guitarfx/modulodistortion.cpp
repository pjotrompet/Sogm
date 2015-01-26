#include "modulodistortion.h"


void moddistortion::setDistorFact(float nDistorFact)
{
	DistorFact=nDistorFact;
}

float moddistortion::getDistorFact()
{
	return DistorFact;
}
void moddistortion::process(float audioin[bufsize])
{
	moddistortion moddist;
	for(int i = 0; i<bufsize; i++)
	{
		audioin[i]=fmod(audioin[i]+1,getDistorFact()+1)-1;
	};

	for(int i = 0; i<bufsize; i++)
		{
			audioin[i]=audioin[i]*moddist.getAmpfactor();
		};
};
