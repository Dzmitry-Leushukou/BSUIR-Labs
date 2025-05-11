#include "ShowStudentsCommand.h"

void ShowStudentsCommand::execute(StudentRepository& repo)
{
	try
	{
		repo.setStudents(FileService::load());
	}
	catch (const std::exception& e)
	{
		if (repo.init())
		{
			output=e.what();
			return;
		}
		FileService::createDataFile();
	}
	if (repo.get().empty())
	{
		output = "No students - no problems (^_^)\n";
		return;
	}
	output = "ID | Name\n";
	for (auto& i : repo.get())
		output+=i.to_string()+"----------------------------------------------\n";
}