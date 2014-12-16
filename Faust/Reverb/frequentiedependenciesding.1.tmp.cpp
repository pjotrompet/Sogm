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
#include <cmath>
template <int N> inline float faustpower(float x) 		{ return powf(x,N); } 
template <int N> inline double faustpower(double x) 	{ return pow(x,N); }
template <int N> inline int faustpower(int x) 			{ return faustpower<N/2>(x) * faustpower<N-N/2>(x); } 
template <> 	 inline int faustpower<0>(int x) 		{ return 1; }
template <> 	 inline int faustpower<1>(int x) 		{ return x; }

#define FAUSTCLASS mydsp

class mydsp : public dsp {
  private:
	FAUSTFLOAT 	fslider0;
	int 	iConst0;
	int 	iRec1[2];
	int 	iRec2[2];
	FAUSTFLOAT 	fslider1;
	FAUSTFLOAT 	fslider2;
	int 	IOTA;
	int 	iRec0[524288];
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
		fslider0 = 25.0f;
		iConst0 = min(192000, max(1, fSamplingFreq));
		for (int i=0; i<2; i++) iRec1[i] = 0;
		for (int i=0; i<2; i++) iRec2[i] = 0;
		fslider1 = 1.0f;
		fslider2 = 1.0f;
		IOTA = 0;
		for (int i=0; i<524288; i++) iRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("frequentiedependenciesding.1.tmp");
		interface->addHorizontalSlider("grainfreq", &fslider0, 25.0f, 25.0f, 1e+03f, 0.1f);
		interface->addHorizontalSlider("time off", &fslider2, 1.0f, 1.0f, 1e+03f, 0.1f);
		interface->addHorizontalSlider("time on", &fslider1, 1.0f, 1.0f, 1e+03f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		int 	iSlow0 = int((iConst0 / fslider0));
		int 	iSlow1 = (7 * iSlow0);
		float 	fSlow2 = fslider1;
		float 	fSlow3 = fslider2;
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			iRec1[0] = faustpower<iSlow0>((7 * iRec1[1]));
			iRec2[0] = (7 * (iSlow0 + iRec2[1]));
			float fTemp0 = (((iSlow1 * (iRec2[0] * powf(iRec1[0],3.141592653589793f))) % iSlow0) % 192000);
			iRec0[IOTA&524287] = (1 + iRec0[(IOTA-int((int(abs(int(min(192000, max(1, (fSlow3 * fTemp0)))))) + int((1 + int(abs(int(min(192000, max(1, (fSlow2 * fTemp0)))))))))))&524287]);
			output0[i] = (FAUSTFLOAT)(sinf((float)input0[i]) * (iRec0[(IOTA-0)&524287] - 1));
			// post processing
			IOTA = IOTA+1;
			iRec2[1] = iRec2[0];
			iRec1[1] = iRec1[0];
		}
	}
};


