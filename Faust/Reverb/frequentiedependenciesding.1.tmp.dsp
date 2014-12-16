import("math.lib");
safer=_:max(1):min(192000):int:abs:_;
random(i, a)=(_*a~_+i:_*(a*i))*(_^i~_*a)^PI:_%(i:int):_%192000;

trigger(x)=(((_@((x*hslider("time off", 1, 1, 1000, 0.1)):safer)):_+1)~
			  _@(x*hslider("time on", 1, 1, 1000, 0.1):safer):_-1);

gate=sin(_)*trigger(random((SR/hslider("grainfreq", 25, 25, 1000, 0.1):int), 7));

process=gate;





//trigger(random(SR/25,7));//random(SR/25, 7);