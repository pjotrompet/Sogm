import("math.lib");
//Gate system
T = hslider("PeriodB",1,1,2000.,0.1); // Period (ms)
U = hslider("PeriodA",1,1,2000.,0.1); // Period (ms)

SP = SR/1000.*T:int; // The period in samples

a = hslider("Cyclic ratio A",0.5,0,1,0.1); // Cyclic ratio
b= hslider("Cyclic ratio B",0.5,0,1,0.1); // Cyclic ratio

i  = +(1)~%(SP):-(1); // 0,1,2...,n

GateA= (_<:_*(_:abs: _> 0.00000000000000000000000000000000000000000000000000000000001))*(i,(SR/1000*T:int)*a : <); // killing unwanted high frequencies when very soft sounds are there	
GateB= (_<:_*(_:abs: _> 0.00000000000000000000000000000000000000000000000000000000001))*(i,(SR/1000*U:int)*b : <);
process = GateA;