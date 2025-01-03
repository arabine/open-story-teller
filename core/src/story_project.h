#ifndef STORY_PROJECT_H
#define STORY_PROJECT_H

#include <vector>
#include <string>
#include <filesystem>
#include "json.hpp"

#include "json.hpp"
#include "resource_manager.h"
#include "connection.h"
#include "base_node.h"
#include "i_story_project.h"
#include "chip32_assembler.h"
#include "story_page.h"
#include "story_options.h"

// FIXME : Structure très proche de la boiboite, à utiliser pour la conversion peut-être ...
struct StoryNode
{
    bool auto_jump;
    int sound;
    int image;
    int id;
    std::vector<int> jumps;

    std::vector<StoryNode *> children;

    StoryNode& operator=(const StoryNode& other) {
        this->auto_jump = other.auto_jump;
        this->sound = other.sound;
        this->image = other.image;
        this->id = other.id;

        this->jumps.clear();
        this->jumps = other.jumps;
        this->children = other.children;

        return *this;
    }

//            "auto_jump": false,
//            "id": 0,
//            "image": 0,
//            "jumps": [1],
//            "sound": 0
};



struct StoryProject : public IStoryProject
{

public:
    StoryProject(ILogger &log);
    ~StoryProject();

    bool *Selected() {
        return &m_selected;
    }
/*
    std::vector<StoryNode> m_nodes;

    std::string m_type;
    std::string m_code;

    StoryNode *m_tree;
*/
    void New(const std::string &uuid, const std::string &library_path);
    std::filesystem::path BinaryFileName() const;
    bool GenerateScript(std::string &codeStr);
    bool GenerateBinary(const std::string &code, Chip32::Assembler::Error &err);
    bool Load(ResourceManager &manager);
    void Save(ResourceManager &manager);
    void SaveBinary();
    void SetPaths(const std::string &uuid, const std::string &library_path);
    void CopyToDevice(const std::string &outputDir);

    void ModelToJson(nlohmann::json &model);
    bool ModelFromJson(const nlohmann::json &model);

    bool CopyProgramTo(uint8_t *memory, uint32_t size);

    // returns >= 0 on success
    bool GetAssemblyLine(uint32_t pointer_counter, uint32_t &line);

    void Clear();

    void Select(bool selected) { m_selected = selected; }
    bool IsSelected() const { return m_selected; }
    
    void SetImageFormat(Resource::ImageFormat format);
    void SetSoundFormat(Resource::SoundFormat format);

    void SetDisplayFormat(int w, int h);
    void SetName(const std::string &name) { m_name = name; }
    void SetUuid(const std::string &uuid) { m_uuid = uuid; }

    std::string GetProjectFilePath() const;
    std::string GetWorkingDir() const;
    std::string GetName() const { return m_name; }
    std::string GetUuid() const { return m_uuid; }
    std::string GetDescription() const { return m_description; }
    uint32_t GetVersion() const { return m_version; }

    std::string BuildFullAssetsPath(const std::string_view fileName) const;

    std::filesystem::path AssetsPath() const { return m_assetsPath; }

    void SetTitleImage(const std::string &titleImage);
    void SetTitleSound(const std::string &titleSound);

    std::string GetTitleImage() const { return m_titleImage; }
    std::string GetTitleSound() const { return m_titleSound; }

    // Initialize with an existing project
    const bool IsInitialized() const { return m_initialized; }

    bool ParseStoryInformation(nlohmann::json &j);
   
    // From IStoryProject
    virtual std::list<std::shared_ptr<Connection>> GetNodeConnections(const std::string &nodeId) override;
    virtual int OutputsCount(const std::string &nodeId) override;
    virtual StoryOptions GetOptions() override { return m_storyOptions; }

    // Node interaction
    std::shared_ptr<StoryPage> CreatePage(const std::string &uuid);
    std::shared_ptr<BaseNode> CreateNode(const std::string_view &page, const std::string &type);
    void AddConnection(const std::string_view &page, std::shared_ptr<Connection> c);
    void DeleteNode(const std::string_view &page, const std::string &id);
    void DeleteLink(const std::string_view &page, std::shared_ptr<Connection> c);
    std::pair<std::list<std::shared_ptr<BaseNode>>::iterator, std::list<std::shared_ptr<BaseNode>>::iterator> Nodes(const std::string_view &page_uuid);
    std::pair<std::list<std::shared_ptr<Connection>>::iterator, std::list<std::shared_ptr<Connection>>::iterator> Links(const std::string_view &page_uuid);

    std::vector<std::string> GetNodeTypes() const { 
        std::vector<std::string> l;
        for(auto const& imap: m_registry) l.push_back(imap.first);
        return l;
    }
    
private:
    ILogger &m_log;

    // Project properties and location
    std::string m_name; /// human readable name
    std::string m_uuid;
    std::string m_titleImage;
    std::string m_titleSound;
    std::string m_description;
    uint32_t m_version;
    bool m_selected{false};

    std::filesystem::path m_assetsPath;

    Chip32::Assembler m_assembler;
    std::vector<uint8_t> m_program;

    std::list<std::shared_ptr<StoryPage>> m_pages;

    StoryOptions m_storyOptions;

    bool m_initialized{false};

    std::filesystem::path m_working_dir; /// Temporary folder based on the uuid, where the archive is unzipped
    std::filesystem::path m_project_file_path; /// JSON project file

    template<class NodeType>
    struct Factory {
        static std::shared_ptr<BaseNode> create_func(const std::string &type) {
            return std::make_shared<NodeType>(type);
        }
    };

    typedef std::shared_ptr<BaseNode> (*GenericCreator)(const std::string &type);
    typedef std::map<std::string, GenericCreator> Registry;
    Registry m_registry;

    template<class Derived>
    void registerNode(const std::string& key) {
        m_registry.insert(typename Registry::value_type(key, Factory<Derived>::create_func));
    }

};

#endif // STORY_PROJECT_H


