import("math.lib");

//*--------------------------------UI's----------------------------------------*\\
DelTime = (hslider("Pre-DelTime's", 0.6, 0, 15, 0.1):max(0):min(15));
//UserInput to change the basic pre-delays (same ratio's)
UserFeBa = (hslider("Roomsize", 0.81, 0, 1, 0.01):max(0):min(2));
//Amount of feedback for the delaylines

//*------------------------------Process----------------------------------------*\\

Del(i) = (par(y, 8, _/8))<:(par(x, 8,
(_+_,_(((_-_):(_@(((DelTime)*i):int))))
))//par 
):>_,_,_,_,_,_,_,_;
//1Early reflection (building block)

Early = _<:Del(1):Del(2):Del(5):Del(7):Del(23):Del(54):Del(79):		
			Del(220):Del(340):Del(454):Del(623):Del(845):Del(1304):Del(1532):Del(2204);
//The Early Reflection chain

delline1(i) = /*(_+_*1.2)*/(par(z, 8, (_@(i*1:_+z*10:int)))):>_,_,_,_,_,_,_,_;
//Building block for the longer delaylines

delline8(x)=(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_:>delline1(x))~(delline1(x):_*UserFeBa,_*UserFeBa,_*UserFeBa,_*UserFeBa,_*UserFeBa,_*UserFeBa,_*UserFeBa,_*UserFeBa);
//Feedbacking for roomsize

Febaroute3 =_,_,_,_,_,_,_,_<:delline8(301),delline8(461),delline8(1025), delline8(1317), 
delline8(1723), delline8(2317), delline8(2913), delline8(506), delline8(2600),delline8(1900),
delline8(100), delline8(750) :>_,_,_,_,_,_,_,_;
//Complete Feedbackprocess

//*--------------------------End-Process---------------------------------------*\\
																
process =Early:Febaroute3;