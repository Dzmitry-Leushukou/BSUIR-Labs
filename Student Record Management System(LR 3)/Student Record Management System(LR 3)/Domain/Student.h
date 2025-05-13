#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include "DataObject.h"
class Student:public DataObject
{
public:
	Student() = default;
	Student(int, std::string);
	Student(int, std::string,std::string);
	Student(int, std::string, std::vector<int>);
	Student(std::vector<std::string>);
	std::string getName() const;
	std::vector<int> getMarks() const;
	int getId();
	void setName(std::string);
	void setMarks(std::vector<int>);
	void setId(int);
	std::string to_string()const;
	std::vector<std::string>to_vector()const;
private:
	int id;
	std::string name;
	std::vector<int>marks;
};

