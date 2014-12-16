/* midievent.h : class definities voor MidiEvent en afgeleide classes

  Voor het gemak heb ik meteen de implementatie van member-functies in de
  classes gezet. Als het programma iets groter wordt kun je beter aparte
  .h files voor alle class definities en .cpp files voor alle class
  implementaties maken.

  Marc G, november 2014

*/
#include <iostream>

using namespace std;


class MidiEvent // base class
{
public:
  MidiEvent() { timestamp=0; }
  void set_timestamp(long ts) {timestamp=ts;}
  long get_timestamp() {return timestamp;}
  virtual void play() { cout << "It is now " << (int)timestamp << endl;};
private:
  long timestamp;
};


class MidiNoteEvent : public MidiEvent // derived class
{
public:
  MidiNoteEvent(){};
  MidiNoteEvent(unsigned char p){notenumber=p;}
  void play() { MidiEvent::play(); // roep play() van base class aan
                cout << " Now playing note " << (int)notenumber << endl;}
private:
  bool on_off;
  unsigned char notenumber;
};

