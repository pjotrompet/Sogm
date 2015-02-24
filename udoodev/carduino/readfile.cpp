#include <iostream>
#include <stdio.h>
#include <stdlib.h>
int main () {
  FILE * pFile;
  long lSize;
  char * buffer;
  size_t result;
for(int i=0; i < 100000; i++ ){
  pFile = fopen ( "/dev/ttyACM0" , "r+" );
  if (pFile==NULL) {fputs ("File error",stderr); return (1);}

  // obtain file size:
  fseek (pFile , 0 , SEEK_END);
  lSize = ftell (pFile);
  rewind (pFile);

  //allocate memory to contain the whole file:
  buffer = (char*) malloc (sizeof(char)*lSize);
  if (buffer == NULL) {fputs ("Memory error",stderr); return (2);}

  // copy the file into the buffer:
  result = fread (buffer,1,lSize,pFile);
  //if (result != lSize) {fputs ("Reading error",stderr); return (3);}
  std::cout << "resultaat " << result << std::endl;
  /* the whole file is now loaded in the memory buffer. */
}
  // terminate
  fclose (pFile);
  free (buffer);
  return 0;
}
