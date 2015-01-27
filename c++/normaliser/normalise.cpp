#include <fstream>
#include <stdio.h>
#include <cstdlib>
#include <sndfile.h>
#include <cmath>
/*
als chn_num = 2; dan zijn er 2 samples per frame!
compile:
$ g++ -o normalise normalise.cpp -lsndfile
*/
enum {ARG_NAME = 0, ARG_PATH, ARG_GAIN, NUM_ARGS}; // enumerator
int main(int argc, char** argv) {
if(argc != NUM_ARGS) {
printf("usage: normaliser [filepath] [gain]\n");
printf("normaliser sets amplitude to 0dB\n");
printf("gain is used to change amplitude after normalisation.\n");
return -1;
}
//char* name = argv[ARG_NAME]; // ARG_NAME = 0 (uit enum)
int gain = std::atoi(argv[ARG_GAIN]);
char* path = argv[ARG_PATH];
SF_INFO in_info;
SNDFILE* in_file;
in_file = sf_open(path, SFM_READ, &in_info);
if(in_file == NULL) {
printf("error opening file!\n");
return -1;
}
int fs = in_info.samplerate;
unsigned long int num_frames = in_info.frames;
unsigned int chn_num = in_info.channels;
printf("length of file: %fs\n", float(num_frames)/fs);
printf("samplerate: %i\n", fs);
printf("num_frames: %li\n", num_frames);
printf("channels: %i\n", chn_num);
float buffer[num_frames*chn_num];
unsigned long int frames_read = sf_readf_float(in_file, buffer, num_frames);
printf("read %li frames form file\n", frames_read);
/*
for(unsigned long int i=0; i<frames_read; i++) {
printf("sample %li:\t%f\n", i, buffer[i]);
}
*/
float max_val = 0;
float abs_val;
for(unsigned long int n=0; n<num_frames*chn_num; n++) {
abs_val = std::sqrt(buffer[n]*buffer[n]);
if(abs_val > max_val) {
max_val = abs_val;
}
}
printf("peak value: %fdB\n", 20*std::log10(max_val));
printf("new max: %fdB\n", 20*std::log10(max_val*gain));
for(unsigned long int n=0; n<num_frames*chn_num; n++) {
buffer[n] = buffer[n]/max_val * gain;
}
SNDFILE* out_file;
std::string out_name = path; //maak string
out_name.append("_cp.wav");
printf("string: %s\n", out_name.c_str());
out_file = sf_open(out_name.c_str(), SFM_WRITE, &in_info);
unsigned long int frames_wrtn = sf_writef_float(out_file, buffer, num_frames);
printf("wrote %li frames to file\n", frames_wrtn);
sf_close(in_file);
sf_close(out_file);
return 0;
}
