//---------------------------------
// A square wave oscillator
//---------------------------------
import("math.lib");
T = hslider("Period",1,0.1,2000.,0.1); // Period (ms)

N = SR/1000.*T:int; // The period in samples

a = hslider("Cyclic ratio",0.5,0,1,0.1); // Cyclic ratio

i  = +(1)~%(N):-(1); // 0,1,2...,n

Gate= _*(i,N*a : <) ;
process=Gate;