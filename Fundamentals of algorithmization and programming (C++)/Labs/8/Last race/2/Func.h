#ifndef FUNC_H
#define FUNC_H
#include<string>

#include "T.h"


std::string cins();

long long cinll();

long long input(long long l, long long r);

long long input_day(long long m, long long y, long long days[]);

std::string dat_input(long long days[]);

void show(T*& goods, long long& v_size);

void pop(long long ind, T*& goods, long long& v_size);

void push(std::fstream& file, T x, T*& goods, long long& v_size);

long long find(std::string s[], std::string a, long long n);

T add(long long flag, long long days[]);

#endif