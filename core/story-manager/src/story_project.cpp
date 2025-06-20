

#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>

#include "story_project.h"
#include "json.hpp"
// #include "media_node.h"
#include "function_node.h"
#include "variable_node.h"
#include "operator_node.h"
#include "print_node.h"
#include "syscall_node.h"
#include "sys_lib.h"
#include "assembly_generator_chip32.h"

StoryProject::StoryProject(ILogger &log)
    : m_log(log)
{

}

StoryProject::~StoryProject()
{
}

void StoryProject::SetPaths(const std::string &uuid, const std::string &library_path)
{
    m_uuid = uuid;
    m_project_file_path = std::filesystem::path(library_path) / uuid / std::filesystem::path("project.json");

    m_working_dir = m_project_file_path.parent_path().generic_string();
    m_assetsPath = m_working_dir /  std::filesystem::path("assets");

    std::cout << "Working dir is: " << m_working_dir << std::endl;
}

void StoryProject::CopyToDevice(const std::string &outputDir, NodesFactory &factory)
{
    ResourceManager manager(m_log);
    
    Load(manager, factory);  

    // Output dir is the root. Build an assets directory to the device location
    std::filesystem::path destRootDir = std::filesystem::path(outputDir) / m_uuid;
    std::filesystem::path destAssetsDir = destRootDir /  "assets";
    std::filesystem::create_directories(destAssetsDir);

    // Generate and copy binary
    std::string code;
    GenerateScript(code);

    std::cout << code << std::endl;

    Chip32::Assembler::Error err;
    if (GenerateBinary(code, err))
    {
        std::filesystem::copy(BinaryFileName(), destRootDir, std::filesystem::copy_options::overwrite_existing);

        // Convert resources (if necessary) and copy them to destination assets
        manager.ConvertResources(AssetsPath(), destAssetsDir, m_storyOptions.image_format, m_storyOptions.sound_format);
    }
}

void StoryProject::New(const std::string &uuid, const std::string &library_path)
{
    SetPaths(uuid, library_path);

    // First try to create the working directory
    if (!std::filesystem::is_directory(m_working_dir))
    {
        std::filesystem::create_directories(m_working_dir);
    }

    std::filesystem::create_directories(m_assetsPath);

    CreatePage(MainUuid());

    m_initialized = true;
}

std::filesystem::path StoryProject::BinaryFileName() const
{
    return m_working_dir / "story.c32";
}


void StoryProject::SaveBinary()
{
    std::ofstream o(BinaryFileName() , std::ios::out | std::ios::binary);
    o.write(reinterpret_cast<const char*>(m_program.data()), m_program.size());
    o.close();
}

bool StoryProject::ParseStoryInformation(nlohmann::json &j)
{
    bool success = false;

    if (j.contains("project"))
    {
        nlohmann::json projectData = j["project"];

        m_name = projectData["name"].get<std::string>();
        m_uuid = projectData["uuid"].get<std::string>();
        m_titleImage = projectData.value("title_image", "");
        m_titleSound = projectData.value("title_sound", "");

        success = true;
    }

    return success;
}

void StoryProject::ModelToJson(nlohmann::json &model)
{
    for (const auto & p : m_pages)
    {
        nlohmann::json page = p->ToJson();
        model.push_back(page);
    }
}

std::shared_ptr<StoryPage> StoryProject::CreatePage(const std::string &uuid)
{
    auto newPage = std::make_shared<StoryPage>(uuid);
    m_pages.push_back(newPage);
    return newPage;
}

void StoryProject::AddConnection(const std::string_view &page_uuid, std::shared_ptr<Connection> c)
{
    for (const auto & p : m_pages)
    {
        if (p->Uuid() == page_uuid)
        {
            p->AddLink(c);
        }
    }
}

void StoryProject::AddNode(const std::string_view &page_uuid, std::shared_ptr<BaseNode> node)
{
    for (const auto & p : m_pages)
    {
        if (p->Uuid() == page_uuid)
        {
            p->AddNode(node);
        }
    }
}

