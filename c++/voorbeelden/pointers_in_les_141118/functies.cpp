#include <iostream>
#include <string.h>

using namespace std;


void process(long *sample)
{
  *sample = *sample + 2;
} // process()


int main()
{
long sample=42;

  cout << "Sample: " << sample << endl;
  process(&sample);
  cout << "Sample: " << sample << endl;

  return 0;
} // main()

