import("math.lib");

IL = _; 															//(linkerinput)
IR = _; 															// Rechterinput
DelTime = (hslider("Pre-DelTime's", 0.6, 0, 15, 0.1):max(0):min(15));			//UserInput to change the basic pre-delays (same ratio's)
A= 0;																//A zero used for the Input of the Early reflections		
Del(i) = ((IL/2,IR/2<:((_-_:_@(((DelTime)*i):int)),_+_)));		//1Early reflection (building block)


Early = (_,A:Del(1):Del(2):Del(5):Del(7):Del(23):Del(54):Del(79):		
			Del(220):Del(340):Del(454):Del(623):Del(845):Del(1304):Del(1532):Del(2204));
process=Early;