/* Stoeien met pointers om toe te werken naar polymorfisme

 Marc G, november 2014
*/
#include "midi_event.h"

int main()
{
MidiEvent* eventqueue[10];

MidiEvent me;
MidiNoteEvent mne;

MidiEvent* me_ptr;
MidiNoteEvent* mne_ptr;

  me.set_timestamp(100);
  cout << me.get_timestamp() << endl;
  mne.set_timestamp(200);
  cout << mne.get_timestamp() << endl;

  me_ptr = new MidiEvent;
  mne_ptr = new MidiNoteEvent;

  me_ptr->set_timestamp(300);
  cout << me_ptr->get_timestamp() << endl;
  mne_ptr->set_timestamp(400);
  cout << mne_ptr->get_timestamp() << endl;

  eventqueue[0] = new MidiNoteEvent;
  eventqueue[1] = new MidiNoteEvent;
  eventqueue[2] = new MidiEvent;
  eventqueue[3] = new MidiNoteEvent;
  eventqueue[4] = new MidiNoteEvent;
  eventqueue[5] = new MidiEvent;
  eventqueue[6] = new MidiEvent;
  eventqueue[7] = new MidiNoteEvent;
  eventqueue[8] = new MidiNoteEvent;
  eventqueue[9] = new MidiNoteEvent;

  for(int i=0; i<10; i++)
  {
    eventqueue[i] -> set_timestamp(i*3);
  }

  for(int i=0; i<10; i++)
  {
    cout << eventqueue[i] -> get_timestamp() << endl;
  }

  delete me_ptr;
  delete mne_ptr;

  for(int i=0; i<10; i++)
  {
    delete eventqueue[i];
  }

  return 0;
}

