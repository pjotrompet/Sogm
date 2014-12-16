
Bitdepth = hslider("Bitdepth", (2^16), (2^16), 0, 1);
soundinput = _ ;

Drive = hslider("Drive", 10000, 0, 10000, 0.1);
driveinput = _;

process= _*Drive:int%1:soundinput*Bitdepth:int:_/Bitdepth;