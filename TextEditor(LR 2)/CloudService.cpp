#include "CloudService.h"

std::string CloudService::access_token = "sl.u.AFrDNZSpppVAsg0Lk1_sfx4GnFdPhy3p4d9o7QaMKE9UPs-KPondCVi4G0PyCnp4hVaUFpG_L22abEGqyCkqI6VbsLz-2GemhC7igz6nbvWxkQl0FmvCOJBwwWqxuPuTanFy4s3or8-NkbR6vFemRUhUWGjgLL7YJ-NOKVUzdrgPwT6wfwIMs1kfG9EBM13ANjyo9rvNT1kwZ3TW_CIQNhRzleFlaK_qHuGHAUZpybB5uycITuIF6tJSTraAYjkSA9lodUJsbOGgPSN7G3O3GcomOG92iyCGrUzvGq5LArlRx1tT47BE1t9mhac0m6zD0BgkwGJlYP7AwXgIlWf49xBhoLOlykuxlyysmfvrq9mQYSlMevldwLDiM_Uo5hvRM3Hv3CSIzM2kPEEeaAdMHNqwiIhKYw055vws4yWbCy1l9qAdXBmaGC77H3jljGG5dcs4Nm0l-ycULWxdaNar0J3GtQUY1ZZgKy1nKhXeKqxbr0bCVRLRTvbYpxmx1321hf7oNZMejfLnd9Ud8GiG2p2u0saZUWAQZ1AsXD2f6bSbpMgMRvXOaXNi2R4HVuk8tUzqp9_f47F9fHuCBEr1bwwHhuHoboALv9_Jxd1lJNxziWppMrF-a7-80g6lxQiZKnJxRA8mzpklKX_exH0x-33M2nh_aklMRutLZQ9iSpeNijedVrhv71mA_Jy7RBPLfM3AUhAjbCxF-vRKEDJ9YWxhYPgxi1-0S8j21YJEjKB3NvhrRaOZLWfbsGcn9MtayIf6bhWinP7dTfjts0PAWdqxAJhKX9_AUjSPELXBvtKktTHoEiM2pWAU7On45_CRWzC_Ivwd-jA8EuZRREAAvFoypw5LsW1aqhrfGbhwrIzvvDyEVrAoqhISqh6G-ZcZIulk31lPyONuOCiSbPT6xbMyPMhBSEsAUCBPnDEz3L9pFGz0u5BKBY6DIWKrv3EmUKMojB1uQkmWu-pQRRv-G0S_2XAkPWBzCLdajhzznvuiX3t8QTFvOnRdfoXWrjEWWnLXmaQlLN-mR9jwIyS1_MlBs9iJH_wBnI-dBN5icXAvNyqU0Pd5BUQ7iTsccRHbFqt5EBXLCGxDvG6yKppY8bOqfzlrxpNB7ZoBD8TaRIyv1Om2b6oGLDnS4llTrNk4CR_gP7c-P4aT-A03LFp0WIOtVHQudE0EPy2IdgbV6H4g2JgvWQ4U21bb7aoKbtOprnAYbop9ftCBoo1HcU5MjGnTNw5aSfB9PJoF9O73H_xCs5rqcvHeVS6N43Tuaz-iMmznHWzOPdxJja1A1nRM45Te4VrWM5ulthP8LWYNPv5lHtay1yTx6yFK5t__9xg0zIWN6ytWH1lJqBaeoofilRSu-ewrBj4N72DSbGC1i2G00-uvKb6Yn2cgX2fAKO2Ekq7fPSY29WMsp-IRBdEljSOU-oNotqUWDYCHhn0m8D1fKA";
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
        while(i >= 0&&filename[i]!='/')
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