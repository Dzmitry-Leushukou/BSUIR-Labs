#include "Application.h"

std::string Application::getStudents()
{
	std::string out;
	ShowStudentsCommand* c = new ShowStudentsCommand(out);
	invoker->setCommand(c);
	invoker->executeCommand();
	delete c;
	c = nullptr;
	return out;
}

void Application::addStudent(StudentDTO obj)
{
	AddStudentCommand* c = new AddStudentCommand(obj);
	invoker->setCommand(c);
	invoker->executeCommand();
	delete c;
	c = nullptr;
}

Application::~Application()
{
	delete invoker;
	invoker = nullptr;
}