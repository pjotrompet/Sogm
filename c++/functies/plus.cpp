#include <iostream>
using namespace std;

int telop(int getal1, int getal2)
{
 return getal1+getal2;
}

main()
{
int get1,get2;
    cout << "geef getal1 ";
    cin >> get1;
    cout << "geef getal2 ";
    cin >> get2;
    int uitkomst=telop(get1,get2);
    cout << "uitkomst = " <<
      uitkomst << endl;
    return 0;
}
