#ifndef _SONG_H_
#define _SONG_H_

#include <iostream>
#include <string>

using namespace std;

class Song
{
public:
  void setTitle(string newtitle);
  int setYear(unsigned short newyear);
  string getTitle();
  unsigned short getYear();

private:
  string title;
  unsigned short year;
}; // Song{}

#endif // _SONG_H_