void StoryProject::DeleteNode(const std::string_view &page_uuid, const std::string &id)
{
    for (const auto & p : m_pages)
    {
        if (p->Uuid() == page_uuid)
        {
            p->DeleteNode(id);
        }
    }
}

std::shared_ptr<StoryPage> StoryProject::GetPage(const std::string &uuid)
{
    for (const auto & p : m_pages)
    {
        if (p->Uuid() == uuid)
        {
            return p;
        }
    }

    return nullptr;

}

void StoryProject::DeleteLink(const std::string_view &page_uuid, std::shared_ptr<Connection> c)
{

    for (const auto & p : m_pages)
    {
        if (p->Uuid() == page_uuid)
        {
            p->DeleteLink(c);
        }
    }
}

std::pair<std::list<std::shared_ptr<BaseNode>>::iterator, std::list<std::shared_ptr<BaseNode>>::iterator> StoryProject::Nodes(const std::string_view &page_uuid)
{
    for (const auto & p : m_pages)
    {
        if (p->Uuid() == page_uuid)
        {
            return p->Nodes();
        }
    }

    return std::pair<std::list<std::shared_ptr<BaseNode>>::iterator, std::list<std::shared_ptr<BaseNode>>::iterator>();
}

std::pair<std::list<std::shared_ptr<Connection>>::iterator, std::list<std::shared_ptr<Connection>>::iterator> StoryProject::Links(const std::string_view &page_uuid)
{
    for (const auto & p : m_pages)
    {
        if (p->Uuid() == page_uuid)
        {
            return p->Links();
        }
    }

    return std::pair<std::list<std::shared_ptr<Connection>>::iterator, std::list<std::shared_ptr<Connection>>::iterator>();
}

void StoryProject::ScanVariable(const std::function<void(std::shared_ptr<Variable> element)>& operation)
{
    for (auto &v : m_variables)
    {
        operation(v);
    }
}

void StoryProject::AddVariable() 
{
    auto v = std::make_shared<Variable>("var_" + std::to_string(m_variables.size()));

    v->SetValue(0);
    v->SetValueType(Variable::ValueType::INTEGER);
    v->SetConstant(false);
    
    m_variables.push_back(v);
}

void StoryProject::DeleteVariable(int i)
{
    m_variables.erase(m_variables.begin() + i);
}

bool StoryProject::ModelFromJson(const nlohmann::json &model, NodesFactory &factory)
{
    bool success = false;
    try {

        nlohmann::json pagesJsonArray = model["pages"];
        m_pages.clear();

        for (auto& pageModel : pagesJsonArray)
        {
            // 1. Create the page in memory
            auto p = std::make_shared<StoryPage>(pageModel["uuid"].get<std::string>());
            m_pages.push_back(p);

            // 2. Load the nodes
            nlohmann::json nodesJsonArray = pageModel["nodes"];
            for (auto& element : nodesJsonArray) {
            
                std::string type = element["type"].get<std::string>();

                auto n = factory.CreateNode(type);
                if (n)
                {
                    AddNode(p->Uuid(), n);
                    n->FromJson(element);
                    n->Initialize();
                }
                else
                {
                    throw std::logic_error(std::string("No registered model with name ") + type);
                }
            }

            // 3. Load the connections
            // std::cout << model.dump(4) << std::endl;

            // Ici on reste flexible sur les connexions, cela permet de créer éventuellement des 
            // projets sans fils (bon, l'élément devrait quand même exister dans le JSON)
            if (pageModel.contains("connections"))
            {
                nlohmann::json connectionJsonArray = pageModel["connections"];

                // key: node UUID, value: output counts
                std::map<std::string, int> outputCounts;

                for (auto& connection : connectionJsonArray)
                {
                    p->AddLink(std::make_shared<Connection>(connection.get<Connection>()));
                }
            }


        }

       
        success = true;
    }
    catch(nlohmann::json::exception &e)
    {
        std::cout << "(NodeEditorWindow::Load) " << e.what() << std::endl;
    }

    return success;
}

bool StoryProject::CopyProgramTo(uint8_t *memory, uint32_t size)
{
    bool success = false;
    // Update ROM memory
    if (m_program.size() < size)
    {
        std::copy(m_program.begin(), m_program.end(), memory);
        success = true;
    }
    return success;
}

