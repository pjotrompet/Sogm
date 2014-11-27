#include <iostream>
using namespace std;
//#include <array> //foutmelding bij laden van library?  
		 //	#error This file requires compiler and library support 
		 //	for the \ using namespace std;

//int eenarray[5] = {12, 14 , 1, 2, 3};
int tweearray[5] = {1, 2, 4, 6, 8};
//int getal = eenarray[x];
//int a = sizeof(tweearray) / sizeof(*tweearray); //of tweearray.size()?

void eenarrayfunctie(int eenarray[5])
{
	int x = 0;
	while (x < 5)
	{
	cout << x <<": " << eenarray[x] << endl;
	x++;
	}
}



main()
{
eenarrayfunctie(tweearray);
}
