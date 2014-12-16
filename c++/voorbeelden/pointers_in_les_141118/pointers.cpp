#include <iostream>

using namespace std;

int main()
{
  long year = 2014;
  // yearpointer laten wijzen naar waar year staat (adres van year)
  long *yearpointer = &year;

  cout << "year: " << year << endl;
  cout << "inhoud yearpointer: " << *yearpointer << endl;

  // via een pointer de inhoud van year veranderen
  *yearpointer = 4102;

  cout << "year: " << year << endl;
  cout << "inhoud yearpointer: " << *yearpointer << endl;

  return 0;
}

