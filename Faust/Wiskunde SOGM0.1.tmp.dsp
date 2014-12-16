import ( "math.lib");
basis= _+_,_*_,_@1;
ms=SR/1000;
FIR= _<:_@ms,_:+;
IIR= (_+_:_@ms)~_*0.99;


process = IIR;