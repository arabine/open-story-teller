#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <thread>
#include <functional>

#include <curl/curl.h>

#include "thread_safe_queue.h"


class Downloader
{
public:

    struct Command {
        std::string order;
        std::string url;
        std::string filename;
        std::function<void(bool, const std::string &filename)> finished_callback;
    };  

    Downloader();
    ~Downloader();

    std::string FetchToken();

    void FetchDataAndSaveToFile(const std::string& token, const std::string& filePath);

private:
    CURL *m_curl;
    std::thread m_downloadThread;
    ThreadSafeQueue<Downloader::Command> m_downloadQueue;
    std::mutex m_downloadMutex;
    bool m_cancel{false};

    std::string PerformGetRequest(const std::string& url, struct curl_slist* headers = nullptr);
    void DownloadThread();
};