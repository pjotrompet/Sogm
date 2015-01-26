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
	FAUSTFLOAT 	fslider1;
	int 	iRec0[2];
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
		fslider0 = 1.0f;
		fConst0 = (0.001f * min(192000, max(1, fSamplingFreq)));
		fslider1 = 0.5f;
		for (int i=0; i<2; i++) iRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Gate.3.tmp");
		interface->addHorizontalSlider("Cyclic ratio A", &fslider1, 0.5f, 0.0f, 1.0f, 0.1f);
		interface->addHorizontalSlider("PeriodB", &fslider0, 1.0f, 1.0f, 2e+03f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		int 	iSlow0 = int((fConst0 * fslider0));
		float 	fSlow1 = (fslider1 * iSlow0);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			iRec0[0] = (1 + (iRec0[1] % iSlow0));
			float fTemp0 = (float)input0[i];
			output0[i] = (FAUSTFLOAT)((fTemp0 * (fabsf(fTemp0) > 1e-59f)) * ((iRec0[0] - 1) < fSlow1));
			// post processing
			iRec0[1] = iRec0[0];
		}
	}
};


