#include "PresentationLayer.h"
#include "InputProcesser.h"
void PresentationLayer::inputHandler()
{
	output(InputProcesser::process("h"));
	while (true) 
	{
		showStudents();
		std::string s;
		std::getline(std::cin, s);
		output(InputProcesser::process(s));
	}
}

void PresentationLayer::output(std::pair<std::string,std::pair<std::vector<std::string>,std::vector<std::string>>>data)
{
	operations(data.second.first);
	std::cout << data.first;
	operations(data.second.second);
}

void PresentationLayer::operations(const std::vector<std::string>op)
{
	for (auto& i : op)
	{
		system(i.c_str());
	}
}

void PresentationLayer::showStudents()
{
	output({ InputProcesser::app->getStudents(),{{"cls"},{}}});
}
