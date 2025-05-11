#pragma once
#include "Command.h"
#include "StudentDTO.h"
#include "StudentRepository.h"
#include "FileService.h"
class ShowStudentsCommand : public Command 
{
public:
    ShowStudentsCommand(std::string&out) :
        output(out)
    {
    }
    void execute(StudentRepository& repo) override;
private:
    std::string&output;
};

