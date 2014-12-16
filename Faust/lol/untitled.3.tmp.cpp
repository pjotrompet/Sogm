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
	float 	fVec0[5];
  public:
	static void metadata(Meta* m) 	{ 
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 1; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		for (int i=0; i<5; i++) fVec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("untitled.3.tmp");
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			float fTemp0 = (float)input0[i];
			fVec0[0] = fTemp0;
			output0[i] = (FAUSTFLOAT)(fVec0[1] + (fVec0[2] + (fVec0[3] + (fVec0[0] + fVec0[4]))));
			// post processing
			for (int i=4; i>0; i--) fVec0[i] = fVec0[i-1];
		}
	}
};


