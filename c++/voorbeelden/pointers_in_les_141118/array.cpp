#include <iostream>

using namespace std;

#define NROFYEARS 4

int main()
{
  /*
   * maak een pointer die nog nergens naar wijst, of misschien naar
   * adres 0 maar zelfs dat mag je niet aannemen
   */
  short *years;

  /*
   * maak een array en laat de pointer years daar naar wijzen
   */
  years = new short[4] {2000,2010,2014,2015}; // C++ 11 constructie
  					// compileer met -std=c++11

  /*
   * maak een kopie van de pointer zodat we de originele pointer
   *  niet kwijtraken
   */
  short *yearpointer=years;

  for(int i=0; i<NROFYEARS; i++) {
    cout << *yearpointer << endl; // drie gelijkwaardige regels
    cout << *(years+i) << endl;
    cout << years[i] << endl;
    yearpointer = yearpointer + 1;
  } // for

  delete[] years; // opruimen

  return 0;
} // main()

