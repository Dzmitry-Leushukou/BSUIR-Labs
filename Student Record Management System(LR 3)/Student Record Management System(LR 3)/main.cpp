#include "PresentationLayer.h"
#include "FileService.h"
#include "StudentDTO.h"	
#include <nlohmann/json.hpp>

int main()
{
	
	PresentationLayer* menu = new PresentationLayer();
	menu->inputHandler();
	delete menu;
	menu = nullptr;
}