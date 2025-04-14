#include "XMLSaver.h"

void XMLSaver::save(std::vector<std::string> content, std::string filepath)
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* root = doc.NewElement("Document"); 

    for (const auto& text : content) {
        tinyxml2::XMLElement* lineElement = doc.NewElement("Line");
        lineElement->SetText(text.c_str()); 
        root->InsertEndChild(lineElement);
    }

    doc.InsertEndChild(root); 

    if (doc.SaveFile(filepath.c_str()) != tinyxml2::XML_SUCCESS)
        throw std::invalid_argument("Can`t create/edit xml file");
}