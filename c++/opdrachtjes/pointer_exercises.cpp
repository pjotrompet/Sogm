#include <iostream>

using namespace std;


int main()
{
char letter = 97;
char *letterpointer;

  cout << "Inhoud van de variabele letter: ";
  cout << letter << endl;

  letterpointer = &letter;
  cout << "Inhoud van de pointer letterpointer: ";
  cout << (unsigned long) letterpointer << endl;

  cout << "Inhoud van waar de pointer letterpointer naar wijst: ";
  cout << *letterpointer << endl;

  *letterpointer = 'b';
  cout << "De variabele letter heeft via letterpointer een nieuwe waarde gekregen: ";
  cout << *letterpointer << endl;

  cout << "Inhoud van de variabele letter: ";
  cout << letter << endl;

  // OEFENINGEN
  unsigned short year = 2013;
  cout << "Inhoud van de variabele year: ";
  cout << "Inhoud van de pointer yearpointer: ";
  cout << "Inhoud van waar de pointer yearpointer naar wijst: ";
  // geef year via de pointer een nieuwe waarde
  cout << "Inhoud van de variabele year: ";

  // maak nog een pointer genaamd anotheryearpointer en laat deze ook naar
  // year wijzen
  //anotheryearpointer = yearpointer;
  cout << "Inhoud van de pointer anotheryearpointer: ";
  cout << "Inhoud van waar de pointer anotheryearpointer naar wijst: ";
  // geef year via de pointer een nieuwe waarde
  cout << "Inhoud van de variabele year: ";
  cout << "Inhoud van waar de pointer yearpointer naar wijst: ";

} // main()

