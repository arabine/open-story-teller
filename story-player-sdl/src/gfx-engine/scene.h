#pragma once

#include "group.h"

class Scene : public Group
{
public:
    typedef std::pair<std::string, Value> Message;
    typedef std::deque<Message> MessageQueue;

    Scene(Display &s)
        : Group(s)
    {
    }

    // Called when scene initially created. Called once.
    virtual void OnCreate(SDL_Renderer *renderer)
    {
        Group::OnCreate(renderer);

        for (auto &g : mGroups)
        {
            g->OnCreate(renderer);
        }
    }
    // Called when scene destroyed. Called at most once (if a scene
    // is not removed from the game, this will never be called).
    virtual void OnDestroy()
    {
        Group::OnDestroy();

        for (auto &g : mGroups)
        {
            g->OnDestroy();
        }
    }

    // Called whenever a scene is transitioned into. Can be
    // called many times in a typical game cycle.
    virtual void OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args = std::map<std::string, Value>())
    {
        mSwitchToScene = 0;
        (void)renderer;
        mArgs = args;
    }
    // Called whenever a transition out of a scene occurs.
    // Can be called many times in a typical game cycle.
    virtual void OnDeactivate(){};

    // The below functions can be overridden as necessary in our scenes.
    virtual void ProcessEvent(const SDL_Event &event)
    {
        Group::ProcessEvent(event);

        for (auto &g : mGroups)
        {
            g->ProcessEvent(event);
        }
    };

    virtual void Draw(SDL_Renderer *renderer)
    {
        Group::Draw(renderer);

        for (auto &g : mGroups)
        {
            g->Draw(renderer);
        }
    };

    virtual void OnMessage(MessageQueue &message)
    {
        (void)message;
    }

    void SwitchToScene(uint32_t newScene, const std::map<std::string, Value> &args = std::map<std::string, Value>())
    {
        mSwitchToScene = newScene;
        mArgs = args;
    }

    uint32_t AddGroup(std::shared_ptr<Group> group)
    {
        uint32_t id = mGroupIds;
        mGroups.push_back(group);
        mGroupIds++;
        return id;
    }

    uint32_t GetNextScene()
    {
        return mSwitchToScene;
    }

    std::map<std::string, Value> GetArgs()
    {
        return mArgs;
    }

private:
    std::string mName;
    uint32_t mGroupIds;
    uint32_t mSwitchToScene = 0;
    std::map<std::string, Value> mArgs;
    std::vector<std::shared_ptr<Group>> mGroups;
};
