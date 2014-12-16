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
	float 	fVec0[32];
	float 	fVec1[128];
	float 	fVec2[128];
	float 	fVec3[512];
	float 	fVec4[1024];
	float 	fVec5[2048];
	float 	fVec6[4096];
	float 	fVec7[8192];
	float 	fVec8[8192];
	float 	fVec9[16384];
	float 	fVec10[16384];
	float 	fVec11[32768];
	float 	fRec0[32];
	FAUSTFLOAT 	fcheckbox0;
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
		fslider0 = 0.6f;
		IOTA = 0;
		for (int i=0; i<32; i++) fVec0[i] = 0;
		for (int i=0; i<128; i++) fVec1[i] = 0;
		for (int i=0; i<128; i++) fVec2[i] = 0;
		for (int i=0; i<512; i++) fVec3[i] = 0;
		for (int i=0; i<1024; i++) fVec4[i] = 0;
		for (int i=0; i<2048; i++) fVec5[i] = 0;
		for (int i=0; i<4096; i++) fVec6[i] = 0;
		for (int i=0; i<8192; i++) fVec7[i] = 0;
		for (int i=0; i<8192; i++) fVec8[i] = 0;
		for (int i=0; i<16384; i++) fVec9[i] = 0;
		for (int i=0; i<16384; i++) fVec10[i] = 0;
		for (int i=0; i<32768; i++) fVec11[i] = 0;
		for (int i=0; i<32; i++) fRec0[i] = 0;
		fcheckbox0 = 0.0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("Freeze.1.tmp");
		interface->addCheckButton("freeze", &fcheckbox0);
		interface->addHorizontalSlider("freezelength", &fslider0, 0.6f, 0.1f, 15.0f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = fslider0;
		int 	iSlow1 = int((1 + int(min(33060, max(1, int(fSlow0))))));
		int 	iSlow2 = int(min(33060, max(1, int((2 * fSlow0)))));
		int 	iSlow3 = int(min(33060, max(1, int((5 * fSlow0)))));
		int 	iSlow4 = int(min(33060, max(1, int((7 * fSlow0)))));
		int 	iSlow5 = int(min(33060, max(1, int((23 * fSlow0)))));
		int 	iSlow6 = int(min(33060, max(1, int((54 * fSlow0)))));
		int 	iSlow7 = int(min(33060, max(1, int((79 * fSlow0)))));
		int 	iSlow8 = int(min(33060, max(1, int((220 * fSlow0)))));
		int 	iSlow9 = int(min(33060, max(1, int((340 * fSlow0)))));
		int 	iSlow10 = int(min(33060, max(1, int((454 * fSlow0)))));
		int 	iSlow11 = int(min(33060, max(1, int((623 * fSlow0)))));
		int 	iSlow12 = int(min(33060, max(1, int((845 * fSlow0)))));
		int 	iSlow13 = int(min(33060, max(1, int((1304 * fSlow0)))));
		float 	fSlow14 = fcheckbox0;
		float 	fSlow15 = (1 - fSlow14);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			float fTemp0 = fRec0[(IOTA-1)&31];
			float fTemp1 = (float)input0[i];
			float fTemp2 = fRec0[(IOTA-iSlow1)&31];
			fVec0[IOTA&31] = (fTemp2 - fTemp0);
			float fTemp3 = fVec0[(IOTA-iSlow2)&31];
			float fTemp4 = (fTemp0 + fTemp2);
			fVec1[IOTA&127] = (0 - (fTemp4 - fTemp3));
			float fTemp5 = fVec1[(IOTA-iSlow3)&127];
			float fTemp6 = (fTemp4 + fTemp3);
			fVec2[IOTA&127] = (0 - (fTemp6 - fTemp5));
			float fTemp7 = fVec2[(IOTA-iSlow4)&127];
			float fTemp8 = (fTemp6 + fTemp5);
			fVec3[IOTA&511] = (0 - (fTemp8 - fTemp7));
			float fTemp9 = fVec3[(IOTA-iSlow5)&511];
			float fTemp10 = (fTemp8 + fTemp7);
			fVec4[IOTA&1023] = (0 - (fTemp10 - fTemp9));
			float fTemp11 = fVec4[(IOTA-iSlow6)&1023];
			float fTemp12 = (fTemp10 + fTemp9);
			fVec5[IOTA&2047] = (0 - (fTemp12 - fTemp11));
			float fTemp13 = fVec5[(IOTA-iSlow7)&2047];
			float fTemp14 = (fTemp12 + fTemp11);
			fVec6[IOTA&4095] = (0 - (fTemp14 - fTemp13));
			float fTemp15 = fVec6[(IOTA-iSlow8)&4095];
			float fTemp16 = (fTemp14 + fTemp13);
			fVec7[IOTA&8191] = (0 - (fTemp16 - fTemp15));
			float fTemp17 = fVec7[(IOTA-iSlow9)&8191];
			float fTemp18 = (fTemp16 + fTemp15);
			fVec8[IOTA&8191] = (0 - (fTemp18 - fTemp17));
			float fTemp19 = fVec8[(IOTA-iSlow10)&8191];
			float fTemp20 = (fTemp18 + fTemp17);
			fVec9[IOTA&16383] = (0 - (fTemp20 - fTemp19));
			float fTemp21 = fVec9[(IOTA-iSlow11)&16383];
			float fTemp22 = (fTemp20 + fTemp19);
			fVec10[IOTA&16383] = (0 - (fTemp22 - fTemp21));
			float fTemp23 = fVec10[(IOTA-iSlow12)&16383];
			fVec11[IOTA&32767] = (0 - ((fTemp22 + fTemp21) - fTemp23));
			fRec0[IOTA&31] = (fVec11[(IOTA-iSlow13)&32767] + (fTemp23 + (fTemp21 + (fTemp19 + (fTemp17 + (fTemp15 + (fTemp13 + (fTemp11 + (fTemp9 + (fTemp7 + (fTemp5 + (fTemp3 + (fTemp2 + (fTemp1 + fTemp0))))))))))))));
			output0[i] = (FAUSTFLOAT)((fSlow14 * fTemp1) + (fSlow15 * fRec0[(IOTA-0)&31]));
			// post processing
			IOTA = IOTA+1;
		}
	}
};


