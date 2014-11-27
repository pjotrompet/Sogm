#include <iostream>
using namespace std;

int main ()
{

long year = 2014;
long *yearpointer = &year;

cout << "year: " << year << endl;
cout << "inhoud yearpointer " << yearpointer << endl;

*yearpointer = 2015;

cout << "year: " << year << endl;


char letter = 'a';
char *letterpointer = &letter;

cout << "letter: " << letter << endl;
cout << "letterpointer: " << (hex) << (long)letterpointer << endl;
cout << "inhoud letterpointer: " << *letterpointer << endl;

*letterpointer = 'z';
cout << "inhoud letterpointer: " << *letterpointer << endl;
cout << "letter: " << letter << endl;
return 0;
}
