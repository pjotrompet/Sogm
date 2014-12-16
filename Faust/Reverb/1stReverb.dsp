import("math.lib");

IL = _; 															//(linkerinput)
IR = _; 															// Rechterinput
DelTime = (hslider("Pre-DelTime's", 0.6, 0, 15, 0.1):max(0):min(15));			//UserInput to change the basic pre-delays (same ratio's)
UserFeBa = (hslider("Roomsize", 0.81, 0, 1, 0.01):max(0):min(1));				//Amount of feedback for the delaylines
Volumeinputwet = (hslider("wet", 5, 0, 100, 0.1):max(0):min(100));				//Amplification of the Wet signal
Volumeinputdry = (hslider("dry", 1, 0, 100, 0.1):max(0):min(100));				//Amplification of the Dry signal

A= 0;																//A zero used for the Input of the Early reflections		

Del(i) = ((IL,IR<:((_-_:_@(((DelTime)*i):int)),_+_)));		//1Early reflection (building block)


Early = (_,A:Del(1):Del(2):Del(5):Del(7):Del(23):Del(54):Del(79):		
			Del(220):Del(340):Del(454):Del(623):Del(845):Del(1304):Del(1532):Del(2204));
																	//An Early Reflection chain


delline1(i) = (_+_)~(_@(i:int):_*UserFeBa);							//Building block for the longer delaylines


Febaroute2 = 	
				Early<:delline1(301),delline1(461),delline1(1025),delline1(1317),
				delline1(1723),delline1(2317),delline1(2913),delline1(506),
				delline1(2600), delline1(3131):>_,_;
																	//Early Reflections + parralel delay lines

process = _<:(_*Volumeinputdry),(Febaroute2<:_+_,_-_:(_/1000:_*Volumeinputwet),(_/1000:_*Volumeinputwet)),(_*Volumeinputdry):_+_,_+_;