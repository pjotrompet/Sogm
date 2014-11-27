#include <iostream>
using namespace std;

int main()
{
//short years[] = {20, 30, 40, 50};
short *yearpointer;

years=new short[4]
yearpointer=years;
for( int i=0; i<4; i++)
	{
	cout << *yearpointer << endl;
	yearpointer++;
	cout << *(years+i) << endl;
	}//for
return 0;
}//main
