#pragma once

#include "i_story_project.h"


class StoryPrimitive : public IStoryProject
{

public:
    StoryPrimitive(const std::string &name, const std::string &description = "", const std::string &uuid = "00000000-0000-0000-0000-000000000000")
        : m_name(name)
        , m_description(description)
        , m_uuid(uuid)
    {

    }
    virtual ~StoryPrimitive() {};

    virtual std::string GetName() const
    {
        return m_name;
    }

    virtual std::string GetDescription() const
    {
        return m_description;
    }

    virtual bool IsModule() const
    {
        return false; // This is not a module, it's a primitive
    }

    virtual std::string GetUuid() const
    {
        return m_uuid;
    }

    virtual std::vector<FunctionInfo> GetFunctionsList() const override {
        return std::vector<FunctionInfo>();
    }


    virtual std::string GetTitleImage() const {
        return ""; // No title image by default
    }
    virtual std::string GetTitleSound() const {
        return ""; // No title sound by default
    }
    virtual Type GetProjectType() const {
        return Type::PROJECT_TYPE_PRIMITIVE;
    }
    virtual void SetTitleImage(const std::string &titleImage) {
        (void)titleImage; // No title image to set
    }
    virtual void SetTitleSound(const std::string &titleSound) {
        (void)titleSound; // No title sound to set
    }
    virtual void SetDescription(const std::string &description) {
        m_description = description;
    }
    virtual void SetProjectType(Type type) {
        (void)type; // Project type is fixed for primitives
    }
    virtual void SetImageFormat(Resource::ImageFormat format) {
        (void)format; // Image format is fixed for primitives
    }
    virtual void SetSoundFormat(Resource::SoundFormat format) {
        (void)format; // Sound format is fixed for primitives
    }
    virtual void SetDisplayFormat(int w, int h) {
        (void)w; // Display width is fixed for primitives
        (void)h; // Display height is fixed for primitives
    }
    virtual void SetName(const std::string &name) {
        m_name = name;
    }
    virtual void SetUuid(const std::string &uuid) {
        m_uuid = uuid;
    }

    virtual void ScanVariable(const std::function<bool(std::shared_ptr<Variable> element)>& operation) override {

    }
    virtual void AddVariable() override {
        
    }
    virtual void DeleteVariable(int i) override {
        
    }

private:
    std::string m_name;
    std::string m_description;
    std::string m_uuid;
    std::string m_titleImage;
    std::string m_titleSound;
};