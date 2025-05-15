#include "API.h"

QuoteFactory * API::qf = new QuoteFactory();
QuoteDTO* API::get()
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.quotable.io/quotes/random");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK)
        {
            auto quote = nlohmann::json::parse(readBuffer);
            std::string q = quote[0]["content"];
            std::string a = quote[0]["author"];
            return dynamic_cast<QuoteDTO*>(qf->CreateObject({ q, a }));
        }
        throw std::runtime_error("Can`t access server");
    }
}
size_t API::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}