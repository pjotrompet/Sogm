import("math.lib");
UserFeBa = (hslider("Roomsize", 0.81, 0, 1.2, 0.01):max(0):min(2));				//Amount of feedback for the delaylines


delline1(i) = (_+_*1.2)~(_@(i*(SR/SR):int):_*UserFeBa);							//Building block for the longer delaylines


Febaroute2 = 	_,_<:
				delline1(301),delline1(461),delline1(1025),delline1(1317),
				delline1(1723),delline1(2317),delline1(2913),delline1(506),
				delline1(2600), delline1(3131):>_,_;
																
process = Febaroute2;