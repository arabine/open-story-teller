
#include "sys_lib.h"
#include <algorithm>
#include <regex>
#include <filesystem>
#include <fstream>

void SysLib::EraseString(std::string &theString, const std::string &toErase)
{
    std::size_t found;
    found = theString.find(toErase);
    if (found != std::string::npos)
    {
        theString.erase(found, toErase.size());
    }
}

std::string SysLib::ToUpper(const std::string &input)
{
    std::string str = input;
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

std::string SysLib::ToLower(const std::string &input)
{
    std::string str = input;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

std::string  SysLib::ReadFile(const std::string &filename)
{
    // Open the stream to 'lock' the file.
    std::ifstream f(filename, std::ios::in | std::ios::binary);

    // Obtain the size of the file.
    const auto sz = std::filesystem::file_size(filename);

    // Create a buffer.
    std::string result(sz, '\0');

    // Read the whole file into the buffer.
    f.read(result.data(), sz);

    return result;
}

std::string SysLib::GetFileExtension(const std::string &fileName)
{
    auto idx = fileName.find_last_of(".");
    if(idx != std::string::npos)
    {
        return fileName.substr(idx + 1);
    }
    return "";
}

std::string SysLib::GetFileName(const std::string &path)
{
    if (path.size() > 0)
    {
        auto found = path.find_last_of("/\\");

        if (found != std::string::npos)
        {
            return path.substr(found+1);
        }
        else
        {
            return path;
        }
    }
    else
    {
        return "";
    }
}

std::string SysLib::GetDirectory(const std::string &filePath)
{
    try {
        std::filesystem::path absPath = std::filesystem::absolute(filePath);
        std::filesystem::path dirPath = absPath.parent_path(); // Récupère le chemin sans le nom du fichier
        return dirPath.string(); // Convertit le chemin en chaîne de caractères
    } catch (const std::filesystem::filesystem_error& e) {
        // std::cerr << "Erreur: " << e.what() << std::endl;
        return "";
    }
}


void SysLib::ReplaceCharacter(std::string &theString, const std::string &toFind, const std::string &toReplace)
{
    std::size_t found;
    do
    {
        found = theString.find(toFind);
        if (found != std::string::npos)
        {
            theString.replace(found, 1, toReplace);
        }
    }
    while (found != std::string::npos);
}

std::string SysLib::RemoveFileExtension(const std::string &filename)
{
    // Trouver la dernière occurrence du point
    std::size_t dotPos = filename.rfind('.');
    if (dotPos == std::string::npos) {
        // Pas d'extension trouvée, retourner le nom tel quel
        return filename;
    }
    // Retourner la sous-chaîne avant le point
    return filename.substr(0, dotPos);
}

std::string SysLib::Normalize(const std::string &input)
{
    std::string valid_file = input;

    std::replace(valid_file.begin(), valid_file.end(), '\\', '_');
    std::replace(valid_file.begin(), valid_file.end(), '/', '_');
    std::replace(valid_file.begin(), valid_file.end(), ':', '_');
    std::replace(valid_file.begin(), valid_file.end(), '?', '_');
    std::replace(valid_file.begin(), valid_file.end(), '\"', '_');
    std::replace(valid_file.begin(), valid_file.end(), '<', '_');
    std::replace(valid_file.begin(), valid_file.end(), '>', '_');
    std::replace(valid_file.begin(), valid_file.end(), '|', '_');
    std::replace(valid_file.begin(), valid_file.end(), ' ', '_');

    return valid_file;
}
