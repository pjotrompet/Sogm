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
	float 	fVec0[10];
	int 	iConst0;
	float 	fConst1;
	float 	fConst2;
	float 	fConst3;
	float 	fConst4;
	float 	fConst5;
	float 	fConst6;
	float 	fConst7;
	float 	fConst8;
	float 	fConst9;
	float 	fConst10;
	float 	fConst11;
	float 	fConst12;
	float 	fConst13;
	float 	fConst14;
	float 	fConst15;
	float 	fConst16;
	float 	fConst17;
	float 	fConst18;
	float 	fConst19;
	float 	fConst20;
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
		for (int i=0; i<10; i++) fVec0[i] = 0;
		iConst0 = min(192000, max(1, fSamplingFreq));
		fConst1 = (0 - (5.04e+05f / iConst0));
		fConst2 = (sinf((6.283185307179586f * fConst1)) / fConst1);
		fConst3 = (0 - (4.92e+05f / iConst0));
		fConst4 = (sinf((6.283185307179586f * fConst3)) / fConst3);
		fConst5 = (0 - (5.16e+05f / iConst0));
		fConst6 = (sinf((6.283185307179586f * fConst5)) / fConst5);
		fConst7 = (0 - (5.28e+05f / iConst0));
		fConst8 = (sinf((6.283185307179586f * fConst7)) / fConst7);
		fConst9 = (0 - (5.4e+05f / iConst0));
		fConst10 = (sinf((6.283185307179586f * fConst9)) / fConst9);
		fConst11 = (0 - (5.52e+05f / iConst0));
		fConst12 = (sinf((6.283185307179586f * fConst11)) / fConst11);
		fConst13 = (0 - (5.64e+05f / iConst0));
		fConst14 = (sinf((6.283185307179586f * fConst13)) / fConst13);
		fConst15 = (0 - (5.76e+05f / iConst0));
		fConst16 = (sinf((6.283185307179586f * fConst15)) / fConst15);
		fConst17 = (0 - (5.88e+05f / iConst0));
		fConst18 = (sinf((6.283185307179586f * fConst17)) / fConst17);
		fConst19 = (0 - (6e+05f / iConst0));
		fConst20 = (sinf((6.283185307179586f * fConst19)) / fConst19);
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("untitled.5.tmp");
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			float fTemp0 = (float)input0[i];
			fVec0[0] = fTemp0;
			output0[i] = (FAUSTFLOAT)((fConst20 * fVec0[0]) + ((fConst18 * fVec0[1]) + ((fConst16 * fVec0[2]) + ((fConst14 * fVec0[3]) + ((fConst12 * fVec0[4]) + ((fConst10 * fVec0[5]) + ((fConst8 * fVec0[6]) + ((fConst6 * fVec0[7]) + ((fConst4 * fVec0[9]) + (fConst2 * fVec0[8]))))))))));
			// post processing
			for (int i=9; i>0; i--) fVec0[i] = fVec0[i-1];
		}
	}
};


