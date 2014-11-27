#include <iostream>

using namespace std;
#define SR 48000
#define BUFFERSIZE 10*SR

void process(float *buf)

{
	for(int i=0;i<BUFFERSIZE;i++)
		{
		cout << *(buf+i) << endl;
		}
}//process

int main()
{

float *buffer;
buffer = new float[BUFFERSIZE];

	for(int i=0;i<BUFFERSIZE;i++)
	{
	*(buffer+i)=i;
	}


	//buffer = new float[BUFFERSIZE]
	process(buffer);

	return 0;
}//main
