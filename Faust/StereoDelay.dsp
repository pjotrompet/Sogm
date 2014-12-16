import("math.lib");
SamplRate=SR;
A=_*vslider("feedbackR", 0, 0, 1, 0.001);
B=_,_:+:mem(hslider("DelayR", 1, 1, 10000, 0.1)*(44100):int:%((44100*10000):int));
C=_*vslider("feedbackR", 0, 0, 1, 0.001);
D=_,_:+:mem(hslider("DelayR", 1, 1, 10000, 0.1)*(44100):int:%((44100*10000):int));

process=B~A,D~C;