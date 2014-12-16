import("math.lib");

IL = _; 															//(linkerinput)
IR = _; 															// Rechterinput
DelTime = (hslider("Pre-DelTime's", 0.6, 0, 15, 0.1):max(0):min(15));			//UserInput to change the basic pre-delays (same ratio's)
UserFeBa = (hslider("Roomsize", 0.81, 0, 1.2, 0.01):max(0):min(2));				//Amount of feedback for the delaylines
Volumeinputwet = (hslider("wet", 5, 0, 10000000, 0.1):max(0):min(1000000000));	//Amplification of the Wet signal
Volumeinputdry = (hslider("dry", 1, 0, 10, 0.1):max(0):min(100));				//Amplification of the Dry signal

A= 0;																//A zero used for the Input of the Early reflections		
//Gate system
T = hslider("PeriodB",1,1,2000.,0.1); // Period (ms)
U = hslider("PeriodA",1,1,2000.,0.1); // Period (ms)

SP = SR/1000.*T:int; // The period in samples

a = hslider("Cyclic ratio A",0.5,0,1,0.1); // Cyclic ratio
b= hslider("Cyclic ratio B",0.5,0,1,0.1); // Cyclic ratio

i  = +(1)~%(SP):-(1); // 0,1,2...,n

GateA= (_<:_*(_:abs: _> 0.00000000000000000000000000000000000000000000000000000000001))*(i,(SR/1000*T:int)*a : <);
GateB= (_<:_*(_:abs: _> 0.00000000000000000000000000000000000000000000000000000000001))*(i,(SR/1000*U:int)*b : <);

//1Early reflection 

Del(i) = ((IL/2,IR/2<:((_-_:_@(((DelTime)*i):int)),_+_)));		//1Early reflection (building block)


Early = (_,A:Del(1):Del(2):Del(5):Del(7):Del(23):Del(54):Del(79):		
			Del(220):Del(340):Del(454):Del(623):Del(845):Del(1304):Del(1532):Del(2204));
																	//An Early Reflection chain


delline1(i) = (_+_*1.2)~(_@(i*(SR/SR):int):_*UserFeBa);							//Building block for the longer delaylines


Febaroute2 = 	
				Early:GateA,GateB:(_:Granulator)+(_:Granulator)<:delline1(301),delline1(461),delline1(1025),delline1(1317),
				delline1(1723),delline1(2317),delline1(2913),delline1(506),
				delline1(2600), delline1(3131):>_,_;
																	//Early Reflections + parralel delay lines




//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Granulator
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

declare name "Granulator";
declare author "Mayank Sanganeria";
declare version "1.0";

// Controls
N = hslider("grain density", 1, 1, maxN, 1);
gLength = hslider("grain length", 0.1, 0.01, 0.5, 0.01);
dLength = hslider("delay length", 10, 0.5, 10, 0.1);

// Globals
counter = +(1)%delayLength~_; // to iterate through the delay line
delayBufferSize = 480000;
maxN = 2;//64;

//Granular synth variables
grainLength = int(SR*gLength);
delayLength = int(SR*dLength);


//Noise Generator

S(1,F) = F;
S(i,F) = F <: S(i-1,F),_ ;
Divide(n,k) = par(i, n, /(k)) ;
random = +(12345) : *(1103515245) ;
RANDMAX = 2^32 - 1 ;
chain(n) = S(n,random) ~ _;
NoiseN(n) = chain(n) : Divide(n,RANDMAX);

noiser = NoiseN(maxN+1);                          //multi channel noiser

NoiseChan(n,0) = noiser:>_,par( j, n-1 , !);
NoiseChan(n,i) = noiser:>par( j, i , !) , _, par( j, n-i-1,!);

noise(i) = (NoiseChan(maxN+1,i) + 1) / 2; //get nth channel of multi-channel noiser
//-------------Noise Generator End

//Sample & Hold
SH(trig,x) = (*(1 - trig) + x * trig) ~ _;


//Grain Positions
grainOffset(i) = int(SH(1-1',int(delayLength*noise(i)))) ;
grainCounterMaster = +(1)%grainLength~_;      // universal counter for all grains
grainCounter(i) = (grainCounterMaster + grainOffset(i) ) % grainLength;
grainRandomStartPos(i) = int(SH(int(grainCounter(i)/(grainLength-1)),int(delayLength*noise(i))));
grainPosition(i) = grainCounter(i) + grainRandomStartPos(i);

//Delay Line
buffer(write,read,x) = rwtable(delayBufferSize, 0.0, write%delayLength, x, read%delayLength);

//sin wave for windowing
window(i) = sin(2*3.14159*grainCounter(i)/(grainLength-1));
Grains = checkbox("Grains!");
Granulator = _<:_*(Grains*-1+1),(_*Grains<: par(i,maxN,buffer(counter, grainPosition(i))*window(i)*(i<N)/N) :> _):_+_:_*(hslider("grainvol", 0, 0, 1000, 0.1));

process =




		 _<:(_*Volumeinputdry),
		(Febaroute2:_/1,_/1:(_*Volumeinputwet,_*Volumeinputwet)),
		(_*Volumeinputdry):(_+_,_+_);