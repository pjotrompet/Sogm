#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <cerrno>
#include <unistd.h>
using namespace std;

int main()
{
    int f;
    FILE *file;
    file = fopen("/dev/ttyACM0","r+");  //Opening device file
    long long i = 0;
    for(i = 0 ; i < 10000 ; i++)
    {
        fscanf(file ,"%i", &f); //hopefully reading? from the file
        //fprintf(file,"%c,','); //To separate digit
        printf("result: %i \n", f);
        usleep(5000);
    }
    fclose(file);
}
