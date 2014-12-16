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
	float 	fConst0;
	int 	IOTA;
	int 	iVec0[262144];
	FAUSTFLOAT 	fslider1;
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
		fslider0 = 0.0f;
		fConst0 = (0.001f * min(192000, max(1, fSamplingFreq)));
		IOTA = 0;
		for (int i=0; i<262144; i++) iVec0[i] = 0;
		fslider1 = 1.0f;
		for (int i=0; i<262144; i++) iRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("timeGate.0.tmp");
		interface->addHorizontalSlider("time off", &fslider1, 1.0f, 1.0f, 1e+03f, 0.1f);
		interface->addHorizontalSlider("time on", &fslider0, 0.0f, 0.0f, 1e+03f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		int 	iSlow0 = int((1 + int(abs(int(min(192000, max(1, (fConst0 * fslider0))))))));
		int 	iSlow1 = int(abs(int(min(192000, max(1, (fConst0 * fslider1))))));
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			iVec0[IOTA&262143] = (iRec0[(IOTA-iSlow0)&262143] - 1);
			iRec0[IOTA&262143] = (1 + iVec0[(IOTA-iSlow1)&262143]);
			output0[i] = (FAUSTFLOAT)((float)input0[i] * iRec0[(IOTA-0)&262143]);
			// post processing
			IOTA = IOTA+1;
		}
	}
};


