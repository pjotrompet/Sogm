import("math.lib");

random(i, a)=(_*a~_+i:_*(a*i))*(_^i~_*a)^PI:_%(i:int):_%192000;

trigger(x)=(_+1~_@(x:max(1):min(192000):int:abs):_-1);

gate=_*trigger(random((SR)/hslider("grainfreq", 25, 25, 1000, 0.1), 7));

process=gate;





//trigger(random(SR/25,7));//random(SR/25, 7);