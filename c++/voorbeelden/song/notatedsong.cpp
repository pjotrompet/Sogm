// Implementation file for NotatedSong class

#include "notatedsong.h"

void NotatedSong::set_sheet(Notation sheet)
{
  this->sheet=sheet;
}

void NotatedSong::display()
{
  cout << "This is the score " << sheet << endl;
}

