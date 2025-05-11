#pragma once
#include "Student.h"
#include "StudentDTO.h"
class StudentRepository
{
public:
	void add(StudentDTO tmp);
	void update(StudentDTO);
	std::vector<Student>get();
	void setStudents(std::vector<Student>);
	bool init() const;
private:
	std::vector<Student>students;
	bool firstInit=false;
};

