// #include <mbedtls/aes.h>
#include "downloader.h"
#include "json.hpp"

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Downloader::Downloader()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Try to download the store index file
    m_curl = curl_easy_init();

    m_downloadThread = std::thread( std::bind(&Downloader::DownloadThread, this) );
}

Downloader::~Downloader()
{
    if (m_curl)
    {
        m_downloadMutex.lock();
        m_cancel = true;
        m_downloadMutex.unlock();
    }

    // Quit download thread
    m_downloadQueue.push({"quit", ""});
    if (m_downloadThread.joinable())
    {
        m_downloadThread.join();
    }

    curl_global_cleanup();
}

void Downloader::DownloadThread()
{
    for (;;)
    {      
        auto cmd = m_downloadQueue.front();
        m_downloadQueue.pop();

        if (cmd.order == "quit")
        {
            curl_easy_cleanup(m_curl);

            return;
        }
        else if (cmd.order == "dl")
        {
         //   download_file(m_curl, cmd.url, cmd.filename, cmd.finished_callback);
        }
    }
}

std::string Downloader::FetchToken()
{
    std::string url = "https://server-auth-prod.lunii.com/guest/create";
    std::string response = PerformGetRequest(url);

    // Parse the response JSON to extract the token
    try {
        auto jsonResponse = nlohmann::json::parse(response);
        return jsonResponse["response"]["token"]["server"].get<std::string>();
    } catch (const std::exception& ex) {
        throw std::runtime_error("Failed to parse token from response: " + std::string(ex.what()));
    }
}

void Downloader::FetchDataAndSaveToFile(const std::string& token, const std::string& filePath)
{
    std::string url = "https://server-data-prod.lunii.com/v2/packs";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("X-AUTH-TOKEN: " + token).c_str());

    std::string response = PerformGetRequest(url, headers);

    // Enregistrer dans un fichier
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing");
    }
    file << response;
    file.close();
    std::cout << "Data saved to " << filePath << std::endl;
}

std::string Downloader::PerformGetRequest(const std::string& url, struct curl_slist* headers)
{

    std::string response;
    curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &response);

    if (headers)
    {
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(m_curl);
    if (res != CURLE_OK)
    {
        // FIXME: handle error
    }

    if (headers)
    {
        curl_slist_free_all(headers);
    }

    return response;
}


