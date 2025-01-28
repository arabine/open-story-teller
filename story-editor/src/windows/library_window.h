#pragma once

#include <thread>
#include "window_base.h"
#include "library_manager.h"
#include "i_story_manager.h"
#include "thread_safe_queue.h"
#include "downloader.h"

#include <curl/curl.h>

struct DownloadCommand {
    std::string order;
    std::string url;
    std::string filename;
    std::function<void(bool, const std::string &filename)> finished_callback;
};


struct TransferProgress {
    long total;
    long current;

    TransferProgress() {
        total = 0;
        current = 0;
    }

    void Reset() {
        total = 0;
        current = 0;
    }

    TransferProgress(const TransferProgress &other) {
       *this = other;
    }

    ~TransferProgress() {
    }

    TransferProgress& operator=(const TransferProgress &other)
    {
        this->current = other.current;
        this->total = other.total;
        return *this;
    }

    TransferProgress(int t, int c) {
        total = t;
        current = c;
    }

    void Set(int t, int c)
    {
        total = t;
        current = c;
    }

    float Precent() const {
        return total == 0 ? 0.0 : static_cast<float>(current) / static_cast<float>(total);
    }
};

class LibraryWindow : public WindowBase
{
public:
    LibraryWindow(IStoryManager &project, LibraryManager &library);

    void Initialize();
    virtual void Draw() override;

    ~LibraryWindow();
private:
    IStoryManager &m_storyManager;
    LibraryManager &m_libraryManager;

    Downloader m_downloader;
    CURL *m_curl;
    char m_storeUrl[1024];
    std::thread m_downloadThread;
    ThreadSafeQueue<DownloadCommand> m_downloadQueue;
    ThreadSafeQueue<TransferProgress> m_transferProgress;
    std::mutex m_downloadMutex;
    bool m_cancel{false};

    std::mutex m_downloadBusyMutex;
    bool m_downloadBusy{false};
    
    std::string m_communityStoreFile;
    std::string m_commercialStoreFile;

    std::string m_storeRawJson;
    void ParseCommercialStoreDataCallback(bool success, const std::string &filename);
    void ParseCommunityStoreDataCallback(bool success, const std::string &filename);
    void StoryFileDownloadedCallback(bool success, const std::string &filename);
    void DownloadThread();
    int TransferCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

    std::string ToLocalStoreFile(const std::string &url);
    bool CheckIfSharepoint(const std::string &url, std::string &decoded_url);
    void SharePointJsonDownloadedCallback(bool success, const std::string &filename);
};

