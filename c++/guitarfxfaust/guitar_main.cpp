#include <iostream>
#include "modulodistortion.h"
#include "ampclass.h"

int main(int argc, char ** argv)
{
	moddistortion ampdist;
	ampdist.setAmpfactor(0.5);
	ampdist.setDistorFact(0.8);
	cout << "DistorFact " << ampdist.getDistorFact() << endl;

	return 0;
	
}
