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
	FAUSTFLOAT 	fbutton0;
	float 	fVec0[16];
	FAUSTFLOAT 	fslider0;
	int 	IOTA;
	float 	fVec1[32];
	float 	fVec2[128];
	float 	fVec3[128];
	float 	fVec4[512];
	float 	fVec5[1024];
	float 	fVec6[2048];
	float 	fVec7[4096];
	float 	fVec8[8192];
	float 	fVec9[8192];
	float 	fVec10[16384];
	float 	fVec11[16384];
	float 	fVec12[32768];
	float 	fRec0[2];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("math.lib/name", "Math Library");
		m->declare("math.lib/author", "GRAME");
		m->declare("math.lib/copyright", "GRAME");
		m->declare("math.lib/version", "1.0");
		m->declare("math.lib/license", "LGPL");
	}

	virtual int getNumInputs() 	{ return 2; }
	virtual int getNumOutputs() 	{ return 3; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fbutton0 = 0.0;
		for (int i=0; i<16; i++) fVec0[i] = 0;
		fslider0 = 0.6f;
		IOTA = 0;
		for (int i=0; i<32; i++) fVec1[i] = 0;
		for (int i=0; i<128; i++) fVec2[i] = 0;
		for (int i=0; i<128; i++) fVec3[i] = 0;
		for (int i=0; i<512; i++) fVec4[i] = 0;
		for (int i=0; i<1024; i++) fVec5[i] = 0;
		for (int i=0; i<2048; i++) fVec6[i] = 0;
		for (int i=0; i<4096; i++) fVec7[i] = 0;
		for (int i=0; i<8192; i++) fVec8[i] = 0;
		for (int i=0; i<8192; i++) fVec9[i] = 0;
		for (int i=0; i<16384; i++) fVec10[i] = 0;
		for (int i=0; i<16384; i++) fVec11[i] = 0;
		for (int i=0; i<32768; i++) fVec12[i] = 0;
		for (int i=0; i<2; i++) fRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Freeze.5.tmp");
		interface->addButton("freeze", &fbutton0);
		interface->addHorizontalSlider("freezelength", &fslider0, 0.6f, 0.1f, 15.0f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = fbutton0;
		float 	fSlow1 = fslider0;
		int 	iSlow2 = int(min(33060, max(1, int(fSlow1))));
		int 	iSlow3 = int(min(33060, max(1, int((2 * fSlow1)))));
		int 	iSlow4 = int(min(33060, max(1, int((5 * fSlow1)))));
		int 	iSlow5 = int(min(33060, max(1, int((7 * fSlow1)))));
		int 	iSlow6 = int(min(33060, max(1, int((23 * fSlow1)))));
		int 	iSlow7 = int(min(33060, max(1, int((54 * fSlow1)))));
		int 	iSlow8 = int(min(33060, max(1, int((79 * fSlow1)))));
		int 	iSlow9 = int(min(33060, max(1, int((220 * fSlow1)))));
		int 	iSlow10 = int(min(33060, max(1, int((340 * fSlow1)))));
		int 	iSlow11 = int(min(33060, max(1, int((454 * fSlow1)))));
		int 	iSlow12 = int(min(33060, max(1, int((623 * fSlow1)))));
		int 	iSlow13 = int(min(33060, max(1, int((845 * fSlow1)))));
		int 	iSlow14 = int(min(33060, max(1, int((1304 * fSlow1)))));
		float 	fSlow15 = (1 - fSlow0);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* input1 = input[1];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		FAUSTFLOAT* output2 = output[2];
		for (int i=0; i<count; i++) {
			float fTemp0 = (float)input0[i];
			float fTemp1 = (fSlow0 * fRec0[1]);
			fVec0[0] = (fTemp1 - fTemp0);
			float fTemp2 = fVec0[iSlow2];
			float fTemp3 = (fTemp0 + fTemp1);
			fVec1[IOTA&31] = (0 - (fTemp3 - fTemp2));
			float fTemp4 = fVec1[(IOTA-iSlow3)&31];
			float fTemp5 = (fTemp3 + fTemp2);
			fVec2[IOTA&127] = (0 - (fTemp5 - fTemp4));
			float fTemp6 = fVec2[(IOTA-iSlow4)&127];
			float fTemp7 = (fTemp5 + fTemp4);
			fVec3[IOTA&127] = (0 - (fTemp7 - fTemp6));
			float fTemp8 = fVec3[(IOTA-iSlow5)&127];
			float fTemp9 = (fTemp7 + fTemp6);
			fVec4[IOTA&511] = (0 - (fTemp9 - fTemp8));
			float fTemp10 = fVec4[(IOTA-iSlow6)&511];
			float fTemp11 = (fTemp9 + fTemp8);
			fVec5[IOTA&1023] = (0 - (fTemp11 - fTemp10));
			float fTemp12 = fVec5[(IOTA-iSlow7)&1023];
			float fTemp13 = (fTemp11 + fTemp10);
			fVec6[IOTA&2047] = (0 - (fTemp13 - fTemp12));
			float fTemp14 = fVec6[(IOTA-iSlow8)&2047];
			float fTemp15 = (fTemp13 + fTemp12);
			fVec7[IOTA&4095] = (0 - (fTemp15 - fTemp14));
			float fTemp16 = fVec7[(IOTA-iSlow9)&4095];
			float fTemp17 = (fTemp15 + fTemp14);
			fVec8[IOTA&8191] = (0 - (fTemp17 - fTemp16));
			float fTemp18 = fVec8[(IOTA-iSlow10)&8191];
			float fTemp19 = (fTemp17 + fTemp16);
			fVec9[IOTA&8191] = (0 - (fTemp19 - fTemp18));
			float fTemp20 = fVec9[(IOTA-iSlow11)&8191];
			float fTemp21 = (fTemp19 + fTemp18);
			fVec10[IOTA&16383] = (0 - (fTemp21 - fTemp20));
			float fTemp22 = fVec10[(IOTA-iSlow12)&16383];
			float fTemp23 = (fTemp21 + fTemp20);
			fVec11[IOTA&16383] = (0 - (fTemp23 - fTemp22));
			float fTemp24 = fVec11[(IOTA-iSlow13)&16383];
			float fTemp25 = (fTemp23 + fTemp22);
			fVec12[IOTA&32767] = (0 - (fTemp25 - fTemp24));
			fRec0[0] = fVec12[(IOTA-iSlow14)&32767];
			float 	fRec1 = (fTemp25 + fTemp24);
			output0[i] = (FAUSTFLOAT)fRec0[0];
			output1[i] = (FAUSTFLOAT)fRec1;
			output2[i] = (FAUSTFLOAT)(fSlow15 * (float)input1[i]);
			// post processing
			fRec0[1] = fRec0[0];
			IOTA = IOTA+1;
			for (int i=15; i>0; i--) fVec0[i] = fVec0[i-1];
		}
	}
};


