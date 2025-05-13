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


std::vector<std::string> Application::getStudentData(int id)
{
	std::vector<Student>data = FileService::load();
	for (auto& i : data)
	{
		if (i.getId() == id)
			return i.to_vector();
	}
	throw std::invalid_argument("User with id = "+std::to_string(id)+" doesn`t exist");
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

void Application::updateStudent(StudentDTO obj)
{
	UpdateStudentCommand* c = new UpdateStudentCommand(obj);
	invoker->setCommand(c);
	invoker->executeCommand();
	delete c;
	c = nullptr;
}