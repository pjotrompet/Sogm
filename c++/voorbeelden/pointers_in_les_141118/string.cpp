#include <iostream>
#include <string.h> // oude stijl, is iets anders dan #include <string>

using namespace std;


/*
 * Oude stijl strings: een array van letters afgesloten met 0
 */

int main()
{
  char *title="New song";
  cout << title << endl;

  char *stringpointer = title;
  for(int i=0; i<=strlen(title); i++) {
    // typecast (int) zorgt dat je niet de letters ziet maar hun ASCII waarden
    cout << (int) *stringpointer << endl;
    // wandel door de string heen door de pointer te verzetten
    stringpointer++;
  } // for

  return 0;
} // main()

