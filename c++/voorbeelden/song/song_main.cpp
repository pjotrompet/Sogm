#include <iostream>
#include "song.h"

int main(int argc, char ** argv)
{
  Song song1;
  song1.setTitle("Never gonna give you up");
  if(song1.setYear(1987) == -1){
    cout << "Alleen hippe liedjes svp\n";
  }
  else{
    cout << "Titel 1: " << song1.getTitle() <<
      " uit het jaar: " << song1.getYear() << endl;
  }

  return 0;
}

