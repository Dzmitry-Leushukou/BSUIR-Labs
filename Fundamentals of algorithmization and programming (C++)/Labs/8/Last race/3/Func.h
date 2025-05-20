#pragma once
#ifndef FUNC_H
#define FUNC_H

#include<fstream>
#include<string>

#include "T.h"

long double cinld();

long long cinll();

long long input(long long l, long long r);

std::string cins();

long long input_day(long long m, long long y, const long long days[]);

std::string dat_input(const long long days[]);

void show(std::fstream& file, const char del[30], long long& v_size);

void show1(const char del[30], long long& v_size, T* &goods);

void pop(long long ind, const char del[30], long long& v_size, T* &goods);

void push(std::fstream& file, T x, T* &goods, long long& v_size);

T add(long long flag, const long long days[]);

long long find(std::string s[], std::string a, long long n);

#endif