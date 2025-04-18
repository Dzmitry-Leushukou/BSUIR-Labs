#include "CloudService.h"

std::string CloudService::access_token = "sl.u.AFqmGJxbrCMLA_a_q9y5cKiOb0idfIZxnSjUwjujoLeY12zlnrpDdgAJyeZc2Vtn2pHWC0U6qJTZvKZlU2Oiujc8bnqINewxU9BpdTb14OEBeczp-ZtoOt5BEH9bPEFuVghpAlky1MzpLRObQeY9m4ySqHJcIhoWbJn4dtUkZSm2EXPC8pXOP2um_MheFU55IzWTrem7Z_N1c3-hpHtNXZeMrDiGflzjudmXd6cal8KS0Wydvb84low9glH12obj8yv8rVKY2PxsmrzKarMbuK_wSfTMwTFKfBZsp88AU7vu4UAo3KhJl7SoNsVBfgu9fYM9rdADo9bJtB4keylBjmPm2iH82IDSU_9A4zVmz6UvYopEFkKq2PT1muhwDUDEAgPLjRU7zj05HGLPQdhL2UZi5VTslthmvKa4wTagwIc27azUh0qpE_WhhYwDe00shQoOm6-aA-ybXVQp7ao44YKG-r1deb-Oiio9NYTW6slAWYybkNbPOy1rvXSIWceNbNS5dhiwLbd5oqROqd-LogRNjjamF_cYYzHfkdB_rIpeN04d6Q_jbIlz2lMMMPnpAy1Hw7uW53cElklw1im9HgFONxSdQSAQP7xlCrVS4qtWQkIEf7J7bF7i82uNe7EFTNdlorvOe97lc3UEHiiwWSn85D0b2GOaiz5Zz8EjZRsWdWpnb3uE82OAflJrCkS0X5xa1VdryxzvFJcm7diqESL-2BJOD6bWjU6rV7nxPTA4ToMbYQfnyxVmeC7_YOzonea1UQf8-5vPVMXZs1_Ro6CC_EOC9NCWk9XHrwtfAZ6NIdD7yIj_Vvb0OHj6FbQFaqK8bU_YAXt6Brnlur6oZlBa_bf9necpqLviB0HxWfaxtg8RXmutYG-o4ycosiEf3PGP57nGt92pEixd6EdBtTJ-7pwOd1ctqwf9w_UhsK7di_kBAc2ZeIfdy08OSXV30IOunuP_U4uHcr2PNBo0dCFcEykCSu_qP5yxOFkH8KHVPmYm1mJhD9lFd3ezpaVW7RmGSnJjSLAcqNiGa5UtQds8CVYLRCyCW9wyIhPEJNIBKVd72sXeTY9P-mqJBmEY9VN8J0v4hrx5pE1P-h1JBNTeFcDcrV87PfDEbs1obcbgnCeJcroIRJfY9ASWDZ9beBmHyP_C1pooberYq6LlVGt3rlfBfUP2xCORngyslc6rtQfmb4ixLeXX7NTGuwQDgZFgqo7nu8WqYDkKASlmH88l9Z4IphwR81k_cvVoP8pux62924JRgv5QmRmPMWPblYVNRnpxALKhD6R3Sqs8w82W8C-LNHXNOVepYHUrh_OfLXiHeOGXfVmqXXO66wVDGKj2M8WiIk2ilsw51X_PMlt0hemirz33-w1I41QfcktVOJho6sHdIuT-AvqKH6pCl5XBkaauhtiU8ciGrTcspU9r6FJjJCxiUo-H7BtZpMj_Rg";
void CloudService::save(std::string filename)
{
    CURL* curl;
    CURLcode res;
    struct curl_slist* headers = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Can`t open file");
        }
        std::string data,s;
        while (std::getline(file, s))
        {
            data += s + "\n";
        }

        int i = filename.size() - 1;
        std::string tmp;
        while(filename[i]!='/'&&i>=0)
        {
            tmp += filename[i];
            i--;
        }
        std::reverse(tmp.begin(), tmp.end());
        filename = tmp;
        curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());

        headers = curl_slist_append(headers, ("Authorization: Bearer "+ access_token).c_str());

        std::string json_arg = "{\"path\": \"/apps/" +filename+  "\",\"mode\": \"add\",\"mute\": true}";
        headers = curl_slist_append(headers, ("Dropbox-API-Arg: " + json_arg).c_str());

        headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            throw std::runtime_error(curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    //system("cls");
}