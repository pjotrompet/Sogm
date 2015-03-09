import("math.lib");

//Userinterface
Delay=hslider("time(x)",1,0,10,0.1);
Feedback=hslider("feedback",0,0.5,2,0.001);
//Building-blocks
vertraag=_@(Delay*SR:int:max(0):min(10000));
//process
process =(_,_:>vertraag)~_*Feedback;