#include <iostream>

int main()
{
	long long a, b;
	std::cin >> a >> b;
	a = a ^ b;
	std::cout << "STEP 1: " << a << " " << b << "\n";
	b = b ^ a;
	std::cout << "STEP 2: " << a << " " << b << "\n";
	a = a ^ b;
	std::cout << "STEP 3: " << a << " " << b;
	 
}
/*
0 0 |0
0 1 |1
1 0 |1
1 1 |0

0110010
   ^
1110001
   =
1000011

*1 step:
a = a ^ b


(Inverse function of xor is xor) and (x xor x = 0) therefore:
*2 step:
b = b ^ (a ^ b) = a

*3 step:
a = (a ^ b) ^ a = b

Why won't the variable overflow?
a xor b <= max(a,b)
*/