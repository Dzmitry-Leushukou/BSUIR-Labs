#pragma once
#ifndef T_H
#define T_H

class T {
public:
	char name[30];
	union q {
		long long amount;
	} kol;
	long double price;
	char dat[30];

};

#endif