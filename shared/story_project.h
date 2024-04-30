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

// FIXME : Structure très Lunii style, à utiliser pour la conversion peut-être ...
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
    enum ImageFormat { IMG_FORMAT_BMP_4BITS, IMG_FORMAT_QOIF, IMG_FORMAT_COUNT };
    enum SoundFormat { SND_FORMAT_WAV, SND_FORMAT_QOAF, SND_FORMAT_COUNT };

    StoryProject();
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
    bool Build(std::string &codeStr);
    bool Load(ResourceManager &manager);
    void Save(ResourceManager &manager);
    void SaveBinary(const std::vector<uint8_t> &m_program);
    void SetPaths(const std::string &uuid, const std::string &library_path);
    void CopyToDevice(const std::string &outputDir);

    void ModelToJson(nlohmann::json &model);
    bool ModelFromJson(const nlohmann::json &model);

    void CreateTree();
    void Clear();


    std::pair<std::list<std::shared_ptr<BaseNode>>::iterator, std::list<std::shared_ptr<BaseNode>>::iterator> Nodes() {
        return std::make_pair(m_nodes.begin(), m_nodes.end());
    }

    std::pair<std::list<std::shared_ptr<Connection>>::iterator, std::list<std::shared_ptr<Connection>>::iterator> Links() {
        return std::make_pair(m_links.begin(), m_links.end());
    }

    void Select(bool selected) { m_selected = selected; }
    bool IsSelected() const { return m_selected; }
    
    void SetImageFormat(ImageFormat format);
    void SetSoundFormat(SoundFormat format);
    void SetDisplayFormat(int w, int h);
    void SetName(const std::string &name) { m_name = name; }
    void SetUuid(const std::string &uuid) { m_uuid = uuid; }

    std::string GetProjectFilePath() const;
    std::string GetWorkingDir() const;
    std::string GetName() const { return m_name; }
    std::string GetUuid() const { return m_uuid; }
    std::string GetDescription() const { return m_description; }
    int GetVersion() const { return m_version; }

    std::string BuildFullAssetsPath(const std::string &fileName) const;

    std::filesystem::path AssetsPath() const { return m_assetsPath; }

    static std::string GetFileExtension(const std::string &FileName);
    static std::string GetFileName(const std::string &path);
    static std::string RemoveFileExtension(const std::string &FileName);
    static void ReplaceCharacter(std::string &theString, const std::string &toFind, const std::string &toReplace);
    static std::string FileToConstant(const std::string &FileName, const std::string &extension);
    static std::string Normalize(const std::string &input);

    void SetTitleImage(const std::string &titleImage);
    void SetTitleSound(const std::string &titleSound);

    std::string GetTitleImage() const { return m_titleImage; }
    std::string GetTitleSound() const { return m_titleSound; }

    // Initialize with an existing project
    const bool IsInitialized() const { return m_initialized; }

    static void EraseString(std::string &theString, const std::string &toErase);
    static std::string ToUpper(const std::string &input);

    bool ParseStoryInformation(nlohmann::json &j);
   
    // From IStoryProject
    virtual std::list<std::shared_ptr<Connection>> GetNodeConnections(const std::string &nodeId) override;
    std::string FindFirstNode() const;
    virtual int OutputsCount(const std::string &nodeId) override;

    std::shared_ptr<BaseNode> CreateNode(const std::string& type);
    void AddConnection(std::shared_ptr<Connection> c);
    void DeleteNode(const std::string &id);
    void DeleteLink(std::shared_ptr<Connection> c);
    
private:
    // Project properties and location
    std::string m_name; /// human readable name
    std::string m_uuid;
    std::string m_titleImage;
    std::string m_titleSound;
    std::string m_description;
    int m_version;
    bool m_selected{false};

    std::filesystem::path m_assetsPath;

    // Model in memory
    std::list<std::shared_ptr<Connection>> m_links;
    std::list<std::shared_ptr<BaseNode>> m_nodes;

    bool m_initialized{false};

    std::filesystem::path m_working_dir; /// Temporary folder based on the uuid, where the archive is unzipped
    std::filesystem::path m_project_file_path; /// JSON project file

    int m_display_w{320};
    int m_display_h{240};

    ImageFormat m_imageFormat{IMG_FORMAT_BMP_4BITS};
    SoundFormat m_soundFormat{SND_FORMAT_WAV};


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


