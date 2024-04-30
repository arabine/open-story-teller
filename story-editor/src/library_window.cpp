#include "library_window.h"
#include "gui.h"
#include "ImGuiFileDialog.h"
#include <filesystem>
#include "IconsMaterialDesignIcons.h"
#include "i_story_manager.h"
#include <functional>
#include "base64.hpp"

typedef int (*xfer_callback_t)(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                               curl_off_t ultotal, curl_off_t ulnow);


void download_file(CURL *curl,
                   const std::string &url,
                   const std::string &output_file,
                   std::function<void(bool, const std::string &filename)> finished_callback)
{
    FILE *fp;
    CURLcode res = CURLE_FAILED_INIT;

    if (curl)
    {
        //SetConsoleTextAttribute(hConsole, 11);
        fp = fopen(output_file.c_str(),"wb");

        if (fp)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            std::cout<<" Start download"<<std::endl<<std::endl;

            res = curl_easy_perform(curl);

            fclose(fp);
            if(res == CURLE_OK)
            {
                std::cout<< "\r\n The file was download with success\r\n"<< std::endl;
            }
            else
            {
                std::cout<< "\r\n Error \r\n"<< std::endl;
            }
        }

        finished_callback(res == CURLE_OK, output_file);
    }
}


LibraryWindow::LibraryWindow(IStoryManager &project, LibraryManager &library)
    : WindowBase("Library Manager")
    , m_storyManager(project)
    , m_libraryManager(library)
{
    m_downloadThread = std::thread( std::bind(&LibraryWindow::DownloadThread, this) );

    m_storeUrl[0] = 0;

   // std::strcpy(m_storeUrl, "https://gist.githubusercontent.com/DantSu/8920929530b58f2acbfcf1ed0e7746f9/raw/stories-contrib.json");
}

LibraryWindow::~LibraryWindow()
{
    // Cancel any pending download

    if (m_curl)
    {
        m_downloadMutex.lock();
        m_cancel = true;
        m_downloadMutex.unlock();
    }

    // Quit download thread
    m_downloadQueue.push({"quit", ""});
    if (m_downloadThread.joinable())
    {
        m_downloadThread.join();
    }

    curl_global_cleanup();
}

void LibraryWindow::Initialize()
{
    // Try to download the store index file
    m_curl = curl_easy_init();

    if (m_curl)
    {
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0);

        //progress_bar : the fonction for the progress bar
        Callback<int(void *, curl_off_t, curl_off_t, curl_off_t, curl_off_t)>::func = std::bind(
            &LibraryWindow::TransferCallback,
            this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5);
        curl_easy_setopt(m_curl, CURLOPT_XFERINFOFUNCTION, static_cast<xfer_callback_t>(Callback<int(void *, curl_off_t, curl_off_t, curl_off_t, curl_off_t)>::callback));
    }


    std::strcpy(&m_storeUrl[0], m_libraryManager.GetStoreUrl().c_str());
    m_storeIndexFilename = ToLocalStoreFile(m_storeUrl);

    if (std::filesystem::is_regular_file(m_storeIndexFilename))
    {
        // seems legit
        ParseStoreDataCallback(true, m_storeIndexFilename);
    }
    else
    {
        // Not exists, download it
        m_downloadQueue.push({
            "dl",
            m_storeUrl,
            m_storeIndexFilename,
            std::bind(&LibraryWindow::ParseStoreDataCallback, this, std::placeholders::_1, std::placeholders::_2)
        });

    }
}


void LibraryWindow::DownloadThread()
{
    for (;;)
    {
        auto cmd = m_downloadQueue.front();
        m_downloadQueue.pop();

        if (cmd.order == "quit")
        {
            curl_easy_cleanup(m_curl);

            return;
        }
        else if (cmd.order == "dl")
        {
            download_file(m_curl, cmd.url, cmd.filename, cmd.finished_callback);
        }
    }
}

