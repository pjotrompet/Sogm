import("math.lib");
FIR=_<:_@del,_*feedBack:+;
IIR=_+_@del~_*(feedBack*-1);
del=hslider("deltime", 1, 1, 4800 , 0.1):max(1):min(4800):int:_;
feedBack=hslider("feedback", 0, 0, 1, 0.1);
process = _<:IIR+FIR;