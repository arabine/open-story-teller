/**
 * MIT License
 * Copyright (c) 2019 Anthony Rabine
 */

#ifndef ZIP_H
#define ZIP_H

#include <cstdint>
#include <string>
#include <vector>


#include "miniz.h"

/*****************************************************************************/
class Zip
{
public:
    Zip();
    ~Zip();

    bool Open(const std::string &zip, bool isFile);
    std::uint32_t NumberOfFiles() { return mNumberOfFiles; }
    bool GetFile(const std::string &fileName, std::string &contents);
    std::vector<std::string> ListFiles();
    void Close();
    bool IsOpen() const { return mIsOpen; }
	bool isOpenForWriting() const	{	return mIsOpenForWriting;	}

    static int CompressBuffer(const char *input, size_t input_size, char *output);

    void CreateInMemory(const std::string &fileName);
    bool AddFile(const std::string &fileName, const std::string &archiveName);
    void AddDirectory(const std::string &dirName);

    static std::vector<std::string> Unzip(const std::string &zipFile, const std::string &destination_dir, const std::string &password);

private:
    mz_zip_archive mZipArchive;
    bool mIsValid;
    bool mIsOpen{false};
    bool mIsOpenForWriting{false};
    std::uint32_t mNumberOfFiles;
    std::vector<std::string> mFiles;
};

#endif // ZIP_H

//=============================================================================
// End of file Zip.cpp
//=============================================================================
