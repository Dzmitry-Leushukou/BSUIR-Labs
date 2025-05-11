#pragma once
#include "Command.h"
#include "StudentRepository.h"
#include "Student.h"
#include "FileService.h"
class Invoker 
{
public:
    void setCommand(Command* command);
    void executeCommand();
private:
    Command* command = nullptr;
    StudentRepository repo;
};
