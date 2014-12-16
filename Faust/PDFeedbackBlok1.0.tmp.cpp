//-----------------------------------------------------
//
// Code generated with Faust 0.9.46 (http://faust.grame.fr)
//-----------------------------------------------------
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  

typedef long double quad;
/* link with  */

#define FAUSTCLASS mydsp

class mydsp : public dsp {
  private:
	int 	IOTA;
	float 	fVec0[4096];
	FAUSTFLOAT 	fslider0;
	float 	fVec1[4096];
  public:
	static void metadata(Meta* m) 	{ 
	}

	virtual int getNumInputs() 	{ return 2; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		IOTA = 0;
		for (int i=0; i<4096; i++) fVec0[i] = 0;
		fslider0 = 6e+01f;
		for (int i=0; i<4096; i++) fVec1[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("PDFeedbackBlok1.0.tmp");
		interface->addHorizontalSlider("sDel", &fslider0, 6e+01f, 1.0f, 2206.0f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		int 	iSlow0 = int(fslider0);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* input1 = input[1];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			fVec0[IOTA&4095] = (float)input0[i];
			output0[i] = (FAUSTFLOAT)(0.1f * fVec0[(IOTA-iSlow0)&4095]);
			fVec1[IOTA&4095] = (float)input1[i];
			output1[i] = (FAUSTFLOAT)(0.1f * fVec1[(IOTA-iSlow0)&4095]);
			// post processing
			IOTA = IOTA+1;
		}
	}
};


