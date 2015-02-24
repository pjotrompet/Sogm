#include <iostream>
using namespace std;

int Recursie(i){
  cout << "hoi" << endl;
  if (i < 0) {
    Recursie(i-1);
  }
  return 0;
}
int main(){Recursie(1000); return 0;}
