import("math.lib");
safer=_:max(1):min(192000):int:abs:_;
trigger(x)=(((_@((x*hslider("time off", 1, 1, 1000, 0.1)):safer)):_+1)~
			  (_@(x*hslider("time on", 0, 0, 1000, 0.1):safer):_-1));


process = _*trigger(SR/1000);