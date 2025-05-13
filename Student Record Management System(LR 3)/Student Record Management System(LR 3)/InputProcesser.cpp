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
		std::string res = "Command | Description\nc - create new student\nstudent_id - to choose user\n";
		res+="P.S.\nFor edit user data you MUST firstly choose student\n";

		return { res,{{"cls"},{"pause","cls"}}};
	}
	try 
	{
		//check user exist
		return { editUserMenu(std::stoi(s),app->getStudentData(std::stoi(s))) ,{{},{"pause","cls"}} };
	}
	catch (const std::exception& e)
	{
		return { "Wrong command/id",{{}, {"pause","cls"}}};
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

std::string InputProcesser::editUserMenu(int id,std::vector < std::string>data)
{
	try {
		system("cls");
		std::cout << "Name: ";
		simulateConsoleInput(data.at(0));
		std::string new_name;
		std::getline(std::cin, new_name);
		std::cout << "Marks (divide by ,): ";
		simulateConsoleInput(data.at(1));
		std::string new_marks;
		std::getline(std::cin, new_marks);
		app->updateStudent(id, new_name, new_marks);
	}
	catch (...)
	{
		return "Something went wrong";
	}
	return "Data was update";
}


void InputProcesser::simulateConsoleInput(const std::string& text)
{
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	if (hInput == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error: GetStdHandle(STD_INPUT_HANDLE)" << std::endl;
		return;
	}
	std::vector<INPUT_RECORD> records;
	for (char c : text)
	{
		INPUT_RECORD rec = { 0 };
		rec.EventType = KEY_EVENT;

		rec.Event.KeyEvent.bKeyDown = TRUE;
		rec.Event.KeyEvent.wRepeatCount = 1;
		rec.Event.KeyEvent.wVirtualKeyCode = 0; // Unicode
		rec.Event.KeyEvent.uChar.UnicodeChar = c;
		records.push_back(rec);

		rec.Event.KeyEvent.bKeyDown = FALSE;
		records.push_back(rec);
	}

	DWORD written;
	if (!WriteConsoleInput(hInput, records.data(), (DWORD)records.size(), &written))
	{
		std::cerr << "WriteConsoleInput failed, error: " << GetLastError() << std::endl;
	}

}