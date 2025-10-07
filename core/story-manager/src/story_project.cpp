

#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>

#include "story_project.h"
#include "json.hpp"
// #include "media_node.h"
#include "variable_node.h"
#include "operator_node.h"
#include "print_node.h"
#include "syscall_node.h"
#include "sys_lib.h"
#include "assembly_generator_chip32_tac.h"
#include "nodes_factory.h"

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
    // GenerateScript(code); // FIXME 

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
        m_description = projectData["description"].get<std::string>();
        std::string typeString = projectData["type"].get<std::string>();

        if (typeString == "story")
        {
            m_projectType = IStoryProject::PROJECT_TYPE_STORY;
        }
        else if (typeString == "module")
        {
            m_projectType = IStoryProject::PROJECT_TYPE_MODULE;
        }
        else
        {
            m_log.Log("Unknown project type: " + typeString, true);
            return false;
        }

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

std::shared_ptr<StoryPage> StoryProject::CreatePage(const std::string_view uuid)
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

std::shared_ptr<StoryPage> StoryProject::GetPage(const std::string_view &uuid)
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
            p->SetName(pageModel.value("name", "Unnamed Page"));

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

std::vector<IStoryProject::FunctionInfo> StoryProject::GetFunctionsList() const
{
    std::vector<IStoryProject::FunctionInfo> functions;
    
    // Parcourir toutes les pages du projet
    for (const auto& page : m_pages)
    {
        // Exclure la page main (MainUuid)
        if (page->Uuid() == MainUuid())
        {
            continue;
        }
        
        // Ajouter la page à la liste des fonctions disponibles
        IStoryProject::FunctionInfo info;
        info.uuid = page->Uuid();
        info.name = page->GetName();
        
        functions.push_back(info);
    }
    
    return functions;
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


// story_project.cpp

bool StoryProject::GenerateCompleteProgram(std::string &assembly) 
{
    // === PHASE 1 : COLLECTE ===
    // Collecter tous les nœuds et connexions de toutes les pages
    std::vector<std::shared_ptr<BaseNode>> allNodes;
    std::map<std::string, std::pair<std::vector<std::shared_ptr<BaseNode>>,
        std::vector<std::shared_ptr<Connection>>
    >> pageData;
    
    for (const auto& page : m_pages) {
        auto [nodesBegin, nodesEnd] = page->Nodes();
        auto [linksBegin, linksEnd] = page->Links();
        
        std::vector<std::shared_ptr<BaseNode>> pageNodes(nodesBegin, nodesEnd);
        std::vector<std::shared_ptr<Connection>> pageLinks(linksBegin, linksEnd);
        
        // Ajouter tous les nœuds à la liste globale pour la section DATA
        allNodes.insert(allNodes.end(), pageNodes.begin(), pageNodes.end());
        
        // Stocker les données de chaque page
        pageData[std::string(page->Uuid())] = {pageNodes, pageLinks};
    }

    std::cout << "\n=== Resolving VariableNode references ===\n";
    for (const auto& baseNode : allNodes) {
        auto varNode = std::dynamic_pointer_cast<VariableNode>(baseNode);
        if (varNode) {
            varNode->ResolveVariable(m_variables);
        }
    }
    
    // PHASE 2 : GÉNÉRATION DE TOUS LES TAC (avant la section DATA!)
    std::cout << "\n=== Generating all TAC programs ===\n";
    std::map<std::string, TACProgram> pageTACPrograms;
    
    for (const auto& page : m_pages) {
        std::string pageUuid(page->Uuid());
        auto& [nodes, connections] = pageData[pageUuid];
        
        // Construire l'AST pour cette page
        ASTBuilder builder(nodes, connections);
        auto astNodes = builder.BuildAST();
        
        // Générer le TAC pour cette page
        TACGenerator tacGen;
        TACProgram pageTAC = tacGen.Generate(astNodes);
        
        // Stocker le TAC
        pageTACPrograms[pageUuid] = pageTAC;
        
        std::cout << "Generated TAC for page: " << page->GetName() << std::endl;
    }
    std::cout << "=== All TAC programs generated ===\n\n";
    
    // === PHASE 3 : GÉNÉRATION DE L'ASSEMBLEUR ===
    AssemblyGenerator::GeneratorContext context(
        m_variables,
        "2025-01-10 15:30:00",
        "story-project",
        true,   // debug
        true,   // optimize
        1024
    );
    
    AssemblyGeneratorChip32TAC generator(context);
    
    // Header
    generator.Reset();
    generator.GenerateHeader();
    
    // === SECTION DATA (maintenant les format strings sont corrects!) ===
    generator.StartSection(AssemblyGenerator::Section::DATA);
    
    // Variables globales (partagées entre toutes les pages)
    generator.GenerateGlobalVariables();
    
    // Constantes de tous les nœuds de toutes les pages
    // Les format strings ont déjà été modifiés par le TAC generator
    generator.GenerateNodesVariables(allNodes);
    
    // === SECTION TEXT (chaque page = une fonction) ===
    generator.AddComment("=======================  CODE  =======================");
    
    // Générer chaque page comme une fonction
    bool isFirstPage = true;
    for (const auto& page : m_pages) {
        std::string pageUuid(page->Uuid());
        
        // Récupérer le TAC pré-généré
        TACProgram& pageTAC = pageTACPrograms[pageUuid];
        
        // Générer le label de fonction
        std::string functionLabel;
        if (isFirstPage || pageUuid == MainUuid()) {
            functionLabel = ".main";
            isFirstPage = false;
        } else {
            functionLabel = ".page_" + pageUuid;
        }
        
        generator.AddComment("========================================");
        generator.AddComment("Page: " + std::string(page->GetName()));
        generator.AddComment("UUID: " + pageUuid);
        generator.AddComment("========================================");
        generator.GetAssembly() << functionLabel << ":\n";
        
        if (context.debugOutput) {
            std::cout << "\n=== TAC for page: " << page->GetName() << " ===\n";
            std::cout << pageTAC.ToString() << std::endl;
        }
        
        // Convertir le TAC en assembleur
        generator.GenerateTACToAssembly(pageTAC);
        
        // Retour de la fonction (sauf pour main)
        if (functionLabel != ".main") {
            generator.GetAssembly() << "    ret\n\n";
        }
    }
    
    // Exit du programme
    generator.GenerateExit();
    
    assembly = generator.GetAssembly().str();

    return true;
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
    try
    {
        nlohmann::json j;
        j["project"] = { 
            {"name", m_name}, 
            {"uuid", m_uuid}, 
            { "title_image", m_titleImage }, 
            { "title_sound", m_titleSound }, 
            {"description", m_description}, 
            {"type", (m_projectType == IStoryProject::PROJECT_TYPE_STORY) ? "story" : "module"},
            {"version", m_version}
        };

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
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}


void StoryProject::Clear()
{
    m_variables.clear();
    m_pages.clear();
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




