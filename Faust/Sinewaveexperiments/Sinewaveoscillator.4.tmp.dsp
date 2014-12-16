import("math.lib");

loop (begin, eind, freq) = (eind - begin):fmod(_ - begin + (eind - begin) * freq / SR) + begin~(_@(1)):_;

//BitDepth=_/2^(8*checkbox("8 or 16 bit")+ 1):_;
//sine=_*(2*PI):sin:_;
process = loop(0., 2.*PI, 100.):sin * 0.5;//*(vslider("foo", 60, 0, 127, 0.1)/127); //loop(0, 1, 10);  //:sine*(vslider("foo", 60, 0, 127, 0.1)/127);

//((eind - begin)/tijdinms:max(1):min(SR/20))
//(((_<:(begin*(_==eind),_*(_!=eind):_+_)))