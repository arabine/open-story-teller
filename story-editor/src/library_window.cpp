#include "library_window.h"
#include "gui.h"
#include "ImGuiFileDialog.h"
#include <filesystem>
#include "IconsMaterialDesignIcons.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#define CA_CERT_FILE "./ca-bundle.crt"

#include <curl/curl.h>

int xfer_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                  curl_off_t ultotal, curl_off_t ulnow)
{
    return 0;
}

void download_file(const std::string &output_file = "pi.txt")
{

    CURL *curl_download;
    FILE *fp;
    CURLcode res;
    std::string url = "http://www.gecif.net/articles/mathematiques/pi/pi_1_million.txt";

    curl_download = curl_easy_init();

    if (curl_download)
    {
        //SetConsoleTextAttribute(hConsole, 11);
        fp = fopen(output_file.c_str(),"wb");

        curl_easy_setopt(curl_download, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_download, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl_download, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl_download, CURLOPT_NOPROGRESS, 0);
        //progress_bar : the fonction for the progress bar
        curl_easy_setopt(curl_download, CURLOPT_XFERINFOFUNCTION, xfer_callback);


        std::cout<<" Start download"<<std::endl<<std::endl;

        res = curl_easy_perform(curl_download);

        fclose(fp);
        if(res == CURLE_OK)
        {

            std::cout<< std::endl<< std::endl<<" The file was download with succes"<< std::endl;
        }
        else
        {

            std::cout<< std::endl << std::endl<<" Error"<< std::endl;
        }
        curl_easy_cleanup(curl_download);
    }


}


LibraryWindow::LibraryWindow(IStoryManager &project, LibraryManager &library)
    : WindowBase("Library Manager")
    , m_storyManager(project)
    , m_libraryManager(library)
{

    download_file();

    httplib::SSLClient cli("gist.githubusercontent.com", 443);
    cli.set_ca_cert_path(CA_CERT_FILE);
    cli.enable_server_certificate_verification(false);

    if (auto res = cli.Get("/UnofficialStories/32702fb104aebfe650d4ef8d440092c1/raw/luniicreations.json")) {
        std::cout << res->status << std::endl;
        std::cout << res->get_header_value("Content-Type") << std::endl;
        m_storeRawJson = res->body;
        ParseStoreData();
        //std::cout << res->body << std::endl;
    } else {
        std::cout << "error code: " << res.error() << std::endl;

        auto result = cli.get_openssl_verify_result();
        if (result) {
            std::cout << "verify error: " << X509_verify_cert_error_string(result) <<std:: endl;
        }

    }
}


void LibraryWindow::ParseStoreData()
{
    try {

        nlohmann::json j = nlohmann::json::parse(m_storeRawJson);

        m_store.clear();
        for (const auto &obj : j)
        {
            StoryInf s;

            s.title = obj["title"].get<std::string>();
            s.age = obj["age"].get<int>();

            m_store.push_back(s);
        }
    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}

void LibraryWindow::Initialize()
{



}

static bool canValidateDialog = false;
inline void InfosPane(const char *vFilter, IGFDUserDatas vUserDatas, bool *vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");
    ImGui::Text("Selected Filter : %s", vFilter);
    ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);
    if (vCantContinue)
        *vCantContinue = canValidateDialog;
}


void LibraryWindow::Draw()
{
//    if (!IsVisible())
//    {
//        return;
//    }


    WindowBase::BeginDraw();
    ImGui::SetWindowSize(ImVec2(626, 744), ImGuiCond_FirstUseEver);



    if (ImGui::Button( ICON_MDI_FOLDER " Select directory"))
    {
        ImGuiFileDialog::Instance()->OpenDialog("ChooseLibraryDirDialog", "Choose a library directory", nullptr, ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
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
                    ImGuiFileDialog::Instance()->OpenDialogWithPane("ImportStoryDlgKey", "Import story", "", "", InfosPane);
                }


                if (ImGui::BeginTable("library_table", 2, tableFlags))
                {
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);

                    ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed);

                    ImGui::TableHeadersRow();

                    for (auto &p : m_libraryManager)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", p->GetName().c_str());

                        ImGui::TableNextColumn();

                        if (ImGui::SmallButton("Load"))
                        {
                            m_storyManager.OpenProject(p->GetUuid());
                        }

                        ImGui::SameLine();

                        if (ImGui::SmallButton("Remove"))
                        {

                        }
                    }

                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }

            // ============================================================================
            // LOCAL TABLE
            // ============================================================================
            if (ImGui::BeginTabItem("Remote Store##StoreTabBar", nullptr, ImGuiTabItemFlags_None))
            {
                if (ImGui::BeginTable("store_table", 3, tableFlags))
                {
                    ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Age", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Download", ImGuiTableColumnFlags_WidthFixed);

                    ImGui::TableHeadersRow();

                    for (const auto &obj : m_store)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", obj.title.c_str());

                        ImGui::TableNextColumn();
                        ImGui::Text("%d", obj.age);

                        ImGui::TableNextColumn();
                        if (ImGui::SmallButton("Download"))
                        {

                        }
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
            // here convert from string because a string was passed as a userDatas, but it can be what you want
            // std::string userDatas;
            // if (ImGuiFileDialog::Instance()->GetUserDatas())
            //     userDatas = std::string((const char*)ImGuiFileDialog::Instance()->GetUserDatas());
            // auto selection = ImGuiFileDialog::Instance()->GetSelection(); // multiselection

            // action
        }
        // close
        ImGuiFileDialog::Instance()->Close();
    }


    // ---------------- CHOOSE LIBRARY DIR
    if (ImGuiFileDialog::Instance()->Display("ChooseLibraryDirDialog"))
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


    WindowBase::EndDraw();
}

