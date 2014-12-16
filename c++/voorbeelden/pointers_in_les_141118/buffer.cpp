#include <iostream>
#include <string.h>

using namespace std;

#define SAMPLERATE 48000
#define BUFFERSIZE 10*SAMPLERATE

/*
 * process() is een functie die een pointer naar een buffer krijgt
 *
 * Binnen de functie kun je de data van het buffer lezen en schrijven
 *  zonder dat er een kopie van het buffer gemaakt hoeft te worden
 */
void process(float *buf)
{
  for(int i=0; i<BUFFERSIZE; i++){
    cout << *(buf+i) << endl; // twee gelijkwaardige regels
    cout << buf[i] << endl;
  }
} // process()


int main()
{
float *buffer;

  buffer = new float[BUFFERSIZE];

  for(int i=0; i<BUFFERSIZE; i++){
    *(buffer+i) = i;
  }

  /*
   * omdat buffer een float-pointer is wordt niet het hele buffer
   * meegegeven maar alleen de pointer,
   * dus het adres van de data
   */
  process(buffer);

  return 0;
} // main()

