#include <iostream>

using namespace std;
#define SR 48000
#define BUFFERSIZE 10*SR

void process(long sample)
{
sample =sample*2
}
/*
{
	for(int i=0;i<BUFFERSIZE;i++)
		{
		cout << *(buf+i) << endl;
		}
}//process
*/
int main()
{

long sample=42;
	cout << "Sample: " << sample << endl;
	process(&smaple)
	cout << "Sample: " << sample << endl;
return 0;
}

/*
buffer = new float[BUFFERSIZE];

	for(int i=0;i<BUFFERSIZE;i++)
	{
	*(buffer+i)=i;
	}


	//buffer = new float[BUFFERSIZE]
	process(buffer);

	return 0;
}//main
