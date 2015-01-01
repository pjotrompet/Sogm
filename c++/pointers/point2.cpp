#include <iostream>

using namespace std;


int main()
{

unsigned short eenvariabele = 2;
unsigned short *eenvariabelepointer;
unsigned short tweedevariabele = 0;

eenvariabelepointer = &eenvariabele;

cout << "eenvariabele "<< eenvariabele << endl;
cout << "pointer naar eenvariabele " << *eenvariabelepointer << endl;
cout << "tweedevariabele " << tweedevariabele << endl;
cout << "bewerking = .... " << endl;



	/*
  // OEFENINGEN
  unsigned short year = 2013;
  unsigned short *yearpointer;

  yearpointer = &year;
  cout << "Inhoud van de variabele year: " << year << endl;
  cout << "Inhoud van de pointer yearpointer: " << yearpointer << endl  ;
  cout << "Inhoud van waar de pointer yearpointer naar wijst: " << *yearpointer << endl;
  // geef year via de pointer een nieuwe waarde
  *yearpointer = 2014;
  cout << "Inhoud van de variabele year: " << *yearpointer << endl;

  // maak nog een pointer genaamd anotheryearpointer en ook naar
  // year wijzen
  //anotheryearpointer = yearpointer;
    unsigned short *anotheryearpointer;
    anotheryearpointer = &*yearpointer;
  cout << "Inhoud van de pointer anotheryearpointer: " << *anotheryearpointer << endl;
  cout << "Inhoud van waar de pointer anotheryearpointer naar wijst: " << anotheryearpointer << endl;
  // geef year via de pointer een nieuwe waarde
  *anotheryearpointer = 2015;
  cout << "Inhoud van de variabele year: " << year << endl;
  cout << "Inhoud van waar de pointer yearpointer naar wijst: " << *yearpointer << endl;
	*/
	
} // main()
