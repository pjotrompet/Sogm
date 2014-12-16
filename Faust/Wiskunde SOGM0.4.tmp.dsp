import ( "math.lib");
basis= _+_,_*_,_@1;
ms=SR/1000;
FIR= _<:_@ms,_:+;
IIR= (_+_:_@ms)~_*0.99;
bouwles2= (_+_:_@401)~_*0.96;


process = bouwles2;

//aantekeningen

//Feedback:
	//negatieve feedback = oneven harmonische
	//positieve feedback = alle harmonische


//set theory

//N=iedere integer boven de 0
//Z=iedere integer
//Q=rationele getallen + breuken + alle andere integers
//R=Reeele getallen = BV pi (o.a. oneindige getallen in decimaal)
//C =complexe getallen = imaginare getallen = i of wortel -16
// quarternionen = voor 3Dgaming enz.


//dB= 20 10log V1/V2
