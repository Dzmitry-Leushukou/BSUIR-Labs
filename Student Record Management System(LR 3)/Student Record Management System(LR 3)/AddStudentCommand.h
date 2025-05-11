#pragma once
#include "Command.h"
#include "StudentDTO.h"
#include "StudentRepository.h"
#include "FileService.h"
class AddStudentCommand : public Command {
public:
    AddStudentCommand(const StudentDTO& student) :
    student(student) 
    {}
    void execute(StudentRepository& repo) override;
private:
    StudentDTO student;
};

