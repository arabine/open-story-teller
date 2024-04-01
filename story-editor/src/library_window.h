#pragma once

#include <thread>
#include "window_base.h"
#include "library_manager.h"
#include "i_story_manager.h"
#include "thread_safe_queue.h"

#include <curl/curl.h>

struct StoryInf {
    int age;
    std::string title;
};

struct DownloadCommand {
    std::string order;
    std::string url;
    std::string filename;
    std::function<void(bool)> finished_callback;
};


struct TransferProgress {
    long total;
    long current;

    TransferProgress() {
        total = 0;
        current = 0;
    }

    TransferProgress(int t, int c) {
        total = t;
        current = c;
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

    CURL *m_curl;
    char m_store_url[1024];
    std::thread m_downloadThread;
    ThreadSafeQueue<DownloadCommand> m_downloadQueue;
    ThreadSafeQueue<TransferProgress> m_transferProgress;
    std::mutex m_downloadMutex;
    bool m_cancel{false};

    std::vector<StoryInf> m_store;
    std::string m_storeIndexFilename;

    std::string m_storeRawJson;
    void ParseStoreData(bool success);
    void DownloadThread();
    int TransferCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

    std::string ToLocalStoreFile(const std::string &url);
};