bool StoryProject::GetAssemblyLine(uint32_t pointer_counter, uint32_t &line)
{
    bool success = false;
    // On recherche quelle est la ligne qui possède une instruction à cette adresse
    std::vector<Chip32::Instr>::const_iterator ptr = m_assembler.Begin();
    for (; ptr != m_assembler.End(); ++ptr)
    {
        if ((ptr->addr == pointer_counter) && ptr->isRomCode())
        {
            break;
        }
    }

    if (ptr != m_assembler.End())
    {
        line = ptr->line;
        success = true; 
    }

    return success;
}

std::list<std::shared_ptr<Connection>> StoryProject::GetNodeConnections(const std::string &nodeId)
{
    std::list<std::shared_ptr<Connection>> c;

    for (const auto &p : m_pages)
    {
        p->GetNodeConnections(c, nodeId);
    }

    return c;
}

int StoryProject::OutputsCount(const std::string &nodeId)
{
    for (const auto &p : m_pages)
    {
        return p->OutputsCount(nodeId);
    }
    return 0;
}

bool StoryProject::UseResource(const std::string &label)
{
    bool used = m_usedLabels.contains(label);

    if (!used)
    {
        m_usedLabels.insert(label);
    }
    return used;
}

bool StoryProject::GenerateScript(std::string &codeStr)
{
    bool retCode = true;
    std::stringstream code;
    
    // Empty resources usage
    m_usedLabels.clear();



     // Create generator context with current time and user
     AssemblyGenerator::GeneratorContext context(
        m_variables,
        "2025-04-08 12:09:01",  // Current UTC time
        "story-editor",              // Current user
        true,                   // Enable debug output
        true,                   // Enable optimizations
        1024                    // Stack size
    );

    // Create generator
    AssemblyGeneratorChip32 generator(context);

    try
    {
        generator.Reset();
            
        // Generate header comments
        generator.GenerateHeader();

        // Generate text section
        generator.StartSection(AssemblyGenerator::Section::TEXT);
        for (const auto & p : m_pages)
        {
            p->BuildNodes(generator);
        }

        // Generate data section
        generator.StartSection(AssemblyGenerator::Section::DATA);
        for (const auto & p : m_pages)
        {
            p->BuildNodesVariables(generator);
        }
        generator.GenerateGlobalVariables();

        generator.GenerateExit();

    }
    catch (const std::exception &e)
    {
        m_log.Log(e.what(), true);
        retCode = false;
    }

    codeStr = generator.GetAssembly();

    return retCode;
}

bool StoryProject::GenerateBinary(const std::string &code, Chip32::Assembler::Error &err)
{
    Chip32::Result result;
    bool success = false;

    if (m_assembler.Parse(code) == true)
    {
        if (m_assembler.BuildBinary(m_program, result) == true)
        {
            result.Print();

            m_log.Log("Binary successfully generated.");
            SaveBinary();
            success = true;
        }
        else
        {
            err = m_assembler.GetLastError();
            
        }
    }
    else
    {
        err = m_assembler.GetLastError();
        m_log.Log(err.ToString(), true);
    }
    return success;
}



