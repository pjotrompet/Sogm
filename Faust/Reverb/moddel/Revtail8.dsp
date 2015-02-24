import("math.lib");

IL = _; 															//(linkerinput)
IR = _; 															// Rechterinput
DelTime = (hslider("Pre-DelTime's", 0.6, 0, 15, 0.1):max(0):min(15));			//UserInput to change the basic pre-delays (same ratio's)
A= 0;
B=Early;



Del(i) = (((par(y, 8, IL/8))<:(par(x, 7,((_-_:_@(((DelTime)*i):int)))),_+_)));								//1Early reflection (building block)
Early = _<:Del(1):Del(2):Del(5):Del(7):Del(23):Del(54):Del(79):		
			Del(220):Del(340):Del(454):Del(623):Del(845):Del(1304):Del(1532):Del(2204);
																						//An Early Reflection chain
	

UserFeBa = (hslider("Roomsize", 0.81, 0, 1.2, 0.01):max(0):min(2));				//Amount of feedback for the delaylines


delline1(i,a) = /*(_+_*1.2)*/(sum(i, 8, (a+_/(i+1)))~(_@(i*(SR/SR):int):_*UserFeBa<:_,_,_,_,_,_,_,_));							//Building block for the longer delaylines


Febaroute2 = 	_,_,_,_,_,_,_,_<:
				delline1(301),delline1(461),delline1(1025),delline1(1317),
				delline1(1723),delline1(2317),delline1(2913),delline1(506),
				delline1(2600), delline1(3131),delline1(100),delline1(750),delline1(4800),delline1(4692),delline1(3492),delline1(3300):>_,_,_,_,_,_,_,_;
																
process =Early:Febaroute2;