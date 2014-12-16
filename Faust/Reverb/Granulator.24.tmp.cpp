//-----------------------------------------------------
// name: "Granulator"
// author: "Mayank Sanganeria"
// version: "1.0"
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
	class SIG0 {
	  private:
		int 	fSamplingFreq;
	  public:
		int getNumInputs() 	{ return 0; }
		int getNumOutputs() 	{ return 1; }
		void init(int samplingFreq) {
			fSamplingFreq = samplingFreq;
		}
		void fill (int count, float output[]) {
			for (int i=0; i<count; i++) {
				output[i] = 0.0f;
			}
		}
	};


	FAUSTFLOAT 	fcheckbox0;
	int 	iVec0[2];
	FAUSTFLOAT 	fslider0;
	int 	iConst0;
	FAUSTFLOAT 	fslider1;
	int 	iRec2[2];
	int 	iRec1[2];
	int 	iRec4[2];
	int 	iRec0[2];
	float 	ftbl0[480000];
	int 	iRec5[2];
	FAUSTFLOAT 	fslider2;
	int 	iRec7[2];
	int 	iRec6[2];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "Granulator");
		m->declare("author", "Mayank Sanganeria");
		m->declare("version", "1.0");
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
		fcheckbox0 = 0.0;
		for (int i=0; i<2; i++) iVec0[i] = 0;
		fslider0 = 1e+01f;
		iConst0 = min(192000, max(1, fSamplingFreq));
		fslider1 = 0.1f;
		for (int i=0; i<2; i++) iRec2[i] = 0;
		for (int i=0; i<2; i++) iRec1[i] = 0;
		for (int i=0; i<2; i++) iRec4[i] = 0;
		for (int i=0; i<2; i++) iRec0[i] = 0;
		SIG0 sig0;
		sig0.init(samplingFreq);
		sig0.fill(480000,ftbl0);
		for (int i=0; i<2; i++) iRec5[i] = 0;
		fslider2 = 1.0f;
		for (int i=0; i<2; i++) iRec7[i] = 0;
		for (int i=0; i<2; i++) iRec6[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Granulator.24.tmp");
		interface->addCheckButton("Grains!", &fcheckbox0);
		interface->addHorizontalSlider("delay length", &fslider0, 1e+01f, 0.5f, 1e+01f, 0.1f);
		interface->addHorizontalSlider("grain density", &fslider2, 1.0f, 1.0f, 2.0f, 1.0f);
		interface->addHorizontalSlider("grain length", &fslider1, 0.1f, 0.01f, 0.5f, 0.01f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = fcheckbox0;
		float 	fSlow1 = (1 - fSlow0);
		int 	iSlow2 = int((iConst0 * fslider0));
		int 	iSlow3 = int((iConst0 * fslider1));
		float 	fSlow4 = (0.5f * iSlow2);
		int 	iSlow5 = (iSlow3 - 1);
		float 	fSlow6 = (1.0f / iSlow5);
		float 	fSlow7 = (6.28318f / iSlow5);
		float 	fSlow8 = fslider2;
		int 	iSlow9 = (0 < fSlow8);
		int 	iSlow10 = (1 < fSlow8);
		float 	fSlow11 = (1.0f / fSlow8);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			float fTemp0 = (float)input0[i];
			iVec0[0] = 1;
			int iTemp1 = (1103515245 * (12345 + iRec2[1]));
			int iTemp2 = (1103515245 * (12345 + iTemp1));
			iRec2[0] = (1103515245 * (12345 + iTemp2));
			int 	iRec3 = iTemp2;
			int iTemp3 = int((fSlow4 * (1 + (2.3283064370807974e-10f * iRec2[0]))));
			int iTemp4 = (1 - iVec0[1]);
			iRec1[0] = ((iTemp4 * iTemp3) + (iVec0[1] * iRec1[1]));
			iRec4[0] = ((1 + iRec4[1]) % iSlow3);
			int iTemp5 = ((iRec4[0] + int(iRec1[0])) % iSlow3);
			int iTemp6 = int((fSlow6 * iTemp5));
			iRec0[0] = ((iTemp3 * iTemp6) + ((1 - iTemp6) * iRec0[1]));
			iRec5[0] = ((1 + iRec5[1]) % iSlow2);
			ftbl0[(iRec5[0] % iSlow2)] = (fSlow0 * fTemp0);
			int iTemp7 = int((fSlow4 * (1 + (2.3283064370807974e-10f * iRec3))));
			iRec7[0] = ((iTemp4 * iTemp7) + (iVec0[1] * iRec7[1]));
			int iTemp8 = ((iRec4[0] + int(iRec7[0])) % iSlow3);
			int iTemp9 = int((fSlow6 * iTemp8));
			iRec6[0] = ((iTemp7 * iTemp9) + ((1 - iTemp9) * iRec6[1]));
			output0[i] = (FAUSTFLOAT)((fSlow11 * ((iSlow10 * (sinf((fSlow7 * iTemp8)) * ftbl0[((iTemp8 + int(iRec6[0])) % iSlow2)])) + (iSlow9 * (sinf((fSlow7 * iTemp5)) * ftbl0[((iTemp5 + int(iRec0[0])) % iSlow2)])))) + (fSlow1 * fTemp0));
			// post processing
			iRec6[1] = iRec6[0];
			iRec7[1] = iRec7[0];
			iRec5[1] = iRec5[0];
			iRec0[1] = iRec0[0];
			iRec4[1] = iRec4[0];
			iRec1[1] = iRec1[0];
			iRec2[1] = iRec2[0];
			iVec0[1] = iVec0[0];
		}
	}
};


