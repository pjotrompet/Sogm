#ifndef _AMP_H
#define _AMP_H
using namespace std;

#define bufsize 10

class Amplifier
{
private:
float audioin[bufsize];
float AmpFactor;
public:
 void setAmpfactor(float AmpFactor);
 float getAmpfactor();
 void process(float audioin[bufsize]);
};

#endif
