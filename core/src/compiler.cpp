
#include "compiler.h"


std::string Compiler::FileToConstant(const std::string &FileName, const std::string &extension)
{
    std::string f = SysLib::RemoveFileExtension(FileName);
    return "$" + FileName + " DC8 \"" + FileName + "\", 8\r\n";
}
