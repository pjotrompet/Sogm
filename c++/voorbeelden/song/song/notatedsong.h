#ifndef _NOTATEDSONG_H_
#define _NOTATEDSONG_H_

#include "song.h" // include base class definition

typedef string Notation; // make type Notation the same as type string

class NotatedSong : public Song
{
public:
  void set_sheet(Notation sheet);
  void display();
private:
  Notation sheet;
};

#endif // _NOTATEDSONG_H_

