#pragma once
#include <string>
#include <vector>
#include "DataObject.h"
class Student:public DataObject
{
public:
	Student() = default;
	Student(int, std::string);
	Student(int, std::string, std::vector<int>);
	std::string getName() const;
	std::vector<int> getMarks() const;
	int getId();
	void setName(std::string);
	void setMarks(std::vector<int>);
	void setId(int);
	std::string to_string()const;
private:
	int id;
	std::string name;
	std::vector<int>marks;
};

