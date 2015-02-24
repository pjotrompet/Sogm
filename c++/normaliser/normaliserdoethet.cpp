#include <cstdlib>
#include <math.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <sndfile.h>
#include <stdlib.h>


using namespace std;



SF_INFO sfInfo;
SNDFILE* inFile;
SNDFILE* outFile;
char* path;
string outName;
string extension;
float gain;
int sampleRate;
unsigned long int channels;
unsigned long int numFrames;
unsigned long int filesize;
float* filepointer;



enum{ARG_NAME = 0, ARG_PATH, ARG_GAIN, NUM_ARGS};

float compute(float file[], long int filesize, float gain)
{
	float maxValue=0;
	int tel=0;
	float factor=0;
	while (tel < filesize)
	{
	if ((fabs(file[tel])) > maxValue)
		maxValue = fabs(file[tel]);
		tel++;
//	else
//		tel++;
	}
	if (maxValue == 0) return 1;
	factor = gain/maxValue;
	cout << factor << endl;
	return factor;
}

float* process(float* file, long int filesize, float gain)
{
		//filepointer= &file[filesize];
		float normFact=compute(file , filesize, gain);
		float* normArray=new float[filesize];
		for(long int i=0; i < filesize; i++)
		{
			normArray[i] = *(file + i)*normFact;
		}
		return &normArray[0];
}






int main(int argc, char* argv[]) {
	if (argc != NUM_ARGS || atof(argv[ARG_GAIN]) <= 0 ||
		atof(argv[ARG_GAIN]) > 1 ) {
			cout << "Give: \n"
			"- file path \n"
			"- gain(0 < gain < 1)" << endl;
			return -1;
		}
		path = argv[ARG_PATH];
		gain = atof(argv[ARG_GAIN]);
		inFile = sf_open(path, SFM_READ, &sfInfo);
		if(inFile == NULL) {
			cout << "Error opening file" << endl;
			return -1;
		}
		sampleRate = sfInfo.samplerate;
		channels = sfInfo.channels;
		numFrames = sfInfo.frames;
		cout << "Gain: " << gain << endl;
		cout << "Length: " << float(numFrames/sampleRate) << "s" << endl;
		cout << "Channels: " << channels << endl;
		cout << "Samplerate: " << sampleRate << endl;
		filesize = channels*numFrames;
		float buffer[filesize];
		float* normBuffer;
		sf_readf_float(inFile, buffer, numFrames);


		normBuffer = process(buffer, filesize, gain);
            cout << "hoi, hij doet het" << endl;
		outName = path;
		unsigned find_ext = outName.find_last_of(".");
		extension = outName.substr(find_ext);
		outName = outName.substr(0,find_ext);
		outName = outName + "_norm" + extension;
		outFile = sf_open(outName.c_str(), SFM_WRITE, &sfInfo);
		sf_writef_float(outFile, normBuffer, numFrames);
		cout << "Normalised file: " << outName << endl;
		sf_close(inFile);
		sf_close(outFile);

	}
