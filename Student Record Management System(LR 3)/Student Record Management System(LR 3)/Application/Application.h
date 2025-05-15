#pragma once
#include "StudentDTO.h"
#include "FileService.h"
#include "../Invoker.h"
#include "../ShowStudentsCommand.h"
#include "../AddStudentCommand.h"
#include "../UpdateStudentCommand.h"
#include "../QuoteAdapter.h"
class Application
{
public:
	QuoteDTO * addStudent(StudentDTO obj);
	std::string getStudents();
	~Application();
	std::vector<std::string>getStudentData(int);
	void updateStudent(StudentDTO obj);
private:
	Invoker * invoker = new Invoker();
	Student st;
	QuoteAdapter* qa=new QuoteAdapter();
};

