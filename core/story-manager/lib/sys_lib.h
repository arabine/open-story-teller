#pragma once

#include <string>
#include <filesystem>

class SysLib
{
public:

    static std::string GetFileExtension(const std::string &FileName);
    static std::string GetFileName(const std::string &path);
    static std::string GetDirectory(const std::string &filePath);
    static std::string RemoveFileExtension(const std::string &FileName);
    static void ReplaceCharacter(std::string &theString, const std::string &toFind, const std::string &toReplace);
    static std::string Normalize(const std::string &input);
    static void EraseString(std::string &theString, const std::string &toErase);
    static std::string ToUpper(const std::string &input);
    static std::string ToLower(const std::string &input);
    static std::string ReadFile(const std::filesystem::path &filename);
};
