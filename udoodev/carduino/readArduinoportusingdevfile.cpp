#include <iostream>
#include <stdio.h>

int main()
{
    unsigned long int lSize;
    void* buffer;
    int data[] = {10,5,13};  //Random data we want to send
    FILE *file;
    file = fopen("/dev/ttyACM0","r+");  //Opening device file
    int i = 0;
    for(i = 0 ; i = -1 ; i++)
    {
        fread(file+i,"%d",file+i,lSize); //Reading?
        fread(file,"%c",','); //To separate digits
    }
    fclose(file);
}
