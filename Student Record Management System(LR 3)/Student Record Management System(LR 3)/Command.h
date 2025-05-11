#pragma once
#include "StudentRepository.h"
class Command 
{
public:
    virtual void execute(StudentRepository& repo) = 0;
    virtual ~Command() {}
};
