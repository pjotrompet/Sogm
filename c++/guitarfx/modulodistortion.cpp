#include "modulodistortion.h"


void moddistortion::setDistorFact(float nDistorFact)
{
	DistorFact=nDistorFact;
}

float moddistortion::getDistorFact()
{
	return DistorFact;
}
float* moddistortion::process(float audioin[bufsize])
{
	moddistortion moddist;
	for(int i = 0; i<bufsize; i++)
	{
		audioin[i] = ((audioin[i] > moddist.getDistorFact())*audioin[i]);
	};
};