int LibraryWindow::TransferCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                  curl_off_t ultotal, curl_off_t ulnow)
{
  //  std::cout << "DL total: " << dltotal << ", now: " << dlnow << std::endl;   
    m_downloadMutex.lock();
    bool cancel = m_cancel;
    m_downloadMutex.unlock();

    static TransferProgress progressItem;
    static TransferProgress progressItemPrev;

    if (cancel)
    {
        return 1; // Annuler la requête curl
    }

    if ((dlnow > 0) && (dltotal > 0))
    {
        progressItem.Set(dltotal, dlnow);
        if ((progressItem.Precent() - progressItemPrev.Precent()) > 0.01)
        {
            progressItemPrev = progressItem;
            m_transferProgress.push(TransferProgress(progressItem));
        }
    }
    else
    {
        progressItem.Reset();
        progressItemPrev.Reset();
    }

    return 0;
}


bool LibraryWindow::CheckIfSharepoint(const std::string &url, std::string &decoded_url)
{
    bool success = false;
    std::size_t pos = url.find("/shares/u!");
        
    if (pos != std::string::npos)
    {
        std::size_t start = pos + 10; // Début de l'URL
        std::size_t end = url.find("/", start); // Fin de l'URL
            
        if (end != std::string::npos)
        {
            std::string base64 = url.substr(start, end - start);

            // Calculer le reste de la division par
            int remainder = url.length() % 4;
            int signsToAdd = 0;
            // Déterminer le nombre de signes '=' à ajouter en fonction du reste
            if (remainder == 1) {
                signsToAdd = 2; // Ajouter 2 signes '=' pour obtenir un multiple de 4
                base64 += "==";
            } else if (remainder == 2) {
                signsToAdd = 1; // Ajouter 1 signe '=' pour compléter la séquence
                base64 += "=";
            } else {
                signsToAdd = 0; // Aucun signe '=' à ajouter
            }

            std::replace( base64.begin(), base64.end(), '_', '/');
            std::replace( base64.begin(), base64.end(), '-', '+');
            
            try
            {
                decoded_url = base64::from_base64(base64);
                std::cout << "Base64: " << decoded_url << std::endl;
                success = true;
            }
            catch(std::exception &e)
            {
                std::cout << e.what() << std::endl;
            }
            
        }

    }

    return success;
}


