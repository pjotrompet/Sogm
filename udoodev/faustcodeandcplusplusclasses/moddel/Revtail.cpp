//-----------------------------------------------------
//
// Code generated with Faust 0.9.71 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with  */
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  


#ifndef FAUSTCLASS 
#define FAUSTCLASS mydsp
#endif

class mydsp : public dsp {
  private:
	FAUSTFLOAT 	fslider0;
	int 	IOTA;
	float 	fRec0[4096];
	float 	fRec1[4096];
	float 	fRec2[2048];
	float 	fRec3[2048];
	float 	fRec4[512];
	float 	fRec5[4096];
	float 	fRec6[512];
	float 	fRec7[4096];
	float 	fRec8[2048];
	float 	fRec9[512];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("math.lib/name", "Math Library");
		m->declare("math.lib/author", "GRAME");
		m->declare("math.lib/copyright", "GRAME");
		m->declare("math.lib/version", "1.0");
		m->declare("math.lib/license", "LGPL with exception");
	}

	virtual int getNumInputs() 	{ return 2; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 0.81f;
		IOTA = 0;
		for (int i=0; i<4096; i++) fRec0[i] = 0;
		for (int i=0; i<4096; i++) fRec1[i] = 0;
		for (int i=0; i<2048; i++) fRec2[i] = 0;
		for (int i=0; i<2048; i++) fRec3[i] = 0;
		for (int i=0; i<512; i++) fRec4[i] = 0;
		for (int i=0; i<4096; i++) fRec5[i] = 0;
		for (int i=0; i<512; i++) fRec6[i] = 0;
		for (int i=0; i<4096; i++) fRec7[i] = 0;
		for (int i=0; i<2048; i++) fRec8[i] = 0;
		for (int i=0; i<512; i++) fRec9[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("0x00");
		interface->addHorizontalSlider("Roomsize", &fslider0, 0.81f, 0.0f, 1.2f, 0.01f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = min((float)2, max((float)0, float(fslider0)));
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* input1 = input[1];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			float fTemp0 = (1.2f * (float)input0[i]);
			fRec0[IOTA&4095] = (fTemp0 + (fSlow0 * fRec0[(IOTA-2601)&4095]));
			fRec1[IOTA&4095] = (fTemp0 + (fSlow0 * fRec1[(IOTA-2914)&4095]));
			fRec2[IOTA&2047] = (fTemp0 + (fSlow0 * fRec2[(IOTA-1724)&2047]));
			fRec3[IOTA&2047] = (fTemp0 + (fSlow0 * fRec3[(IOTA-1026)&2047]));
			fRec4[IOTA&511] = ((fSlow0 * fRec4[(IOTA-302)&511]) + fTemp0);
			output0[i] = (FAUSTFLOAT)((((fRec4[(IOTA-0)&511] + fRec3[(IOTA-0)&2047]) + fRec2[(IOTA-0)&2047]) + fRec1[(IOTA-0)&4095]) + fRec0[(IOTA-0)&4095]);
			float fTemp1 = (1.2f * (float)input1[i]);
			fRec5[IOTA&4095] = (fTemp1 + (fSlow0 * fRec5[(IOTA-3132)&4095]));
			fRec6[IOTA&511] = (fTemp1 + (fSlow0 * fRec6[(IOTA-507)&511]));
			fRec7[IOTA&4095] = (fTemp1 + (fSlow0 * fRec7[(IOTA-2318)&4095]));
			fRec8[IOTA&2047] = (fTemp1 + (fSlow0 * fRec8[(IOTA-1318)&2047]));
			fRec9[IOTA&511] = ((fSlow0 * fRec9[(IOTA-462)&511]) + fTemp1);
			output1[i] = (FAUSTFLOAT)((((fRec9[(IOTA-0)&511] + fRec8[(IOTA-0)&2047]) + fRec7[(IOTA-0)&4095]) + fRec6[(IOTA-0)&511]) + fRec5[(IOTA-0)&4095]);
			// post processing
			IOTA = IOTA+1;
		}
	}
};


