import("math.lib");
loop(begin, eind, tijdinms)=
(_+((eind - begin)/tijdinms:max(1):min(SR/20)))~(((_<:(begin*(_==eind),_*(_!=eind):_+_)))@(1)):

_;

BitDepth=_/2^(8*checkbox("8 or 16 bit")+ 1):_;
sine=_*(2*PI):sin:_;
process = loop(0, 1, 100);//BitDepth:sine;

//((eind - begin)/tijdinms:max(1):min(SR/20))
