//-----------------------------------------------------
//
// Code generated with Faust 0.9.58 (http://faust.grame.fr)
//-----------------------------------------------------
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  

typedef long double quad;
/* link with  */

#ifndef FAUSTCLASS 
#define FAUSTCLASS mydsp
#endif

class mydsp : public dsp {
  private:
	FAUSTFLOAT 	fslider0;
	int 	IOTA;
	float 	fVec0[8192];
	FAUSTFLOAT 	fslider1;
	float 	fRec0[2];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("math.lib/name", "Math Library");
		m->declare("math.lib/author", "GRAME");
		m->declare("math.lib/copyright", "GRAME");
		m->declare("math.lib/version", "1.0");
		m->declare("math.lib/license", "LGPL with exception");
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 1; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 0.0f;
		IOTA = 0;
		for (int i=0; i<8192; i++) fVec0[i] = 0;
		fslider1 = 1.0f;
		for (int i=0; i<2; i++) fRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("allpass2.7.tmp");
		interface->addHorizontalSlider("deltime", &fslider1, 1.0f, 1.0f, 4.8e+03f, 0.1f);
		interface->addHorizontalSlider("feedback", &fslider0, 0.0f, 0.0f, 1.0f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = fslider0;
		float 	fSlow1 = (0 - fSlow0);
		int 	iSlow2 = int(min((float)4800, max((float)1, fslider1)));
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			float fTemp0 = (float)input0[i];
			fVec0[IOTA&8191] = fTemp0;
			float fTemp1 = fVec0[(IOTA-iSlow2)&8191];
			fRec0[0] = (fTemp1 + (fSlow1 * fRec0[1]));
			output0[i] = (FAUSTFLOAT)((fSlow0 * fVec0[IOTA&8191]) + (fTemp1 + fRec0[0]));
			// post processing
			fRec0[1] = fRec0[0];
			IOTA = IOTA+1;
		}
	}
};


