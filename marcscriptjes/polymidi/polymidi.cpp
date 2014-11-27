/*
 * Polymorfisme: 
 * Een array met objecten met types (classes) die allemaal afgeleid zijn van
 *  dezelfde class.
 *
 * Concreet: een lijst met pointers naar midi-events. Tijdens uitvoeren van
 *  het programma wordt pas duidelijk welke specifieke events het zijn.
 *
 * In dit voorbeeld worden MidiEvent-objecten en MidiNoteEvent-objecten
 *  in de queue geplaatst. In de praktijk zul je geen MidiEvent-objecten
 *  maken omdat die niks nuttigs doen, maar met nog een afgeleide class
 *  zoals bv. MidiCommandEvent (broertje van MidiNoteEvent) kun je een
 *  array met MidiCommandEvents en MidiNoteEvents maken. Die array kan dan
 *  van het type MidiEvent* zijn omdat zowel MidiCommandEvent als
 *  MidiNoteEvent daarvan afgeleid zijn en dus speciale gevallen van
 *  MidiEvents zijn.
 *
 * Marc G, november 2014
 */

#include "midi_event.h"

int main()
{
/* lijst met pointers naar objecten van type MidiEvent of afgeleide types
 * N.B.: de pointers hebben nog geen inhoud gekregen en wijzen dus nog
 * nergens naar (of naar adres 0 misschien...)
 */
MidiEvent* eventqueue[10];

  /*
   * vul de lijst: maak telkens met 'new' een nieuw object, dat geeft een
   * pointer naar het object en die pointer stop je in het eerstvolgende vakje
   * van de queue zodat dat naar het nieuwe naamloze object gaat wijzen
   */
  eventqueue[0] = new MidiNoteEvent(60);
  eventqueue[1] = new MidiNoteEvent(62);
  eventqueue[2] = new MidiEvent;
  eventqueue[3] = new MidiNoteEvent(64);
  eventqueue[4] = new MidiNoteEvent(65);
  eventqueue[5] = new MidiEvent;
  eventqueue[6] = new MidiEvent;
  eventqueue[7] = new MidiNoteEvent(63);
  eventqueue[8] = new MidiNoteEvent(62);
  eventqueue[9] = new MidiNoteEvent(60);

  // geef alle events een timestamp
  for(int i=0; i<10; i++)
  {
    eventqueue[i] -> set_timestamp(i*3);
  }

  // speel alle events af (in een sequencer zou je natuurlijk naar de
  //  timestamps moeten kijken)
  for(int i=0; i<10; i++)
  {
    eventqueue[i] -> play();
  }

  /* Polymorfisme: tijdens uitvoeren van het programma wordt pas bepaald
   * welke play() wordt aangeroepen, dus in dit voorbeeld die van het
   * MidiEvent deel van een object of die van het MidiNoteEvent deel.
   *
   * Voeg eens een afgeleide class MidiCommandEvent toe die ook een eigen
   * play() heeft.
   */

  return 0;
}

