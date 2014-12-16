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
	float 	fVec14[65536];
	FAUSTFLOAT 	fslider2;
	int 	iVec15[2];
	int 	iConst0;
	float 	fConst1;
	FAUSTFLOAT 	fslider3;
	int 	iRec1[2];
	FAUSTFLOAT 	fcheckbox0;
	FAUSTFLOAT 	fslider4;
	FAUSTFLOAT 	fslider5;
	int 	iRec4[2];
	int 	iRec3[2];
	int 	iRec6[2];
	int 	iRec2[2];
	float 	ftbl0[480000];
	int 	iRec7[2];
	FAUSTFLOAT 	fslider6;
	int 	iRec9[2];
	int 	iRec8[2];
	FAUSTFLOAT 	fslider7;
	FAUSTFLOAT 	fslider8;
	float 	ftbl1[480000];
	FAUSTFLOAT 	fslider9;
	float 	fRec0[4096];
	float 	fRec10[4096];
	float 	fRec11[2048];
	float 	fRec12[2048];
	float 	fRec13[512];
	FAUSTFLOAT 	fslider10;
	FAUSTFLOAT 	fslider11;
	float 	fRec14[4096];
	float 	fRec15[512];
	float 	fRec16[4096];
	float 	fRec17[2048];
	float 	fRec18[512];
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
		for (int i=0; i<65536; i++) fVec14[i] = 0;
		fslider2 = 1.0f;
		for (int i=0; i<2; i++) iVec15[i] = 0;
		iConst0 = min(192000, max(1, fSamplingFreq));
		fConst1 = (0.001f * iConst0);
		fslider3 = 0.5f;
		for (int i=0; i<2; i++) iRec1[i] = 0;
		fcheckbox0 = 0.0;
		fslider4 = 1e+01f;
		fslider5 = 0.1f;
		for (int i=0; i<2; i++) iRec4[i] = 0;
		for (int i=0; i<2; i++) iRec3[i] = 0;
		for (int i=0; i<2; i++) iRec6[i] = 0;
		for (int i=0; i<2; i++) iRec2[i] = 0;
		SIG0 sig0;
		sig0.init(samplingFreq);
		sig0.fill(480000,ftbl0);
		for (int i=0; i<2; i++) iRec7[i] = 0;
		fslider6 = 1.0f;
		for (int i=0; i<2; i++) iRec9[i] = 0;
		for (int i=0; i<2; i++) iRec8[i] = 0;
		fslider7 = 1.0f;
		fslider8 = 0.5f;
		sig0.init(samplingFreq);
		sig0.fill(480000,ftbl1);
		fslider9 = 0.0f;
		for (int i=0; i<4096; i++) fRec0[i] = 0;
		for (int i=0; i<4096; i++) fRec10[i] = 0;
		for (int i=0; i<2048; i++) fRec11[i] = 0;
		for (int i=0; i<2048; i++) fRec12[i] = 0;
		for (int i=0; i<512; i++) fRec13[i] = 0;
		fslider10 = 5.0f;
		fslider11 = 1.0f;
		for (int i=0; i<4096; i++) fRec14[i] = 0;
		for (int i=0; i<512; i++) fRec15[i] = 0;
		for (int i=0; i<4096; i++) fRec16[i] = 0;
		for (int i=0; i<2048; i++) fRec17[i] = 0;
		for (int i=0; i<512; i++) fRec18[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("1StReverb(shortstuffBeta).2.tmp");
		interface->addHorizontalSlider("Cyclic ratio A", &fslider3, 0.5f, 0.0f, 1.0f, 0.1f);
		interface->addHorizontalSlider("Cyclic ratio B", &fslider8, 0.5f, 0.0f, 1.0f, 0.1f);
		interface->addCheckButton("Grains!", &fcheckbox0);
		interface->addHorizontalSlider("PeriodA", &fslider7, 1.0f, 1.0f, 2e+03f, 0.1f);
		interface->addHorizontalSlider("PeriodB", &fslider2, 1.0f, 1.0f, 2e+03f, 0.1f);
		interface->addHorizontalSlider("Pre-DelTime's", &fslider1, 0.6f, 0.0f, 15.0f, 0.1f);
		interface->addHorizontalSlider("Roomsize", &fslider0, 0.81f, 0.0f, 1.2f, 0.01f);
		interface->addHorizontalSlider("delay length", &fslider4, 1e+01f, 0.5f, 1e+01f, 0.1f);
		interface->addHorizontalSlider("dry", &fslider11, 1.0f, 0.0f, 1e+01f, 0.1f);
		interface->addHorizontalSlider("grain density", &fslider6, 1.0f, 1.0f, 2.0f, 1.0f);
		interface->addHorizontalSlider("grain length", &fslider5, 0.1f, 0.01f, 0.5f, 0.01f);
		interface->addHorizontalSlider("grainvol", &fslider9, 0.0f, 0.0f, 1e+03f, 0.1f);
		interface->addHorizontalSlider("wet", &fslider10, 5.0f, 0.0f, 1e+07f, 0.1f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = min(2, max(0, fslider0));
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
		int 	iSlow17 = int((fConst1 * fslider2));
		float 	fSlow18 = (fslider3 * iSlow17);
		float 	fSlow19 = fcheckbox0;
		float 	fSlow20 = (1 - fSlow19);
		int 	iSlow21 = int((iConst0 * fslider4));
		int 	iSlow22 = int((iConst0 * fslider5));
		float 	fSlow23 = (0.5f * iSlow21);
		int 	iSlow24 = (iSlow22 - 1);
		float 	fSlow25 = (1.0f / iSlow24);
		float 	fSlow26 = (6.28318f / iSlow24);
		float 	fSlow27 = fslider6;
		int 	iSlow28 = (0 < fSlow27);
		int 	iSlow29 = (1 < fSlow27);
		float 	fSlow30 = (1.0f / fSlow27);
		float 	fSlow31 = (fslider8 * int((fConst1 * fslider7)));
		float 	fSlow32 = (1.2f * fslider9);
		float 	fSlow33 = min(1000000000, max(0, fslider10));
		float 	fSlow34 = min(100, max(0, fslider11));
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			float fTemp0 = (float)input0[i];
			fVec0[0] = fTemp0;
			float fTemp1 = fVec0[iSlow2];
			float fTemp2 = (0.125f * (fVec0[0] + fTemp1));
			fVec1[IOTA&31] = ((0.25f * fTemp1) - (0.25f * fVec0[0]));
			float fTemp3 = (0.5f * fVec1[(IOTA-iSlow3)&31]);
			fVec2[IOTA&127] = (fTemp3 - fTemp2);
			float fTemp4 = fVec2[(IOTA-iSlow4)&127];
			float fTemp5 = (fTemp2 + fTemp3);
			float fTemp6 = (0.25f * (fTemp5 + fTemp4));
			fVec3[IOTA&127] = ((0.5f * fTemp4) - (0.5f * fTemp5));
			float fTemp7 = (0.5f * fVec3[(IOTA-iSlow5)&127]);
			fVec4[IOTA&511] = (fTemp7 - fTemp6);
			float fTemp8 = fVec4[(IOTA-iSlow6)&511];
			float fTemp9 = (fTemp6 + fTemp7);
			float fTemp10 = (0.25f * (fTemp9 + fTemp8));
			fVec5[IOTA&1023] = ((0.5f * fTemp8) - (0.5f * fTemp9));
			float fTemp11 = (0.5f * fVec5[(IOTA-iSlow7)&1023]);
			fVec6[IOTA&2047] = (fTemp11 - fTemp10);
			float fTemp12 = fVec6[(IOTA-iSlow8)&2047];
			float fTemp13 = (fTemp10 + fTemp11);
			float fTemp14 = (0.25f * (fTemp13 + fTemp12));
			fVec7[IOTA&4095] = ((0.5f * fTemp12) - (0.5f * fTemp13));
			float fTemp15 = (0.5f * fVec7[(IOTA-iSlow9)&4095]);
			fVec8[IOTA&8191] = (fTemp15 - fTemp14);
			float fTemp16 = fVec8[(IOTA-iSlow10)&8191];
			float fTemp17 = (fTemp14 + fTemp15);
			float fTemp18 = (0.25f * (fTemp17 + fTemp16));
			fVec9[IOTA&8191] = ((0.5f * fTemp16) - (0.5f * fTemp17));
			float fTemp19 = (0.5f * fVec9[(IOTA-iSlow11)&8191]);
			fVec10[IOTA&16383] = (fTemp19 - fTemp18);
			float fTemp20 = fVec10[(IOTA-iSlow12)&16383];
			float fTemp21 = (fTemp18 + fTemp19);
			float fTemp22 = (0.25f * (fTemp21 + fTemp20));
			fVec11[IOTA&16383] = ((0.5f * fTemp20) - (0.5f * fTemp21));
			float fTemp23 = (0.5f * fVec11[(IOTA-iSlow13)&16383]);
			fVec12[IOTA&32767] = (fTemp23 - fTemp22);
			float fTemp24 = fVec12[(IOTA-iSlow14)&32767];
			float fTemp25 = (fTemp22 + fTemp23);
			float fTemp26 = (0.25f * (fTemp25 + fTemp24));
			fVec13[IOTA&32767] = ((0.5f * fTemp24) - (0.5f * fTemp25));
			float fTemp27 = (0.5f * fVec13[(IOTA-iSlow15)&32767]);
			fVec14[IOTA&65535] = (fTemp27 - fTemp26);
			float fTemp28 = fVec14[(IOTA-iSlow16)&65535];
			iVec15[0] = 1;
			iRec1[0] = (1 + (iRec1[1] % iSlow17));
			int iTemp29 = (iRec1[0] - 1);
			float fTemp30 = (((iTemp29 < fSlow18) * fTemp28) * (fabsf(fTemp28) > 1e-59f));
			int iTemp31 = (1103515245 * (12345 + iRec4[1]));
			int iTemp32 = (1103515245 * (12345 + iTemp31));
			iRec4[0] = (1103515245 * (12345 + iTemp32));
			int 	iRec5 = iTemp32;
			int iTemp33 = int((fSlow23 * (1 + (2.3283064370807974e-10f * iRec4[0]))));
			int iTemp34 = (1 - iVec15[1]);
			iRec3[0] = ((iTemp34 * iTemp33) + (iVec15[1] * iRec3[1]));
			iRec6[0] = ((1 + iRec6[1]) % iSlow22);
			int iTemp35 = ((iRec6[0] + int(iRec3[0])) % iSlow22);
			int iTemp36 = int((fSlow25 * iTemp35));
			iRec2[0] = ((iTemp33 * iTemp36) + ((1 - iTemp36) * iRec2[1]));
			int iTemp37 = ((iTemp35 + int(iRec2[0])) % iSlow21);
			iRec7[0] = ((1 + iRec7[1]) % iSlow21);
			int iTemp38 = (iRec7[0] % iSlow21);
			ftbl0[iTemp38] = (fSlow19 * fTemp30);
			float fTemp39 = sinf((fSlow26 * iTemp35));
			int iTemp40 = int((fSlow23 * (1 + (2.3283064370807974e-10f * iRec5))));
			iRec9[0] = ((iTemp34 * iTemp40) + (iVec15[1] * iRec9[1]));
			int iTemp41 = ((iRec6[0] + int(iRec9[0])) % iSlow22);
			int iTemp42 = int((fSlow25 * iTemp41));
			iRec8[0] = ((iTemp40 * iTemp42) + ((1 - iTemp42) * iRec8[1]));
			int iTemp43 = ((iTemp41 + int(iRec8[0])) % iSlow21);
			float fTemp44 = sinf((fSlow26 * iTemp41));
			float fTemp45 = (fTemp26 + fTemp27);
			float fTemp46 = (((iTemp29 < fSlow31) * fTemp45) * (fabsf(fTemp45) > 1e-59f));
			ftbl1[iTemp38] = (fSlow19 * fTemp46);
			float fTemp47 = (fSlow32 * (((fSlow30 * ((iSlow29 * (fTemp44 * ftbl1[iTemp43])) + (iSlow28 * (fTemp39 * ftbl1[iTemp37])))) + (fSlow20 * fTemp46)) + ((fSlow30 * ((iSlow29 * (fTemp44 * ftbl0[iTemp43])) + (iSlow28 * (fTemp39 * ftbl0[iTemp37])))) + (fSlow20 * fTemp30))));
			fRec0[IOTA&4095] = (fTemp47 + (fSlow0 * fRec0[(IOTA-2601)&4095]));
			fRec10[IOTA&4095] = (fTemp47 + (fSlow0 * fRec10[(IOTA-2914)&4095]));
			fRec11[IOTA&2047] = (fTemp47 + (fSlow0 * fRec11[(IOTA-1724)&2047]));
			fRec12[IOTA&2047] = (fTemp47 + (fSlow0 * fRec12[(IOTA-1026)&2047]));
			fRec13[IOTA&511] = (fTemp47 + (fSlow0 * fRec13[(IOTA-302)&511]));
			float fTemp48 = (fSlow34 * fVec0[0]);
			output0[i] = (FAUSTFLOAT)(fTemp48 + (fSlow33 * ((((fRec13[(IOTA-0)&511] + fRec12[(IOTA-0)&2047]) + fRec11[(IOTA-0)&2047]) + fRec10[(IOTA-0)&4095]) + fRec0[(IOTA-0)&4095])));
			fRec14[IOTA&4095] = (fTemp47 + (fSlow0 * fRec14[(IOTA-3132)&4095]));
			fRec15[IOTA&511] = (fTemp47 + (fSlow0 * fRec15[(IOTA-507)&511]));
			fRec16[IOTA&4095] = (fTemp47 + (fSlow0 * fRec16[(IOTA-2318)&4095]));
			fRec17[IOTA&2047] = (fTemp47 + (fSlow0 * fRec17[(IOTA-1318)&2047]));
			fRec18[IOTA&511] = (fTemp47 + (fSlow0 * fRec18[(IOTA-462)&511]));
			output1[i] = (FAUSTFLOAT)(fTemp48 + (fSlow33 * ((((fRec18[(IOTA-0)&511] + fRec17[(IOTA-0)&2047]) + fRec16[(IOTA-0)&4095]) + fRec15[(IOTA-0)&511]) + fRec14[(IOTA-0)&4095])));
			// post processing
			iRec8[1] = iRec8[0];
			iRec9[1] = iRec9[0];
			iRec7[1] = iRec7[0];
			iRec2[1] = iRec2[0];
			iRec6[1] = iRec6[0];
			iRec3[1] = iRec3[0];
			iRec4[1] = iRec4[0];
			iRec1[1] = iRec1[0];
			iVec15[1] = iVec15[0];
			IOTA = IOTA+1;
			for (int i=15; i>0; i--) fVec0[i] = fVec0[i-1];
		}
	}
};


