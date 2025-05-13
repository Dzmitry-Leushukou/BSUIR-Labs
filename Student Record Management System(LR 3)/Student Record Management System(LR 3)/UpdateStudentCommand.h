#pragma once
#include "Command.h"
#include "StudentDTO.h"
#include "StudentRepository.h"
#include "FileService.h"
class UpdateStudentCommand : public Command {
public:
    UpdateStudentCommand(const StudentDTO& student) :
        student(student)
    {
    }
    void execute(StudentRepository& repo) override;
private:
    StudentDTO student;
};
