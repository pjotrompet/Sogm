simplelow=_<:_@1,_:_+_:_;
parlow(x)=par(i,x,simplelow/x);
serlow(x)=seq(i,x,simplelow);
sigmalow(x)=sum(i,x,simplelow/x);
PI_low(x)=prod(i,x,(simplelow/i)/x);
process =
simplelow;
//parlow(10);
//serlow(10);
//sigmalow(10);
//PI_low(10);