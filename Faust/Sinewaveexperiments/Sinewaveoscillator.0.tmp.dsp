import("math.lib");
Ramp(begin, eind, tijdinms)=

_;

BitDepth=_/2^(8*checkbox("8 or 16 bit")+ 1):_;
sine=_*(2*PI):sin:_;
process = BitDepth:sine;