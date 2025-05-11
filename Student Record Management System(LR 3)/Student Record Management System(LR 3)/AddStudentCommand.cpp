#include "AddStudentCommand.h"

void AddStudentCommand::execute(StudentRepository& repo)
{
	repo.add(student);
	FileService::save(repo.get());
}