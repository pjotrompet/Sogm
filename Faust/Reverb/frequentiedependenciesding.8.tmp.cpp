//-----------------------------------------------------
//
// Code generated with Faust 0.9.46 (http://faust.grame.fr)
//-----------------------------------------------------
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  

typedef long double quad;
/* link with  */
#include <math.h>

#define FAUSTCLASS mydsp

class mydsp : public dsp {
  private:
	FAUSTFLOAT 	fslider0;
	int 	iConst0;
	float 	fRec1[2];
	float 	fRec2[2];
	int 	iConst1;
	int 	IOTA;
	int 	iRec0[262144];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("math.lib/name", "Math Library");
		m->declare("math.lib/author", "GRAME");
		m->declare("math.lib/copyright", "GRAME");
		m->declare("math.lib/version", "1.0");
		m->declare("math.lib/license", "LGPL");
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 1; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 25.0f;
		iConst0 = min(192000, max(1, fSamplingFreq));
		for (int i=0; i<2; i++) fRec1[i] = 0;
		for (int i=0; i<2; i++) fRec2[i] = 0;
		iConst1 = (7 * iConst0);
		IOTA = 0;
		for (int i=0; i<262144; i++) iRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("frequentiedependenciesding.8.tmp");
		interface->addHorizontalSlider("grainfreq", &fslider0, 25.0f, 25.0f, 1e+03f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = fslider0;
		float 	fSlow1 = (iConst0 / fSlow0);
		int 	iSlow2 = int(fSlow1);
		float 	fSlow3 = (iConst1 / fSlow0);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			fRec1[0] = powf((7 * fRec1[1]),fSlow1);
			fRec2[0] = (7 * (fSlow1 + fRec2[1]));
			iRec0[IOTA&262143] = (1 + iRec0[(IOTA-int((1 + int(abs(int(min(192000, max(1, (((fSlow3 * (fRec2[0] * powf(fRec1[0],3.141592653589793f))) % iSlow2) % 192000)))))))))&262143]);
			output0[i] = (FAUSTFLOAT)((float)input0[i] * (iRec0[(IOTA-0)&262143] - 1));
			// post processing
			IOTA = IOTA+1;
			fRec2[1] = fRec2[0];
			fRec1[1] = fRec1[0];
		}
	}
};


