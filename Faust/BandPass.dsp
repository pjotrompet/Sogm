A=_;
input=(_+_~Delays):_*0.001;
Delays= _@1<:(_*vslider("freq", 0, -1.9, 1.9, 0.1),(_@1:_*vslider("q",0,0,-1,0.1))):_+_;
Bandpass=input;
process = Bandpass;