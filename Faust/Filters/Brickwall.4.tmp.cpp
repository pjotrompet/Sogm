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
	float 	fVec0[2];
	FAUSTFLOAT 	fslider0;
	int 	iConst0;
	int 	iConst1;
	int 	iConst2;
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
		for (int i=0; i<2; i++) fVec0[i] = 0;
		fslider0 = 1.0f;
		iConst0 = min(192000, max(1, fSamplingFreq));
		iConst1 = (50 / iConst0);
		iConst2 = (49 / iConst0);
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Brickwall.4.tmp");
		interface->addHorizontalSlider("cutoff", &fslider0, 1.0f, 1.0f, 2.4e+04f, 1.0f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = fslider0;
		float 	fSlow1 = (0 - (iConst1 * fSlow0));
		float 	fSlow2 = (sinf((6.283185307179586f * fSlow1)) / fSlow1);
		float 	fSlow3 = (0 - (iConst2 * fSlow0));
		float 	fSlow4 = (sinf((6.283185307179586f * fSlow3)) / fSlow3);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			float fTemp0 = (float)input0[i];
			fVec0[0] = fTemp0;
			output0[i] = (FAUSTFLOAT)((fSlow4 * fVec0[1]) + (fSlow2 * fVec0[0]));
			// post processing
			fVec0[1] = fVec0[0];
		}
	}
};


