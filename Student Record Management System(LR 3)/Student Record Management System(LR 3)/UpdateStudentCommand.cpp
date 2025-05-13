#include "UpdateStudentCommand.h"
void UpdateStudentCommand::execute(StudentRepository& repo)
{
	repo.update(student);
	FileService::save(repo.get());
}