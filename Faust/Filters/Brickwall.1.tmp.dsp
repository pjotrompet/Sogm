import ("math.lib");

freq=12000;
a=SR/freq;
lala(x,a)= sum(i, 2, x@i*(sin((i-50)/a*2*PI)/((i-50)/a)));
process = (lala);