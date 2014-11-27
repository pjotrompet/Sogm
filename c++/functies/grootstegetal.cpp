#include <iostream>
using namespace std;


int tweearray[5] = {1, 100, 400, 6, 8};

int grootsteuitdearray(int array[5])
{
int n = 0;
int tel = 0;

	while (tel < 5)
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
