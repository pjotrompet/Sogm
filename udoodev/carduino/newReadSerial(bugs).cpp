#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <cerrno>
#include <unistd.h>
using namespace std;

int main()
{
    unsigned char[2] sensorarray;
    unsigned char f;
    unsigned char test;
    FILE *file;
    file = fopen("/dev/ttyACM0","rb+");  //Opening device file
    long long i = 0;
    for(i = 0 ; i < 1000 ; i){
      fscanf(file,"%c", &f); //hopefully reading? from the file
        if(f != test){
        //no double nummers (change object)
        unsigned short out = f;
        printf("result: %i \n", out);
        usleep(1);
        test=f;
      }//if
      //else{printf("no change \n");}//else
    }//for
    fclose(file);
}
