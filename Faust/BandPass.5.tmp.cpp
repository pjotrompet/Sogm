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
	FAUSTFLOAT 	fslider0;
	FAUSTFLOAT 	fslider1;
	float 	fRec0[4];
  public:
	static void metadata(Meta* m) 	{ 
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 1; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 0.0f;
		fslider1 = 0.0f;
		for (int i=0; i<4; i++) fRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("BandPass.5.tmp");
		interface->addVerticalSlider("freq", &fslider1, 0.0f, -1.9f, 1.9f, 0.1f);
		interface->addVerticalSlider("q", &fslider0, 0.0f, 0.0f, -1.0f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = fslider0;
		float 	fSlow1 = fslider1;
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			fRec0[0] = ((fSlow1 * fRec0[2]) + ((float)input0[i] + (fSlow0 * fRec0[3])));
			output0[i] = (FAUSTFLOAT)(0.001f * fRec0[0]);
			// post processing
			for (int i=3; i>0; i--) fRec0[i] = fRec0[i-1];
		}
	}
};


