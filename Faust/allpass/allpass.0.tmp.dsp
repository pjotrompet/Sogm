import("math.lib");
FIR=_<:_*pass+_(_@del:max 1: min 40000);
IIR=_:_+_~(_:_@100:_*(pass*-1));
pass=hslider("feedback",0.1,0.1,1,0.001);
del=hslider("Deltime", 1, 40000, 100, 0.1);
process = _<:IIR+FIR;