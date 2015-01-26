#include <iostream>
#include "modulodistortion.h"
#include "ampclass.h"


float eenaudioarray[10]={0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.2, 0.1, 0.9};


int main(int argc, char ** argv)
{
	moddistortion ampdist;
	ampdist.setAmpfactor(1);
	ampdist.setDistorFact(0.3);
	cout << "DistorFact " << ampdist.getDistorFact() << endl;
	cout << "AmpFact " <<ampdist.getAmpfactor() << endl;
	for(int i = 0; i < bufsize; i++)
	{
		cout << "op plek " << i << " " << eenaudioarray[i]<< endl;
	};


	ampdist.process(eenaudioarray);
	for(int i = 0; i < bufsize; i++)
	{
		cout << "op plek " << i << " " << eenaudioarray[i]<< endl;
	};

	return 0;

}
