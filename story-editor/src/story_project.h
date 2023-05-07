#ifndef STORY_PROJECT_H
#define STORY_PROJECT_H

#include <vector>
#include <string>
#include <filesystem>

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

struct Resource
{
    std::string file;
    std::string description;
    std::string format;
};

struct StoryProject
{
    enum ImageFormat { IMG_FORMAT_BMP_4BITS, IMG_FORMAT_QOIF, IMG_FORMAT_COUNT };
    enum SoundFormat { SND_FORMAT_WAV, SND_FORMAT_QOAF, SND_FORMAT_COUNT };

    // Project properties and location
    std::string name; /// human readable name
    std::vector<StoryNode> m_nodes;

    std::string m_type;
    std::string m_code;

    std::vector<Resource> m_images;
    std::vector<Resource> m_sounds;

    StoryNode *m_tree;


    bool Load(const std::string &file_path);
    void CreateTree();
    void Clear() {
        m_uuid = "";
        m_working_dir = "";
        m_images.clear();
        m_sounds.clear();

        m_initialized = false;
    }

    std::string Compile();
    void SetImageFormat(ImageFormat format);
    void SetSoundFormat(SoundFormat format);
    void SetDisplayFormat(int w, int h);

    std::string GetProjectFilePath() const;
    std::string GetWorkingDir() const;

    std::filesystem::path ImagesPath() const { return m_imagesPath; }
    std::filesystem::path SoundsPath() const { return m_soundsPath; }

    static std::string GetFileExtension(const std::string &FileName);
    static std::string GetFileName(const std::string &path);
    static void ReplaceCharacter(std::string &theString, const std::string &toFind, const std::string &toReplace);

public:
    // Initialize with an existing project
    void Initialize(const std::string &file_path);
    void New(const std::string &uuid, const std::string &file_path);

    static void EraseString(std::string &theString, const std::string &toErase);
    static std::string ToUpper(const std::string &input);

private:
    std::string m_uuid;
    std::filesystem::path m_imagesPath;
    std::filesystem::path m_soundsPath;
    bool m_initialized{false};

    std::string m_working_dir; /// Temporary folder based on the uuid, where the archive is unzipped
    std::string m_project_path; /// JSON project file

    int m_display_w{320};
    int m_display_h{240};

    ImageFormat m_imageFormat{IMG_FORMAT_BMP_4BITS};
    SoundFormat m_soundFormat{SND_FORMAT_WAV};
};

#endif // STORY_PROJECT_H
