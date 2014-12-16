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
	float 	fVec13[32768];
	float 	fVec14[65536];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("math.lib/name", "Math Library");
		m->declare("math.lib/author", "GRAME");
		m->declare("math.lib/copyright", "GRAME");
		m->declare("math.lib/version", "1.0");
		m->declare("math.lib/license", "LGPL");
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
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
		for (int i=0; i<32768; i++) fVec13[i] = 0;
		for (int i=0; i<65536; i++) fVec14[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("predel.3.tmp");
		interface->addHorizontalSlider("Pre-DelTime's", &fslider0, 0.6f, 0.0f, 15.0f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = min(15, max(0, fslider0));
		int 	iSlow1 = int(fSlow0);
		int 	iSlow2 = int((2 * fSlow0));
		int 	iSlow3 = int((5 * fSlow0));
		int 	iSlow4 = int((7 * fSlow0));
		int 	iSlow5 = int((23 * fSlow0));
		int 	iSlow6 = int((54 * fSlow0));
		int 	iSlow7 = int((79 * fSlow0));
		int 	iSlow8 = int((220 * fSlow0));
		int 	iSlow9 = int((340 * fSlow0));
		int 	iSlow10 = int((454 * fSlow0));
		int 	iSlow11 = int((623 * fSlow0));
		int 	iSlow12 = int((845 * fSlow0));
		int 	iSlow13 = int((1304 * fSlow0));
		int 	iSlow14 = int((1532 * fSlow0));
		int 	iSlow15 = int((2204 * fSlow0));
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			float fTemp0 = (float)input0[i];
			fVec0[0] = fTemp0;
			float fTemp1 = fVec0[iSlow1];
			float fTemp2 = (0.125f * (fVec0[0] + fTemp1));
			fVec1[IOTA&31] = ((0.25f * fTemp1) - (0.25f * fVec0[0]));
			float fTemp3 = (0.5f * fVec1[(IOTA-iSlow2)&31]);
			fVec2[IOTA&127] = (fTemp3 - fTemp2);
			float fTemp4 = fVec2[(IOTA-iSlow3)&127];
			float fTemp5 = (fTemp2 + fTemp3);
			float fTemp6 = (0.25f * (fTemp5 + fTemp4));
			fVec3[IOTA&127] = ((0.5f * fTemp4) - (0.5f * fTemp5));
			float fTemp7 = (0.5f * fVec3[(IOTA-iSlow4)&127]);
			fVec4[IOTA&511] = (fTemp7 - fTemp6);
			float fTemp8 = fVec4[(IOTA-iSlow5)&511];
			float fTemp9 = (fTemp6 + fTemp7);
			float fTemp10 = (0.25f * (fTemp9 + fTemp8));
			fVec5[IOTA&1023] = ((0.5f * fTemp8) - (0.5f * fTemp9));
			float fTemp11 = (0.5f * fVec5[(IOTA-iSlow6)&1023]);
			fVec6[IOTA&2047] = (fTemp11 - fTemp10);
			float fTemp12 = fVec6[(IOTA-iSlow7)&2047];
			float fTemp13 = (fTemp10 + fTemp11);
			float fTemp14 = (0.25f * (fTemp13 + fTemp12));
			fVec7[IOTA&4095] = ((0.5f * fTemp12) - (0.5f * fTemp13));
			float fTemp15 = (0.5f * fVec7[(IOTA-iSlow8)&4095]);
			fVec8[IOTA&8191] = (fTemp15 - fTemp14);
			float fTemp16 = fVec8[(IOTA-iSlow9)&8191];
			float fTemp17 = (fTemp14 + fTemp15);
			float fTemp18 = (0.25f * (fTemp17 + fTemp16));
			fVec9[IOTA&8191] = ((0.5f * fTemp16) - (0.5f * fTemp17));
			float fTemp19 = (0.5f * fVec9[(IOTA-iSlow10)&8191]);
			fVec10[IOTA&16383] = (fTemp19 - fTemp18);
			float fTemp20 = fVec10[(IOTA-iSlow11)&16383];
			float fTemp21 = (fTemp18 + fTemp19);
			float fTemp22 = (0.25f * (fTemp21 + fTemp20));
			fVec11[IOTA&16383] = ((0.5f * fTemp20) - (0.5f * fTemp21));
			float fTemp23 = (0.5f * fVec11[(IOTA-iSlow12)&16383]);
			fVec12[IOTA&32767] = (fTemp23 - fTemp22);
			float fTemp24 = fVec12[(IOTA-iSlow13)&32767];
			float fTemp25 = (fTemp22 + fTemp23);
			float fTemp26 = (0.25f * (fTemp25 + fTemp24));
			fVec13[IOTA&32767] = ((0.5f * fTemp24) - (0.5f * fTemp25));
			float fTemp27 = (0.5f * fVec13[(IOTA-iSlow14)&32767]);
			fVec14[IOTA&65535] = (fTemp27 - fTemp26);
			output0[i] = (FAUSTFLOAT)fVec14[(IOTA-iSlow15)&65535];
			output1[i] = (FAUSTFLOAT)(fTemp26 + fTemp27);
			// post processing
			IOTA = IOTA+1;
			for (int i=15; i>0; i--) fVec0[i] = fVec0[i-1];
		}
	}
};


