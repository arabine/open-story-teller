// core/story-manager/src/nodes/wait_event_node.h
#pragma once

#include <string>
#include "base_node.h"
#include "i_story_project.h"
#include "variable.h"

class WaitEventNode : public BaseNode
{
public:
    WaitEventNode(const std::string &type = "wait-event-node");

    void Initialize() override;

    // Event mask (bits pour différents types d'événements)
    uint32_t GetEventMask() const { return m_eventMask; }
    void SetEventMask(uint32_t mask);
    
    // Timeout en millisecondes (0 = infini)
    uint32_t GetTimeout() const { return m_timeout; }
    void SetTimeout(uint32_t ms);
    
    // Variable de destination pour le code de retour
    std::string GetResultVariableUuid() const { return m_resultVariableUuid; }
    void SetResultVariable(const std::string& varUuid);
    
    // Helper pour résoudre la variable (utilisé par TAC Generator)
    std::shared_ptr<Variable> ResolveResultVariable(const std::vector<std::shared_ptr<Variable>>& variables) const;

private:
    uint32_t m_eventMask{0b10000000000};  // Défaut: OK + Home + EndAudio
    uint32_t m_timeout{0};                 // 0 = infini
    std::string m_resultVariableUuid;
};