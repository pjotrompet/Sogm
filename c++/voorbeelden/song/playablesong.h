#ifndef _PLAYABLESONG_H_
#define _PLAYABLESONG_H_

#include "song.h" // include base class definition

class PlayableSong : public Song
{
public:
  void set_url(string url);
  void play();
  void play(long duration);
  void play(int from,int to);
private:
  string url;
};

#endif // _PLAYABLESONG_H_

