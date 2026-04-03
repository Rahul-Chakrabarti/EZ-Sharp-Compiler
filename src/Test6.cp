def int gcd(int a, int b)
int r;
if (a<0) then 
a = a 
fi;
if (b<0) then 
b = b 
fi;

if (a<b) then 
r=a; 
b=r;

fi;
while (b<>0) do
	r = a % b;
	a=r;
od;
return(a)
fed;


int a,b;
a = gcd(a,b)
.