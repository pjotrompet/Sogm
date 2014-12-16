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
	float 	fVec0[2];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("math.lib/name", "Math Library");
		m->declare("math.lib/author", "GRAME");
		m->declare("math.lib/copyright", "GRAME");
		m->declare("math.lib/version", "1.0");
		m->declare("math.lib/license", "LGPL");
	}

	virtual int getNumInputs() 	{ return 2; }
	virtual int getNumOutputs() 	{ return 1; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		for (int i=0; i<2; i++) fVec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Brickwall.1.tmp");
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* input1 = input[1];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			float fTemp0 = (float)input1[i];
			float fTemp1 = (0 - (50 / fTemp0));
			float fTemp2 = (float)input0[i];
			fVec0[0] = fTemp2;
			float fTemp3 = (0 - (49 / fTemp0));
			output0[i] = (FAUSTFLOAT)(((fVec0[1] * sinf((6.283185307179586f * fTemp3))) / fTemp3) + ((fVec0[0] * sinf((6.283185307179586f * fTemp1))) / fTemp1));
			// post processing
			fVec0[1] = fVec0[0];
		}
	}
};


