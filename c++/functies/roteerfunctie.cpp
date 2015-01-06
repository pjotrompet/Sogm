#include <iostream>

using namespace std;

void roteerarray(int array[], int lengte){

int newarray[lengte];

	newarray[lengte - 1] = array[0];

	for(int i = 1; i < lengte; i++) {
	newarray[i-1] = array[i];
}
	for (int i=0; i < lengte ; i++) {
	cout << "newarray: " << newarray[i] << endl;
	}
	
	for (int i=0; i < lengte; i++)
	{
		array[i]=newarray[i];
		cout << "array: " << array[i] << endl;
	}
};

int main() {

int telarray[] = {1, 2, 3, 4,5};
int tellength = 5;

roteerarray (telarray, tellength);
return 0;
}
