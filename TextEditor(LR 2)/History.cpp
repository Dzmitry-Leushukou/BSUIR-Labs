#include "History.h"

std::string History::filesaver = "history.txt";

void History::createdFile(std::string user, std::string filepath)
{
	std::ofstream fout(filesaver,std::ios::app);
	fout << filepath << " created by " << user <<" "<<getTime()<< '\n';
    fout.close();
}

void History::savedFile(std::string user, std::string filepath)
{
    std::ofstream fout(filesaver, std::ios::app);
    fout << filepath << " saved by " << user << " " << getTime() << '\n';
    fout.close();
}

std::string History::getTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::tm local_time;
    localtime_s(&local_time, &now_c);

    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string History::getNotify(std::string filepath)
{
    std::string s;
    std::ifstream fin(filesaver);
    std::string ans;
    while (std::getline(fin, s))
    {
        std::string file;
        int i = 0;
        while (s[i] != ' ')
        {
            file += s[i];
            i++;
        }
        i++;
        if (file != filepath)
        {
            continue;
        }
        std::string type;
        while (s[i] != ' ')
        {
            type += s[i];
            i++;
        }
        type += s[i];
        i++;
        
        while (s[i] != ' ')
        {
            i++;
        }
     
        i++;

        std::string user;

        while (s[i] != ' ')
        {
            user += s[i];
            i++;
        }
        i++;

        std::string data;

        while (i<s.size())
        {
            data += s[i];
            i++;
        }
      
        std::time_t t1, t2;
       // t1 = parseDateTime(data);
        //t2 = parseDateTime(lastTime);
        //if (data > lastTime)
        //{
            ans += data + " " + user + " " + type+"\n";
        //}
    }
    fin.close();
    return ans;
}
std::time_t History::parseDateTime(const std::string& dateTimeStr)
{
    std::tm tm = {};
    std::istringstream ss(dateTimeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S"); // Парсим строку
    return std::mktime(&tm); // Преобразуем в time_t
}
