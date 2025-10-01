// events/vm_state_event.h
#ifndef ALL_EVENTS_H
#define ALL_EVENTS_H

#include "event.h"
#include <string>
#include <set>
#include <chip32_vm.h> // Pour chip32_ctx_t

struct VmStateEvent : public Event
{
    chip32_ctx_t vmContext;
    int currentLine;
    chip32_result_t vmResult;
    std::set<int> breakpoints;
    // Ajoutez d'autres données nécessaires, ex: std::vector<std::shared_ptr<Variable>> variables;

    // Constructeur pour faciliter la création de l'événement
    VmStateEvent(const chip32_ctx_t& ctx, int line, chip32_result_t result, const std::set<int>& bps)
        : vmContext(ctx), currentLine(line), vmResult(result), breakpoints(bps) {}
};

class OpenProjectEvent : public Event
{
public:
    OpenProjectEvent(const std::string &uuid) : m_uuid(uuid) {}
    const std::string& GetUuid() const { return m_uuid; }
private:
    std::string m_uuid; // UUID du projet à ouvrir
};

class OpenFunctionEvent : public Event
{
public:
    OpenFunctionEvent(const std::string &uuid, const std::string &name)
        : m_uuid(uuid), m_name(name) {}
    const std::string& GetUuid() const { return m_uuid; }
    const std::string& GetName() const { return m_name; }
private:
    std::string m_uuid; // UUID du projet ou module
    std::string m_name; // Nom de la fonction à ouvrir
};

class GenericResultEvent : public Event
{
public:
    GenericResultEvent(bool success, const std::string& message)
        : m_success(success), m_message(message) {}

    bool IsSuccess() const { return m_success; }
    const std::string& GetMessage() const { return m_message; }

private:
    bool m_success;
    std::string m_message;
};

class ModuleEvent : public Event
{
public:
    enum class Type
    {
        Opened,
        Closed,
        BuildSuccess,
        BuildFailure
    };

    ModuleEvent(Type type, const std::string &uuid)
        : m_type(type), m_uuid(uuid) {}

    Type GetType() const { return m_type; }
    const std::string& GetUuid() const { return m_uuid; }

    const std::string& GetScript() const { return m_script; }
    bool IsSuccess() const { return success; }

    void SetScript(const std::string& script) { m_script = script; }
    void SetSuccess(bool s) { success = s; }

private:
    Type m_type;
    std::string m_uuid;
    std::string m_script;
    bool success{false};
};

#endif // ALL_EVENTS_H