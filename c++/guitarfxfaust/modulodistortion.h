#ifndef _MODDIST_H
#define _MODDIST_H
using namespace std;
#include "ampclass.h"

class moddistortion : public Amplifier
{
private:
float audioin[bufsize];
float DistorFact;
public:
void setDistorFact(float DistorFact);
float getDistorFact();
float* process(float audioin[bufsize]);
};

#endif
