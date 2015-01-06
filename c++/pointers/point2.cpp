#include <iostream>

using namespace std;

short changevalue(short x)
	{
		x = x+1;
		return x;
	};
short* changevaluepoint(short* x)
	{
		*x = *x+1;
		return x;//hoe de plek waar de pointer naar wijst terug te geven?
	}


int main()
{

short getal = 4;
short *pointergetal;
pointergetal = &getal;

cout << "het getal "<< getal << endl;
cout << "pointer naar het getal " << *pointergetal << endl;
cout << "bewerking met het getal " << changevalue(getal) << endl;
cout << "het getal " << getal << endl;
cout << "bewerking met de pointer " << changevaluepoint(pointergetal)<< endl; //(geen pointer?!)
cout << "het getal " << getal << endl;
};




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

	
} // main()

*/

