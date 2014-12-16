// Implementation file for Song class

#include "song.h"

void Song::setTitle(string newtitle)
{
  title=newtitle;
}

int Song::setYear(unsigned short newyear)
{
  if(newyear < 1900) return -1;
  year=newyear;
  return 0;
} // setYear()

string Song::getTitle()
{
  return title;
}

unsigned short Song::getYear()
{
  return year;
}


