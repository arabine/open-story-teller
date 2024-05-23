#pragma once

#include <string>
#include <memory>
#include <random>
#include <string>

#include "json.hpp"
#include "i_story_project.h"

class BaseNode
{
public:
    struct NodePosition
    {
        float x;
        float y;
    };

    BaseNode(const std::string &type);
    virtual ~BaseNode();

    static std::string GetEntryLabel(const std::string &id);

    virtual void Initialize() = 0;
    virtual std::string Build(IStoryProject &story, int nb_out_conns) = 0;
    virtual std::string GenerateConstants(IStoryProject &story, int nb_out_conns) = 0;

    void SetPosition(float x, float y);

    // make this virtual so that graphical node override the behavior
    virtual float GetX() const;
    virtual float GetY() const;

    std::string GetType() const
    {
        return m_type;
    }

    void SetId(const std::string &id) { m_uuid = id; }
    std::string GetId() const { return m_uuid; }

    void SetTitle(const std::string &title) { m_title = title; }
    std::string GetTitle() const { return m_title; }

    void FromJson(const nlohmann::json &);
    nlohmann::json ToJson() const;


    void SetInternalData(const nlohmann::json &j);
    nlohmann::json GetInternalData() const;


private:
    std::string m_title{"Default title"};
    std::string m_type;
    std::string m_uuid;
    NodePosition m_pos;

    nlohmann::json m_internal_data{{}};
};

