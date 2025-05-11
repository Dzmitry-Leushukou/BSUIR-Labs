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
private:
	Invoker * invoker = new Invoker();
};

