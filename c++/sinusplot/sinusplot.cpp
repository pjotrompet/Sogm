#include <iostream>
#include <math.h>
using namespace std;
#define PI 3.14159265
int main ()
{

	for(float x=0; x<100; x++) 
	{
	float nummer=sin(x * ((1.0/50.0)*(M_PI)));
	cout << nummer << endl;
	}
	return 0;
}
