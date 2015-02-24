#include <cmath>
#include <complex>
#include <iostream>
using namespace std;
const double EulerConstant = std::exp(1.0);
int main()
{
  complex<double> z(0.0,1.0);   // i
  complex<double> m(1.0,0.0);   // -1

  cout << "does it work?" << endl;
  cout << z << endl;
  cout << z*z << endl;
  for(int n = -2; n < 13; n++){
  int a = cos(n*M_PI/2*);
  int b = abs(cos(n*M_PI/2)*(pow (EulerConstant, -z*n*M_PI/2)));
}//for

  return 0;
}

//compileren met g++ -O2 -Wall C.cpp -o C -lcomplex -lm
