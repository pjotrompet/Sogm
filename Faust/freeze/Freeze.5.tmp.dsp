import ("math.lib");
Button=button("freeze");
UserFeBa = hslider("Feedback", 0.81, 0, 1, 0.01);	
DelTime = hslider("freezelength", 0.6, 0.1, 15, 0.1);	
Del(i) = ((_,_<:((_-_:_@(((DelTime)*i):int:max(1):min(33060))),_+_)));
Early = (_,_:Del(1):Del(2):Del(5):Del(7):Del(23):Del(54):Del(79):		
			Del(220):Del(340):Del(454):Del(623):Del(845):Del(1304));	
process = (Early~_*Button),_*(Button*-1:_+1);