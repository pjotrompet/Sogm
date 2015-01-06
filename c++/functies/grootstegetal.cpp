#include <iostream>
using namespace std;

#define lengtofarray  5

int tweearray[lengtofarray] = {1, 100, 400, 6, 8};

int grootsteuitdearray(int array[lengtofarray])
{
int n = 0;
int tel = 0;

	while (tel < lengtofarray)
	{
	if (array[tel] > n )
	n = array[tel];
	tel++;
	}
	return n;
}


main()
{
cout << grootsteuitdearray(tweearray) << endl;
}