bool StoryProject::Load(ResourceManager &manager, NodesFactory &factory)
{
    try {
        std::ifstream f(m_project_file_path);
        nlohmann::json j = nlohmann::json::parse(f);

        manager.Clear();

        if (ParseStoryInformation(j))
        {
            if (j.contains("resources"))
            {
                nlohmann::json resourcesData = j["resources"];

                for (const auto &obj : resourcesData)
                {
                    auto rData = std::make_shared<Resource>();

                    rData->type = obj["type"].get<std::string>();
                    rData->format = obj["format"].get<std::string>();
                    rData->description = obj["description"].get<std::string>();
                    rData->file = obj["file"].get<std::string>();
                    manager.Add(rData);
                }

                if (j.contains("pages"))
                {
                    ModelFromJson(j, factory);
                    m_initialized = true;
                }

                if (j.contains("variables"))
                {
                    nlohmann::json variablesData = j["variables"];

                    for (const auto &obj : variablesData)
                    {
                        auto v = std::make_shared<Variable>(obj["label"].get<std::string>());
                        v->SetUuid(obj["uuid"].get<std::string>());
                        v->SetValueType(Variable::StringToValueType(obj["type"].get<std::string>()));
                        v->SetScalePower(obj["scale"].get<int>());
                        v->SetConstant(obj["constant"].get<bool>());
                        v->SetVariableName(obj["name"].get<std::string>());

                        if (v->IsFloat())
                        {
                            v->SetFloatValue(std::stof(obj["value"].get<std::string>()));
                        }
                        else if (v->IsInteger())
                        {
                            v->SetIntegerValue(std::stoi(obj["value"].get<std::string>()));
                        }
                        else if (v->IsString())
                        {
                            v->SetTextValue(obj["value"].get<std::string>());
                        }
                        else if (v->IsBool())
                        {
                            v->SetBoolValue(obj["value"].get<std::string>() == "true");
                        }

                        m_variables.push_back(v);
                    }
                }
            }
        }
    }
    catch(nlohmann::json::exception &e)
    {
        std::cout << e.what() << std::endl;
    }

    if (m_pages.size() == 0)
    {
        CreatePage(MainUuid());
    }

    return m_initialized;
}

void StoryProject::Save(ResourceManager &manager)
{
    nlohmann::json j;
    j["project"] = { {"name", m_name}, {"uuid", m_uuid}, { "title_image", m_titleImage }, { "title_sound", m_titleSound } };

    {
        nlohmann::json resourcesData;

        auto [b, e] = manager.Items();
        for (auto it = b; it != e; ++it)
        {
            nlohmann::json obj = {{"type", (*it)->type},
                                  {"format", (*it)->format},
                                  {"description", (*it)->description},
                                  {"file", (*it)->file}};

            resourcesData.push_back(obj);
        }
        j["resources"] = resourcesData;
    }

    nlohmann::json model;
    ModelToJson(model);
    j["pages"] = model;

    nlohmann::json variablesData;
    for (const auto &v : m_variables)
    {
        std::string value;

        if (v->IsFloat())
        {
            value = std::to_string(v->GetFloatValue());
        }
        else if (v->IsInteger())
        {
            value = std::to_string(v->GetIntegerValue());
        }
        else if (v->IsString())
        {
            value = v->GetStringValue();
        }
        else if (v->IsBool())
        {
            value = v->GetBoolValue() ? "true" : "false";
        }

        nlohmann::json obj = {{"name", v->GetVariableName()},
                              {"label", v->GetLabel()},
                              {"uuid", v->GetUuid()},
                              {"value", value},
                              {"scale", v->GetScalePower()},
                              {"constant", v->IsConstant()},
                              {"type", Variable::ValueTypeToString(v->GetValueType())}};

        variablesData.push_back(obj);
    }
    j["variables"] = variablesData;

    std::ofstream o(m_project_file_path);
    o << std::setw(4) << j << std::endl;
}


void StoryProject::Clear()
{
    m_uuid = "";
    m_working_dir = "";
    m_project_file_path = "";
    m_initialized = false;
    m_variables.clear();
}


void StoryProject::SetTitleImage(const std::string &titleImage)
{
    m_titleImage = titleImage;
}

void StoryProject::SetTitleSound(const std::string &titleSound)
{
    m_titleSound = titleSound;
}

void StoryProject::SetImageFormat(Resource::ImageFormat format)
{
    m_storyOptions.image_format = format;
}

void StoryProject::SetSoundFormat(Resource::SoundFormat format)
{
    m_storyOptions.sound_format = format;
}

void StoryProject::SetDisplayFormat(int w, int h)
{
    m_storyOptions.display_w = w;
    m_storyOptions.display_h = h;
}

std::string StoryProject::GetProjectFilePath() const
{
    return m_project_file_path.generic_string();
}

std::string StoryProject::GetWorkingDir() const
{
    return m_working_dir.string();
}

std::string StoryProject::BuildFullAssetsPath(const std::string_view fileName) const
{
    return (AssetsPath() / fileName).generic_string();
}




