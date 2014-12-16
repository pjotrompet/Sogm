Volume1= _*hslider("a", 60, 0, 127, 0.1)/127;
Volume2= _*hslider("b", 60, 0, 127, 0.1)/127;
process = par (i,2,_@hslider("sDel", 60, 1, 2206, 0.1):_*0.1),Volume1,Volume2;