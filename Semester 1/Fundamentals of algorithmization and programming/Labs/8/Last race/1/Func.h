#ifndef FUNC_H
#define FUNC_H

#include<string>

void newname(std::string s, std::string names[], long long& names_size);

std::string cins();

long long cinll();

long long input(long long l, long long r);

long long input_day(long long m, long long y, long long days[]);

std::string dat_input(long long days[]);

void show(T*& goods, long long& v_size);

void pop(long long ind, T*& goods, long long& v_size);

void push(T x, T*& goods, long long& v_size);

void sort(T*& goods, long long& v_size);

T add(long long flag, long long& names_size, std::string names[], long long days[]);

#endif