// Implementation file for PlayableSong class

#include "playablesong.h"

void PlayableSong::set_url(string url)
{
  this->url = url;
}

void PlayableSong::play()
{
  cout << "Now playing \"" << url <<
    "\"" << endl;
}

void PlayableSong::play(long duration)
{
  cout << "Now playing " << url <<
    " for " << duration << " seconds" << endl;
}

void PlayableSong::play(int from,int to)
{
  cout << "Now playing " << url <<
    " from " << from << " to " << to << endl;
}

