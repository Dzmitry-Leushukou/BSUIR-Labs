#include "InputProcesser.h"

Application * InputProcesser::app = new Application();

std::pair<std::string, std::pair<std::vector<std::string>, std::vector<std::string>>> InputProcesser::process(std::string s)
{
	//create
	if (s == "c")
	{
		try
		{
			std::string username = getUsername();
			StudentDTO tmp(username, {});
			app->addStudent(tmp);
			return { "Student: "+username+" succesfully create!\n",{{"cls"},{"pause","cls"}} };
		}
		catch (...)
		{
			return { "",{{},{"cls"}} };
		}
	}
	//help
	if (s == "h")
	{
		std::string res = "Command | Description\nc - create new student\nstudent_id - to choose user\n+ - add mark\n";
		res += "e - edit student data\n* - edit student mark\n";
		res+="P.S.\nFor edit user data you MUST firstly choose student\n";

		return { res,{{"cls"},{"pause","cls"}}};
	}
	if (s == "+")
	{
		//add mark
	}
	if (s == "e")
	{
		//edit user data
	}
	if (s == "*")
	{
		//edit student marks
	}
	try 
	{
		//check user exist
		throw std::exception("govno");
	}
	catch (const std::exception& e)
	{
		return { e.what(),{{}, { "pause","cls" }} };
	}
}

std::string InputProcesser::getUsername()
{
	std::string s;
	std::cout << "Write username or (\"_-1_\" - to exit(without(\")): ";
	std::getline(std::cin, s);
	if (s == "_-1_")
		throw std::exception("INTERRUPT");
	return s;
}