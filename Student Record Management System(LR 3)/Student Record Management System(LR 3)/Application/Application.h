#pragma once
#include "StudentDTO.h"
#include "FileService.h"
#include "../Invoker.h"
#include "../ShowStudentsCommand.h"
#include "../AddStudentCommand.h"
class Application
{
public:
	void addStudent(StudentDTO obj);
	std::string getStudents();
	~Application();
	std::vector<std::string>getStudentData(int);
	void updateStudent(int, std::string, std::string);
private:
	Invoker * invoker = new Invoker();
	Student st;
};

