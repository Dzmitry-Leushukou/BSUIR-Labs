#include "StudentRepository.h"

void StudentRepository::add(StudentDTO obj)
{
	if (obj.getId() == 0)
	{
		obj.setId(students.size() + 1);
	}
	students.push_back(Student(obj.getId(), obj.getName(), obj.getMarks()));
}
void StudentRepository::update(StudentDTO obj)
{
	for (auto& i : students)
	{
		if (i.getId() == obj.getId())
		{
			i = Student(obj.getId(), obj.getName(), obj.getMarks());
			break;
		}
	}
}
std::vector<Student>StudentRepository::get()
{
	return students;
}

void StudentRepository::setStudents(std::vector<Student>v)
{
	students = v;
	firstInit = true;
}
bool StudentRepository::init() const
{
	return firstInit;
}