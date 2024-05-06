#pragma once

class Group
{
public:
    Group(Display &s)
        : mSystem(s)
    {
        mEntityIds = 0;

        mOrigin.x = 0;
        mOrigin.y = 0;
    }

    // Called when scene initially created. Called once.
    virtual void OnCreate(SDL_Renderer *renderer);
    // Called when scene destroyed. Called at most once (if a scene
    // is not removed from the game, this will never be called).
    virtual void OnDestroy() {}

    // The below functions can be overridden as necessary in our scenes.
    virtual void ProcessEvent(const SDL_Event &event)
    {
        for (auto &e : mEntities)
        {
            e->ProcessEvent(event, mOrigin);
        }
    };

    virtual void Update(double deltaTime)
    {
        for (auto &e : mEntities)
        {
            e->Update(deltaTime);
        }
    };

    virtual void Draw(SDL_Renderer *renderer)
    {
        for (auto &e : mEntities)
        {
            e->Draw(renderer, mOrigin.x, mOrigin.y);
        }
    };

    uint32_t AddEntity(std::shared_ptr<Entity> entity)
    {
        uint32_t id = mEntityIds;
        entity->SetId(id);
        mEntities.push_back(entity);
        mEntityIds++;
        return id;
    }

    void DeleteEntity(uint32_t id)
    {
        mEntities.remove_if([id](const std::shared_ptr<Entity> &e)
                            { return e->GetId() == id; });
    }

    // Sort according to Z value of each entity
    void Sort()
    {
        mEntities.sort([](const std::shared_ptr<Entity> &a, const std::shared_ptr<Entity> &b)
                       { return a->GetZ() < b->GetZ(); });
    }

    Vector2 GetOrigin() const
    {
        return mOrigin;
    }

    void SetOrigin(int x, int y)
    {
        mOrigin.x = x;
        mOrigin.y = y;
    }

    Display &GetSystem() { return mSystem; }

    void NewGrid() { mGrid.clear(); }
    void NewLine() { mGrid.push_back(std::list<std::shared_ptr<Entity>>()); }
    void AddToLine(std::shared_ptr<Entity> entity)
    {
        if (mGrid.size() > 0)
        {
            mGrid[mGrid.size() - 1].push_back(entity);
        }
    }

    uint32_t GetGridW() const { return mGridW; }
    uint32_t GetGridH() const { return mGridH; }

private:
    Display &mSystem;
    uint32_t mEntityIds = 0;
    Vector2 mOrigin;
    std::list<std::shared_ptr<Entity>> mEntities;
    std::vector<std::list<std::shared_ptr<Entity>>> mGrid;

    uint32_t mGridW{0};
    uint32_t mGridH{0};
};

