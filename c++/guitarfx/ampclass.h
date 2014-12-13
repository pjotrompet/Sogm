#ifndef _AMP_H
#define _AMP_H
using namespace std;

#define bufsize 64

class Amplifier
{
private:
float audioin[bufsize];
float AmpFactor;
public:
 void setAmpfactor(float AmpFactor);
 float getAmpfactor();
 float* process(float audioin[bufsize]);
};

#endif	
