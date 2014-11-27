#include <iostream>
#include <stdio.h>
#include <sndfile.h>
#include <stdlib.h>


using namespace std;


enum{ARG_NAME = 0, ARG_PATH, ARG_GAIN, NUM_ARGS};


int compute(int file[], int filesize)
{
	int bewerkfile[filesize];
	int n=0;
	int tel=0;
	int factor=0;		
	while (tel < filesize)
	{
	if (bewerkfile[tel] > n)	
	n = bewerkfile[tel];
	tel++
	}
	factor = 1/n;
	return factor;
}

int process(int file[filesize])
{
		int normArray[filesize] = file[0]
		for(int i tel=0; i < filesize i++)
		{
			normArray[i] = file[i*compute(file[filesize])]
		}
		return normArray[]
	

}




int main(int argc, char* argv[]) 
{
	if (argc != NUM_ARGS || atof(argv[ARG_GAIN]) <= 0 ||atof(argv[ARG_GAIN]) > 1 ) 
{
		cout << "Give: \n"
		"- file path \n"
		"- gain(0 < gain > 1)" << endl;
		return -1;
}
SF_INFO info;
SNDFILE* InFile;
InFile= sf_open(argv[ARG_PATH], SFM_READ,&info );

}







