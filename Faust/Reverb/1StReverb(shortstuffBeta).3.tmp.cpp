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
	float 	fVec0[16];
	FAUSTFLOAT 	fslider1;
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
	float 	fRec0[512];
	float 	fVec14[65536];
	float 	fRec1[4096];
	float 	fRec2[4096];
	float 	fRec3[2048];
	float 	fRec4[2048];
	float 	fRec5[512];
	float 	fRec6[2048];
	float 	fRec7[4096];
	float 	fRec8[512];
	float 	fRec9[4096];
	FAUSTFLOAT 	fslider2;
	FAUSTFLOAT 	fslider3;
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
		fslider0 = 0.81f;
		for (int i=0; i<16; i++) fVec0[i] = 0;
		fslider1 = 0.6f;
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
		for (int i=0; i<512; i++) fRec0[i] = 0;
		for (int i=0; i<65536; i++) fVec14[i] = 0;
		for (int i=0; i<4096; i++) fRec1[i] = 0;
		for (int i=0; i<4096; i++) fRec2[i] = 0;
		for (int i=0; i<2048; i++) fRec3[i] = 0;
		for (int i=0; i<2048; i++) fRec4[i] = 0;
		for (int i=0; i<512; i++) fRec5[i] = 0;
		for (int i=0; i<2048; i++) fRec6[i] = 0;
		for (int i=0; i<4096; i++) fRec7[i] = 0;
		for (int i=0; i<512; i++) fRec8[i] = 0;
		for (int i=0; i<4096; i++) fRec9[i] = 0;
		fslider2 = 5.0f;
		fslider3 = 1.0f;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("1StReverb(shortstuffBeta).3.tmp");
		interface->addHorizontalSlider("Pre-DelTime's", &fslider1, 0.6f, 0.0f, 15.0f, 0.1f);
		interface->addHorizontalSlider("Roomsize", &fslider0, 0.81f, 0.0f, 1.0f, 0.01f);
		interface->addHorizontalSlider("dry", &fslider3, 1.0f, 0.0f, 1e+02f, 0.1f);
		interface->addHorizontalSlider("wet", &fslider2, 5.0f, 0.0f, 1e+02f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = min(1, max(0, fslider0));
		float 	fSlow1 = min(15, max(0, fslider1));
		int 	iSlow2 = int(fSlow1);
		int 	iSlow3 = int((2 * fSlow1));
		int 	iSlow4 = int((5 * fSlow1));
		int 	iSlow5 = int((7 * fSlow1));
		int 	iSlow6 = int((23 * fSlow1));
		int 	iSlow7 = int((54 * fSlow1));
		int 	iSlow8 = int((79 * fSlow1));
		int 	iSlow9 = int((220 * fSlow1));
		int 	iSlow10 = int((340 * fSlow1));
		int 	iSlow11 = int((454 * fSlow1));
		int 	iSlow12 = int((623 * fSlow1));
		int 	iSlow13 = int((845 * fSlow1));
		int 	iSlow14 = int((1304 * fSlow1));
		int 	iSlow15 = int((1532 * fSlow1));
		int 	iSlow16 = int((2204 * fSlow1));
		float 	fSlow17 = (0.001f * min(100, max(0, fslider2)));
		float 	fSlow18 = min(100, max(0, fslider3));
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			float fTemp0 = (float)input0[i];
			fVec0[0] = fTemp0;
			float fTemp1 = fVec0[iSlow2];
			fVec1[IOTA&31] = (fTemp1 - fVec0[0]);
			float fTemp2 = fVec1[(IOTA-iSlow3)&31];
			float fTemp3 = (fVec0[0] + fTemp1);
			fVec2[IOTA&127] = (0 - (fTemp3 - fTemp2));
			float fTemp4 = fVec2[(IOTA-iSlow4)&127];
			float fTemp5 = (fTemp3 + fTemp2);
			fVec3[IOTA&127] = (0 - (fTemp5 - fTemp4));
			float fTemp6 = fVec3[(IOTA-iSlow5)&127];
			float fTemp7 = (fTemp5 + fTemp4);
			fVec4[IOTA&511] = (0 - (fTemp7 - fTemp6));
			float fTemp8 = fVec4[(IOTA-iSlow6)&511];
			float fTemp9 = (fTemp7 + fTemp6);
			fVec5[IOTA&1023] = (0 - (fTemp9 - fTemp8));
			float fTemp10 = fVec5[(IOTA-iSlow7)&1023];
			float fTemp11 = (fTemp9 + fTemp8);
			fVec6[IOTA&2047] = (0 - (fTemp11 - fTemp10));
			float fTemp12 = fVec6[(IOTA-iSlow8)&2047];
			float fTemp13 = (fTemp11 + fTemp10);
			fVec7[IOTA&4095] = (0 - (fTemp13 - fTemp12));
			float fTemp14 = fVec7[(IOTA-iSlow9)&4095];
			float fTemp15 = (fTemp13 + fTemp12);
			fVec8[IOTA&8191] = (0 - (fTemp15 - fTemp14));
			float fTemp16 = fVec8[(IOTA-iSlow10)&8191];
			float fTemp17 = (fTemp15 + fTemp14);
			fVec9[IOTA&8191] = (0 - (fTemp17 - fTemp16));
			float fTemp18 = fVec9[(IOTA-iSlow11)&8191];
			float fTemp19 = (fTemp17 + fTemp16);
			fVec10[IOTA&16383] = (0 - (fTemp19 - fTemp18));
			float fTemp20 = fVec10[(IOTA-iSlow12)&16383];
			float fTemp21 = (fTemp19 + fTemp18);
			fVec11[IOTA&16383] = (0 - (fTemp21 - fTemp20));
			float fTemp22 = fVec11[(IOTA-iSlow13)&16383];
			float fTemp23 = (fTemp21 + fTemp20);
			fVec12[IOTA&32767] = (0 - (fTemp23 - fTemp22));
			float fTemp24 = fVec12[(IOTA-iSlow14)&32767];
			float fTemp25 = (fTemp23 + fTemp22);
			fVec13[IOTA&32767] = (0 - (fTemp25 - fTemp24));
			float fTemp26 = fVec13[(IOTA-iSlow15)&32767];
			float fTemp27 = (fTemp25 + fTemp24);
			float fTemp28 = (fTemp27 + fTemp26);
			fRec0[IOTA&511] = (fTemp28 + (fSlow0 * fRec0[(IOTA-462)&511]));
			float fTemp29 = fRec0[(IOTA-0)&511];
			fVec14[IOTA&65535] = (0 - (fTemp27 - fTemp26));
			float fTemp30 = fVec14[(IOTA-iSlow16)&65535];
			fRec1[IOTA&4095] = (fTemp30 + (fSlow0 * fRec1[(IOTA-2601)&4095]));
			fRec2[IOTA&4095] = (fTemp30 + (fSlow0 * fRec2[(IOTA-2914)&4095]));
			fRec3[IOTA&2047] = (fTemp30 + (fSlow0 * fRec3[(IOTA-1724)&2047]));
			fRec4[IOTA&2047] = ((fSlow0 * fRec4[(IOTA-1026)&2047]) + fTemp30);
			fRec5[IOTA&511] = (fTemp30 + (fSlow0 * fRec5[(IOTA-302)&511]));
			float fTemp31 = ((((fRec5[(IOTA-0)&511] + fRec4[(IOTA-0)&2047]) + fRec3[(IOTA-0)&2047]) + fRec2[(IOTA-0)&4095]) + fRec1[(IOTA-0)&4095]);
			fRec6[IOTA&2047] = (fTemp28 + (fSlow0 * fRec6[(IOTA-1318)&2047]));
			float fTemp32 = fRec6[(IOTA-0)&2047];
			fRec7[IOTA&4095] = (fTemp28 + (fSlow0 * fRec7[(IOTA-2318)&4095]));
			float fTemp33 = fRec7[(IOTA-0)&4095];
			fRec8[IOTA&511] = (fTemp28 + (fSlow0 * fRec8[(IOTA-507)&511]));
			float fTemp34 = fRec8[(IOTA-0)&511];
			fRec9[IOTA&4095] = (fTemp28 + (fSlow0 * fRec9[(IOTA-3132)&4095]));
			float fTemp35 = fRec9[(IOTA-0)&4095];
			float fTemp36 = (fSlow18 * fVec0[0]);
			output0[i] = (FAUSTFLOAT)(fTemp36 + (fSlow17 * (fTemp35 + (fTemp34 + (fTemp33 + (fTemp32 + (fTemp31 + fTemp29)))))));
			output1[i] = (FAUSTFLOAT)(fTemp36 + (fSlow17 * (0 - (((((fTemp29 + fTemp32) + fTemp33) + fTemp34) + fTemp35) - fTemp31))));
			// post processing
			IOTA = IOTA+1;
			for (int i=15; i>0; i--) fVec0[i] = fVec0[i-1];
		}
	}
};


