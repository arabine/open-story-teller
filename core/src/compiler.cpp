
#include "compiler.h"


std::string Compiler::FileToConstant(const std::string &FileName, const std::string &extension, IStoryProject &project)
{
    std::string label = "$" + FileName;

    if (!project.UseResource(label))
    {
        std::string f = SysLib::RemoveFileExtension(FileName);
        return label + " DC8 \"" + FileName + "\", 8\r\n";
    }

    // Label of file is already existing, so we do not add anything
    return "";    
}
