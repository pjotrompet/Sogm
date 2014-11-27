#include <iostream>
using namespace std;

int oppervlakte(int lengte, int breedte)
{
 return lengte*breedte;
}

main()
{
int l,b;
    cout << "geef lengte ";
    cin >> l;
    cout << "geef breedte ";
    cin >> b;
    int op1=oppervlakte(l,b);
    int op2=oppervlakte(1,2);
    cout << "Totale oppervlakte " <<
      op1+op2 << endl;
    return 0;
}
