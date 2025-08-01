#ifndef STORY_PROJECT_H
#define STORY_PROJECT_H

#include <vector>
#include <string>
#include <filesystem>
#include <unordered_set>
#include "json.hpp"

#include "json.hpp"
#include "resource_manager.h"
#include "connection.h"
#include "base_node.h"
#include "i_story_project.h"
#include "chip32_assembler.h"
#include "story_page.h"
#include "story_options.h"
#include "variable.h"

class NodesFactory;

struct StoryProject : public std::enable_shared_from_this<StoryProject>, public IStoryProject
{

public:
    StoryProject(ILogger &log);
    ~StoryProject();

    bool *Selected() {
        return &m_selected;
    }

    // std::shared_ptr<StoryProject> shared_from_this() {
    //     return shared_from_this();
    // }

    std::string MainUuid() const {
        return "490745ab-df4d-476d-ae27-027e94b8ee0a";
    }

    void New(const std::string &uuid, const std::string &library_path);
    std::filesystem::path BinaryFileName() const;
    bool GenerateScript(std::string &codeStr);
    bool GenerateBinary(const std::string &code, Chip32::Assembler::Error &err);
    bool Load(ResourceManager &manager, NodesFactory &factory);
    void Save(ResourceManager &manager);
    void SaveBinary();
    void SetPaths(const std::string &uuid, const std::string &library_path);
    void CopyToDevice(const std::string &outputDir, NodesFactory &factory);

    void ModelToJson(nlohmann::json &model);
    bool ModelFromJson(const nlohmann::json &model, NodesFactory &factory);

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
    virtual std::string GetName() const override { return m_name; }
    std::string GetUuid() const { return m_uuid; }
    std::string GetDescription() const { return m_description; }
    uint32_t GetVersion() const { return m_version; }

    std::string BuildFullAssetsPath(const std::string_view fileName) const;

    std::filesystem::path AssetsPath() const { return m_assetsPath; }

    void SetTitleImage(const std::string &titleImage);
    void SetTitleSound(const std::string &titleSound);
    void SetDescription(const std::string &description) { m_description = description; }

    std::string GetTitleImage() const { return m_titleImage; }
    std::string GetTitleSound() const { return m_titleSound; }

    // Initialize with an existing project
    bool IsInitialized() const { return m_initialized; }
    bool IsModule() const override { return m_projectType == IStoryProject::PROJECT_TYPE_MODULE; }
    bool IsStory() const { return m_projectType == IStoryProject::PROJECT_TYPE_STORY; }
    void SetProjectType(IStoryProject::Type type) { m_projectType = type; }

    bool ParseStoryInformation(nlohmann::json &j);
   
    // From IStoryProject
    virtual std::list<std::shared_ptr<Connection>> GetNodeConnections(const std::string &nodeId);
    virtual int OutputsCount(const std::string &nodeId);
    StoryOptions GetOptions() { return m_storyOptions; }
    virtual bool UseResource(const std::string &label);

    // Node interaction
    std::shared_ptr<StoryPage> CreatePage(const std::string &uuid);
    std::shared_ptr<StoryPage> GetPage(const std::string &uuid);
    void AddNode(const std::string_view &page, std::shared_ptr<BaseNode> node);

    void AddConnection(const std::string_view &page, std::shared_ptr<Connection> c);
    void DeleteNode(const std::string_view &page, const std::string &id);
    void DeleteLink(const std::string_view &page, std::shared_ptr<Connection> c);
    std::pair<std::list<std::shared_ptr<BaseNode>>::iterator, std::list<std::shared_ptr<BaseNode>>::iterator> Nodes(const std::string_view &page_uuid);
    std::pair<std::list<std::shared_ptr<Connection>>::iterator, std::list<std::shared_ptr<Connection>>::iterator> Links(const std::string_view &page_uuid);


    void ScanVariable(const std::function<void(std::shared_ptr<Variable> element)>& operation);
    void AddVariable();
    void DeleteVariable(int i);
    
private:
    ILogger &m_log;

    // Project properties and location
    std::string m_name; /// human readable name
    std::string m_uuid;
    std::string m_titleImage;
    std::string m_titleSound;
    std::string m_description;

    IStoryProject::Type m_projectType{IStoryProject::PROJECT_TYPE_STORY};

    uint32_t m_version;
    bool m_selected{false};

    std::unordered_set<std::string> m_usedLabels; // permet de ne pas générer un label qui existe déjà

    std::filesystem::path m_assetsPath;

    Chip32::Assembler m_assembler;
    std::vector<uint8_t> m_program;

    std::list<std::shared_ptr<StoryPage>> m_pages;

    std::vector<std::shared_ptr<Variable>> m_variables;

    StoryOptions m_storyOptions;

    bool m_initialized{false};

    std::filesystem::path m_working_dir; /// Temporary folder based on the uuid, where the archive is unzipped
    std::filesystem::path m_project_file_path; /// JSON project file
};

#endif // STORY_PROJECT_H


