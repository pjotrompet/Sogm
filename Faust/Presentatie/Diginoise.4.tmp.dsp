//Bitcrushing
userinput=-2 / (0.01*(hslider("crush", 400, 400,499, 1))-5)-1;
Bit=_*_;
user=_/userinput;
Bitcrushing = userinput,_:Bit:int:user;

//Distortion
pre = hslider("cleanamp", 1, 1, 100, 1)*_;
A=_*(1500):int;
B=_%_;
C=_/(1500);
Distortion = hgroup("Distortion",pre:A,(hslider("Distortion", 2, 2, 1500, 0.1):int)<:B:C);

//Downsampling



//userinput
Slide = (hslider("Downsampling", 1, 1, 8, 0.2)^2:int);

//Hold it
SH(trig,x) = _*(1 - trig) + x * trig~_;

//capture it
tel1 = _+1:_@1:int;
Tellen = tel1~%(Slide);
vergelijk = Tellen:_== 1;
Downsample = SH(vergelijk);

//Process
process = hgroup("Bitcrush",Bitcrushing):hgroup("Distortion",Distortion):hgroup("Downsampling",Downsample) ;