void LibraryWindow::SharePointJsonDownloadedCallback(bool success, const std::string &filename)
{
    try {
        std::ifstream f(filename);
        nlohmann::json j = nlohmann::json::parse(f);


        std::string archive = j["@content.downloadUrl"].get<std::string>();
        std::string name = j["name"].get<std::string>();


        m_downloadQueue.push({
            "dl",
            archive,
            ToLocalStoreFile(StoryProject::Normalize(name)),
            std::bind(&LibraryWindow::StoryFileDownloadedCallback, this, std::placeholders::_1, std::placeholders::_2)
        });

    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}

void LibraryWindow::StoryFileDownloadedCallback(bool success, const std::string &filename)
{
    std::cout << "Finished to download: " << filename << std::endl;


    std::string ext = StoryProject::GetFileExtension(filename);

    if (ext == "zip") 
    {
        std::cout << "Extracting zip..." << std::endl;
    }
    else if (ext == "")
    {
        std::ifstream t(filename);
        std::string htmlCode((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());

        t.close();

        // Trouver la position de "url="
        std::size_t pos = htmlCode.find("url=");
        
       if (pos != std::string::npos) {
            // Trouver la position du premier guillemet après "url="
            std::size_t start = pos + 4; // Début de l'URL
            std::size_t end = htmlCode.find("/root/", start); // Fin de l'URL
            
            if (end != std::string::npos)
            {
                // Extraire l'URL
                std::string url = htmlCode.substr(start, end - start);
                std::cout << "URL extraite : " << url << std::endl;

                m_downloadQueue.push({
                    "dl",
                    url + "/driveItem",
                    ToLocalStoreFile(filename + ".json"),
                    std::bind(&LibraryWindow::SharePointJsonDownloadedCallback, this, std::placeholders::_1, std::placeholders::_2)
                });

            } else {
                std::cout << "Le guillemet de fin n'a pas été trouvé." << std::endl;
            }
        } else {
            std::cout << "La chaîne 'url=' n'a pas été trouvée." << std::endl;
        }
    }
    else {
        std::cout << "Unsupported extension: " << ext << std::endl;
    }

}

void LibraryWindow::ParseStoreDataCallback(bool success, const std::string &filename)
{
    try {
        std::ifstream f(m_storeIndexFilename);
        nlohmann::json j = nlohmann::json::parse(f);

        m_store.clear();
        for (const auto &obj : j)
        {
            StoryInf s;

            s.title = obj["title"].get<std::string>();
            s.description = obj["description"].get<std::string>();
            s.download = obj["download"].get<std::string>();
            s.age = obj["age"].get<int>();

            m_store.push_back(s);
        }
    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}

static bool canValidateDialog = true;
static int formatFilter = 0;

inline void InfosPane(const char *vFilter, IGFDUserDatas vUserDatas, bool *vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");
    ImGui::Text("Selected Filter : %s", vFilter);

    
    ImGui::Text("Select file format: ");
    ImGui::RadioButton("Commercial stories", &formatFilter, 0); ImGui::SameLine();
    ImGui::RadioButton("Studio format", &formatFilter, 1); ImGui::SameLine();

  //  ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);
    if (vCantContinue)
        *vCantContinue = canValidateDialog;
}

std::string LibraryWindow::ToLocalStoreFile(const std::string &url)
{
    auto filename = StoryProject::GetFileName(url);

    filename = m_libraryManager.LibraryPath() + "/store/" + filename;
    std::cout << "Store file: " << filename << std::endl;
    return filename;
}

void LibraryWindow::Draw()
{
    static int importFormat = 0;

    WindowBase::BeginDraw();
    ImGui::SetWindowSize(ImVec2(626, 744), ImGuiCond_FirstUseEver);

    if (ImGui::Button(ICON_MDI_FOLDER " Export to device"))
    {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;

        ImGuiFileDialog::Instance()->OpenDialog("ChooseOutputDirDialog", "Choose an output directory", nullptr, config);
    }

    if (ImGui::Button( ICON_MDI_FOLDER " Select directory"))
    {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;

        ImGuiFileDialog::Instance()->OpenDialog("ChooseLibraryDirDialog", "Choose a library directory", nullptr, config);
    }

    if (!m_libraryManager.IsInitialized())
    {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
        ImGui::Text("No any library directory set. Please select one where stories will be located.");
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::SameLine();
        ImGui::Text("Library path: %s", m_libraryManager.LibraryPath().c_str());


        static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
        if (ImGui::BeginTabBar("LibraryTabBar", tab_bar_flags))
        {
             static ImGuiTableFlags tableFlags =
                 ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
                 | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
                 | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
                 | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
                 | ImGuiTableFlags_SizingFixedFit;

            // ============================================================================
            // LOCAL TABLE
            // ============================================================================
            if (ImGui::BeginTabItem("Local library ##LocalTabBar", nullptr, ImGuiTabItemFlags_None))
            {
                if (ImGui::Button("Scan library"))
                {
                    m_libraryManager.Scan();
                }

                ImGui::SameLine();

                if (ImGui::Button("Import story"))
                {
                    IGFD::FileDialogConfig config;
                    config.path = m_libraryManager.LibraryPath();
                    config.countSelectionMax = 1;
                    config.sidePane = std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
                    config.sidePaneWidth = 350.0f;
                    config.flags = ImGuiFileDialogFlags_Modal;
                    ImGuiFileDialog::Instance()->OpenDialog("ImportStoryDlgKey", 
                        "Import story", 
                        ".zip, .json",
                        config
                    );
                }

                if (ImGui::BeginTable("library_table", 3, tableFlags))
                {
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Select", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed);

                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TableHeader(ImGui::TableGetColumnName(0));

                    static bool select_all = false;
                    
                    ImGui::TableSetColumnIndex(1);
                    const char* column_name = ImGui::TableGetColumnName(1); // Retrieve name passed to TableSetupColumn()
             
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                    if (ImGui::Checkbox("##checkall", &select_all))
                    {
                        // Mettre à jour tous les checkboxes des lignes en fonction de l'état du checkbox de l'en-tête
                        for (auto &p : m_libraryManager)
                        {
                            p->Select(select_all);
                        }
                    }
                    ImGui::PopStyleVar();
                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                    ImGui::TableHeader(column_name);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TableHeader(ImGui::TableGetColumnName(2));

                    int internal_id = 1;
                    for (auto &p : m_libraryManager)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", p->GetName().c_str());

                        ImGui::PushID(internal_id++);
                        
                        // Select
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("", p->Selected());

                        if (!p->IsSelected() && select_all)
                        {
                            select_all = false; // Désélectionner "Select All" si un des items est désélectionné
                        }

                        ImGui::TableNextColumn();
                        if (ImGui::SmallButton("Load"))
                        {
                            m_storyManager.OpenProject(p->GetUuid());
                        }

                        ImGui::SameLine();
                        if (ImGui::SmallButton("Remove"))
                        {

                        }
                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
 
            // ============================================================================
            // STORE STORY LIST
            // ============================================================================
            if (ImGui::BeginTabItem("Remote Store##StoreTabBar", nullptr, ImGuiTabItemFlags_None))
            {
                ImGui::InputTextWithHint("##store_url", "Store URL", m_storeUrl, IM_ARRAYSIZE(m_storeUrl));
                ImGui::SameLine();
                if (ImGui::Button("Load"))
                {
                    m_storeIndexFilename = ToLocalStoreFile(m_storeUrl);
                    m_downloadQueue.push({
                        "dl",
                        m_storeUrl,
                        m_storeIndexFilename,
                        std::bind(&LibraryWindow::ParseStoreDataCallback, this, std::placeholders::_1, std::placeholders::_2)
                    });
                }

                static TransferProgress progressItem;
                static std::string currentDownload;
                static float progress = 0.0f;
             
                if (m_transferProgress.try_pop(progressItem))
                {
                    progress = progressItem.Precent();

                    if (progress >= .98)
                    {
                        m_downloadBusy = false;
                        progress = 1.0;
                    }
                }
                // Typically we would use ImVec2(-1.0f,0.0f) or ImVec2(-FLT_MIN,0.0f) to use all available width,
                // or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
                ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
                // ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

                // ImGui::Text("Current download: ");

                if (ImGui::BeginTable("store_table", 4, tableFlags))
                {
                    ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Age", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Download", ImGuiTableColumnFlags_WidthFixed);

                    ImGui::TableHeadersRow();

                    int id = 1;
                    for (const auto &obj : m_store)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", obj.title.c_str());

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", obj.description.c_str());

                        ImGui::TableNextColumn();
                        ImGui::Text("%d", obj.age);

                        ImGui::TableNextColumn();
                        ImGui::PushID(id++);
                        if (ImGui::SmallButton("Download"))
                        {
                            if (!m_downloadBusy)
                            {
                                m_downloadBusy = true;

                                m_downloadQueue.push({
                                    "dl",
                                    obj.download,
                                    ToLocalStoreFile(StoryProject::Normalize(obj.title)),
                                    std::bind(&LibraryWindow::StoryFileDownloadedCallback, this, std::placeholders::_1, std::placeholders::_2)
                                });
                            }
                        }
                        ImGui::PopID();
                    }

                    ImGui::EndTable();
                }



                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }


    // ---------------- ADD EXISTING STORY TELLER STORY
    if (ImGuiFileDialog::Instance()->Display("ImportStoryDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            std::string filter = ImGuiFileDialog::Instance()->GetCurrentFilter();
            
            m_storyManager.ImportProject(filePathName, importFormat);

            // action
        }
        // close
        ImGuiFileDialog::Instance()->Close();
    }


    // ---------------- CHOOSE LIBRARY DIR
    static ImGuiWindowFlags window_flags = 0;
    if (ImGuiFileDialog::Instance()->Display("ChooseLibraryDirDialog", window_flags, ImVec2(300, 200)))
    {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string projdir = ImGuiFileDialog::Instance()->GetCurrentPath();

            if (std::filesystem::is_directory(projdir))
            {
                m_libraryManager.Initialize(projdir);
            }
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }

    // ---------------- EXPORT SELECTED STORIES TO SPECIFIED DIRECTORY
    static ImGuiWindowFlags choose_output_window_flags = 0;
    if (ImGuiFileDialog::Instance()->Display("ChooseOutputDirDialog", choose_output_window_flags, ImVec2(300, 200)))
    {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string outputDir = ImGuiFileDialog::Instance()->GetCurrentPath();

            if (std::filesystem::is_directory(outputDir))
            {
                // Generate TLV file (index of all stories)
                m_libraryManager.Save();

                // Copy all files to device
                m_libraryManager.CopyToDevice(outputDir);
            }
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }


    WindowBase::EndDraw();
}

