#pragma once
#include <string>
#include <vector>
#include <stdexcept>
class StudentDTO
{
public:
	StudentDTO(std::string);
	StudentDTO(std::string, std::vector<int>);
	StudentDTO(int, std::string, std::string);
	std::string getName() const;
	std::vector<int> getMarks() const;
	int getId()const;
	void setName(std::string);
	void setMarks(std::vector<int>);
	void setId(int);
private:
	std::string name;
	std::vector<int>marks;
	int id;
};

