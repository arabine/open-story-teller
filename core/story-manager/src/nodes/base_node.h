#pragma once

#include <string>
#include <memory>
#include <random>
#include <string>

#include "json.hpp"
#include "i_story_page.h"
#include "i_story_project.h"
#include "story_options.h"

class BaseNode
{
public:
    enum Behavior
    {
        BEHAVIOR_EXECUTION,
        BEHAVIOR_DATA,
    };

    struct Port
    {
        enum Type {
            EXECUTION_PORT,
            DATA_PORT
        };

        Port::Type type{EXECUTION_PORT};
        std::string label;
    };

    struct NodePosition
    {
        float x;
        float y;
    };

    enum RandomFlags
    {
        CHARSET_ALPHABET_LOWER = 0x1, // "abcdefghijklmnopqrstuvwxyz"
        CHARSET_ALPHABET_UPPER = 0x2, // "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        CHARSET_NUMBERS = 0x4, // "0123456789"
        CHARSET_SIGNS = 0x8, // "!@#$%^&*()_+-=[]{}|;:,.<>?";
        ALL_CHARSETS = CHARSET_ALPHABET_LOWER | CHARSET_ALPHABET_UPPER |CHARSET_NUMBERS | CHARSET_SIGNS
    };

    struct ConstantValue
    {
        enum Type {
            TYPE_STRING,
            TYPE_INT,
        };
        std::string stringVal;
        int intVal{0};
        ConstantValue::Type type{TYPE_INT};
    };

    struct Constant
    {
        std::string label;
        int elementSize; // in bits: 8, 16 or 32
        std::vector<ConstantValue> values;
    };

    BaseNode(const std::string &type, const std::string &typeName, Behavior behavior = BEHAVIOR_EXECUTION);
    virtual ~BaseNode();

    static std::string GetEntryLabel(const std::string &id);

    virtual void Initialize() = 0;
    virtual std::string Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns) = 0;
    virtual std::string GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns) = 0;

    virtual std::string GenerateConstants() const { return ""; }
    virtual std::string GenerateAssembly() const = 0;

    void SetPosition(float x, float y);

    // make this virtual so that graphical node override the behavior
    virtual float GetX() const;
    virtual float GetY() const;

    std::string GetMyEntryLabel() const {
        return GetEntryLabel(m_uuid);
    }

    // Coded type, internal use
    std::string GetType() const
    {
        return m_type;
    }

    // Human readable type
    std::string GetTypeName() const
    {
        return m_typeName;
    }

    void SetId(const std::string &id) { m_uuid = id; }
    std::string GetId() const { return m_uuid; }

    void SetTitle(const std::string &title) { m_title = title; }
    std::string GetTitle() const { return m_title; }

    void FromJson(const nlohmann::json &);
    nlohmann::json ToJson() const;


    void SetInternalData(const nlohmann::json &j);
    nlohmann::json GetInternalData() const;

    static std::string GenerateRandomString(size_t length, uint32_t flags = RandomFlags::ALL_CHARSETS);

    void ClearPorts() {
        m_inputPorts.clear();
        m_outputPorts.clear();
    }

    // Port management
    void AddInputPort(Port::Type type, const std::string& label) {
        m_inputPorts.push_back({type, label});
    }

    void AddOutputPort(Port::Type type, const std::string& label) {
        m_outputPorts.push_back({type, label});
    }
    

    void SetBehavior(Behavior behavior) {
        m_behavior = behavior;
    }
    Behavior GetBehavior() const {
        return m_behavior;
    }

private:
    std::string m_title{"Default title"};
    std::string m_type;
    std::string m_typeName;
    std::string m_uuid;
    NodePosition m_pos;
    Behavior m_behavior{BEHAVIOR_EXECUTION};

    std::vector<Port> m_inputPorts;
    std::vector<Port> m_outputPorts;

    nlohmann::json m_internal_data{{}};
};

