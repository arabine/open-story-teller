#ifndef STORY_PROJECT_H
#define STORY_PROJECT_H

#include <vector>
#include <string>

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


    std::string m_working_dir;
    std::vector<StoryNode> m_nodes;

    std::string m_type;
    std::string m_code;
    std::string m_name;

    std::vector<Resource> m_images;
    std::vector<Resource> m_sounds;

    StoryNode *m_tree;

    bool Load(const std::string &file_path);
    void CreateTree();
    void Clear() {
        m_images.clear();
        m_sounds.clear();
    }

    std::string Compile();

    static std::string GetFileExtension(const std::string &FileName);
    static std::string GetFileName(const std::string &path);
    static void ReplaceCharacter(std::string &theString, const std::string &toFind, const std::string &toReplace);


public:
    static void EraseString(std::string &theString, const std::string &toErase);
    static std::string ToUpper(const std::string &input);
private:

};

#endif // STORY_PROJECT_H
