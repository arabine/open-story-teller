#pragma once

#include <list>
#include <string>
#include "connection.h"
#include "story_options.h"
#include "variable.h"

class IStoryProject
{
public:

    enum Type
    {
        PROJECT_TYPE_STORY = 0,
        PROJECT_TYPE_MODULE = 1,
        PROJECT_TYPE_PRIMITIVE = 2
    };

    struct FunctionInfo {
        std::string uuid;
        std::string name;
    };

    virtual ~IStoryProject() {};

    virtual std::string GetName() const = 0;
    virtual std::string GetDescription() const = 0;
    virtual Type GetProjectType() const = 0;
    virtual bool IsModule() const = 0;
    virtual std::string GetUuid() const = 0;
    virtual std::string GetTitleImage() const = 0;
    virtual std::string GetTitleSound() const = 0;
    virtual std::vector<FunctionInfo> GetFunctionsList() const = 0;

    virtual void SetTitleImage(const std::string &titleImage) = 0;
    virtual void SetTitleSound(const std::string &titleSound) = 0;
    virtual void SetDescription(const std::string &description) = 0;
    virtual void SetProjectType(Type type) = 0;
    virtual void SetImageFormat(Resource::ImageFormat format) = 0;
    virtual void SetSoundFormat(Resource::SoundFormat format) = 0;
    virtual void SetDisplayFormat(int w, int h) = 0;
    virtual void SetName(const std::string &name) = 0;
    virtual void SetUuid(const std::string &uuid) = 0;

    // Callback can return false to break the loop
    virtual void ScanVariable(const std::function<bool(std::shared_ptr<Variable> element)>& operation) = 0;
    virtual void AddVariable() = 0;
    virtual void DeleteVariable(int i) = 0;

};



