// Main program entry

#include <iostream>
#include "playablesong.h"
#include "notatedsong.h"

int main(int argc, char ** argv)
{
  Song song1;
  song1.setTitle("Never gonna give you up");
  cout << "song 1 is titled " << "\"" <<
    song1.getTitle() << "\"" << endl;

  PlayableSong song2;
  song2.setTitle("Gurlz just wannah 'ave fun");
  song2.setYear(1983);
  song2.set_url("/home/marcg/gurlz.mp3");
  song2.play();

  NotatedSong song3;
  song3.setTitle("Black Beauty");
  song3.setYear(1972);
  song3.set_sheet("/home/marcg/blackbeauty.pdf");
  song3.display();

  return 0;
} // main()

