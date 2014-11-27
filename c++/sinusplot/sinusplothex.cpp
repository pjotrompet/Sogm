#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
#define PI 3.14159265
int main ()
{
	char hexdec [33];

	for(float x=0; x<100; x++) 
	{
	float nummer=sin(x * ((1.0/50.0)*(M_PI)));
	short (hexint=nummer*(pow(2,15)));
	sprintf(hexdec,"%.4x",hexint);
	cout << hexdec << endl;
	}
	return 0;

	
}

// itoa (i,buffer,16);
// printf ("hexadecimal: %s\n",buffer);
