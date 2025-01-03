#pragma once

#include <string>

class IStoryDb
{

public:
    struct Info {
        int age;
        std::string title;
        std::string description;
        std::string download;
        std::string image_url;
        std::string sound;
        std::string uuid;
        std::string url;
    };

    virtual ~IStoryDb() {}


    virtual bool FindStory(const std::string &uuid, Info &info) = 0;
    virtual void AddStory(IStoryDb::Info &info, int origin) = 0;
};
