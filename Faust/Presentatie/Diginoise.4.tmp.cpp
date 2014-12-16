//-----------------------------------------------------
//
// Code generated with Faust 0.9.46 (http://faust.grame.fr)
//-----------------------------------------------------
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  

typedef long double quad;
/* link with  */
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
	int 	iVec0[2];
	int 	iRec1[2];
	FAUSTFLOAT 	fslider1;
	FAUSTFLOAT 	fslider2;
	FAUSTFLOAT 	fslider3;
	float 	fRec0[2];
  public:
	static void metadata(Meta* m) 	{ 
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 1; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 1.0f;
		for (int i=0; i<2; i++) iVec0[i] = 0;
		for (int i=0; i<2; i++) iRec1[i] = 0;
		fslider1 = 2.0f;
		fslider2 = 4e+02f;
		fslider3 = 1.0f;
		for (int i=0; i<2; i++) fRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Diginoise.4.tmp");
		interface->openHorizontalBox("Bitcrush");
		interface->addHorizontalSlider("crush", &fslider2, 4e+02f, 4e+02f, 499.0f, 1.0f);
		interface->closeBox();
		interface->openHorizontalBox("Distortion");
		interface->openHorizontalBox("Distortion");
		interface->addHorizontalSlider("Distortion", &fslider1, 2.0f, 2.0f, 1.5e+03f, 0.1f);
		interface->addHorizontalSlider("cleanamp", &fslider3, 1.0f, 1.0f, 1e+02f, 1.0f);
		interface->closeBox();
		interface->closeBox();
		interface->openHorizontalBox("Downsampling");
		interface->addHorizontalSlider("Downsampling", &fslider0, 1.0f, 1.0f, 8.0f, 0.2f);
		interface->closeBox();
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		int 	iSlow0 = int(faustpower<2>(fslider0));
		int 	iSlow1 = int(fslider1);
		float 	fSlow2 = (0 - (1 + (2 / ((0.01f * fslider2) - 5))));
		float 	fSlow3 = (1500 * (fslider3 / fSlow2));
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			iVec0[0] = (1 + (iRec1[1] % iSlow0));
			iRec1[0] = int(iVec0[1]);
			int iTemp0 = (iRec1[0] == 1);
			fRec0[0] = ((0.0006666666666666666f * (iTemp0 * (int((fSlow3 * int((fSlow2 * (float)input0[i])))) % iSlow1))) + ((1 - iTemp0) * fRec0[1]));
			output0[i] = (FAUSTFLOAT)fRec0[0];
			// post processing
			fRec0[1] = fRec0[0];
			iRec1[1] = iRec1[0];
			iVec0[1] = iVec0[0];
		}
	}
};


