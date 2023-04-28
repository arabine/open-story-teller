/**
 * MIT License
 * Copyright (c) 2019 Anthony Rabine
 */


#include "zip.h"
#include <fstream>
#include <cstring>
#include <functional>
#include <memory>
#include <filesystem>

static bool ensure_file_exists_and_is_readable(const char *pFilename)
{
  FILE *p = nullptr;
#ifdef Q_OS_WIN
  fopen_s(& p, pFilename, "rb");
#else
  p = fopen(pFilename, "rb");
#endif
  if (!p) {
	  return false;
  }

    fseek(p, 0, SEEK_END);
    long int src_file_size = ftell(p);
    fseek(p, 0, SEEK_SET);

  if (src_file_size)
  {
    char buf[1];
    if (fread(buf, 1, 1, p) != 1)
    {
      fclose(p);
      return false;
    }
  }
  fclose(p);
  return true;
}

/*****************************************************************************/
Zip::Zip()
    : mIsValid(false)
    , mNumberOfFiles(0U)
{
    std::memset(&mZipArchive, 0, sizeof(mZipArchive));
}
/*****************************************************************************/
Zip::~Zip()
{
    Close();
}
/*****************************************************************************/
void Zip::CreateInMemory(const std::string &fileName)
{
    mz_bool status = MZ_FALSE;
    std::memset(&mZipArchive, 0, sizeof(mZipArchive));

    status = mz_zip_writer_init_file(&mZipArchive, fileName.c_str(), 65537);

    if (status)
    {
        mIsOpenForWriting = true;
        mIsValid = true;
        mIsOpen = true;
    }
    else
    {
        mIsOpenForWriting = false;
        mIsValid = false;
    }
}
/*****************************************************************************/
bool Zip::AddFile(const std::string &fileName, const std::string &archiveName)
{
    mz_bool status = MZ_FALSE;
    if (ensure_file_exists_and_is_readable(fileName.c_str()))
    {
        status = mz_zip_writer_add_file(&mZipArchive, archiveName.c_str(), fileName.c_str(), NULL, 0, MZ_NO_COMPRESSION);
    }

    return status == MZ_TRUE;
}
/*****************************************************************************/
void Zip::AddDirectory(const std::string &dirName)
{
     std::string d = dirName + "/";
     mz_zip_writer_add_mem(&mZipArchive, d.c_str(), NULL, 0, MZ_NO_COMPRESSION);
}
/*****************************************************************************/
std::vector<std::string> Zip::Unzip(std::string const &zipFile, const std::string &destination_dir, std::string const &password)
{
    (void)(password);
    std::vector<std::string> files = {};
    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));

    auto status = mz_zip_reader_init_file(&zip_archive, zipFile.c_str(), 0);
    if (!status) return files;
    int fileCount = (int)mz_zip_reader_get_num_files(&zip_archive);
    if (fileCount == 0)
    {
        mz_zip_reader_end(&zip_archive);
        return files;
    }
    mz_zip_archive_file_stat file_stat;
    if (!mz_zip_reader_file_stat(&zip_archive, 0, &file_stat))
    {
        mz_zip_reader_end(&zip_archive);
        return files;
    }
    // Get root folder
//	QFileInfo fileInfo(file_stat.m_filename);
//	QFileInfo filePath(fileInfo.path());
//    QString baseName = filePath.path();
//	QString dstDir = QString::fromStdString(destination_dir);
//	QString separator = "/";
    std::string base = std::filesystem::path(file_stat.m_filename).parent_path().string();

    // Get and print information about each file in the archive.
    for (int i = 0; i < fileCount; i++)
    {
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) continue;
        if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) continue; // skip directories for now
        std::string fileName = base + std::filesystem::path::preferred_separator + file_stat.m_filename; // make path relative
        std::string destFile = destination_dir + std::filesystem::path::preferred_separator + fileName; // make full dest path

        // creates the directory where the file will be decompressed
        std::filesystem::create_directories(std::filesystem::path(destFile).parent_path());

        // Extract file
        if (mz_zip_reader_extract_to_file(&zip_archive, i, destFile.c_str(), 0))
        {
            files.emplace_back(destFile);
        }
    }

    // Close the archive, freeing any resources it was using
    mz_zip_reader_end(&zip_archive);
    return files;
}
/*****************************************************************************/
bool Zip::Open(const std::string &zip, bool isFile)
{
    mz_bool status;
    mIsValid = false;
    mIsOpenForWriting = true;

    mNumberOfFiles = 0U;

    std::memset(&mZipArchive, 0, sizeof(mZipArchive));

    if (isFile)
    {
        // Physical file on disk
        status = mz_zip_reader_init_file(&mZipArchive, zip.c_str(), 0);
    }
    else
    {
        // Zipped memory
        status = mz_zip_reader_init_mem(&mZipArchive, zip.c_str(), zip.size(), 0);
    }

    if (status)
    {
        mFiles.clear();
        // Get and print information about each file in the archive.
        for (std::uint32_t i = 0; i < mz_zip_reader_get_num_files(&mZipArchive); i++)
        {
            mz_zip_archive_file_stat file_stat;
            if (mz_zip_reader_file_stat(&mZipArchive, i, &file_stat))
            {
                mNumberOfFiles++;
                mFiles.push_back(file_stat.m_filename);
                //printf("Filename: \"%s\", Comment: \"%s\", Uncompressed size: %u, Compressed size: %u\n", file_stat.m_filename, file_stat.m_comment, (std::uint32_t)file_stat.m_uncomp_size, (std::uint32_t)file_stat.m_comp_size);
            }
        }
    }

    if (mNumberOfFiles == mz_zip_reader_get_num_files(&mZipArchive))
    {
        mIsValid = true;
        mIsOpen = true;
    }

    return mIsValid;
}
/*****************************************************************************/
void Zip::Close()
{
    if (mIsValid)
    {
        if (mIsOpenForWriting)
        {
            if (!mz_zip_writer_finalize_archive(&mZipArchive))
            {
                mz_zip_writer_end(&mZipArchive);
            }
        }
        else
        {
            mz_zip_reader_end(&mZipArchive);
        }
    }
    mIsOpen = false;
}
/*****************************************************************************/
struct UserData
{
    char *output;
    int offset;
};

static mz_bool DeflateCallback(const void *pBuf, int len, void *pUser)
{
    UserData *ud = static_cast<UserData*>(pUser);
    std::memcpy(ud->output + ud->offset, pBuf, len);
    ud->offset += len;
    (void) len;
    (void) pUser;
    return MZ_TRUE;
}

/*****************************************************************************/
int Zip::CompressBuffer(const char *input, size_t input_size, char *output)
{
    int finalsize = -1;
    tdefl_compressor Comp;

    UserData ud;

    ud.offset = 0U;
    ud.output = output;

    if (tdefl_init(&Comp, DeflateCallback, &ud, 0) == TDEFL_STATUS_OKAY)
    {
        if(tdefl_compress_buffer(&Comp, input, input_size, TDEFL_FINISH) == TDEFL_STATUS_DONE)
        {
            finalsize = ud.offset;
        }
    }

    return finalsize;
}
/*****************************************************************************/
bool Zip::GetFile(const std::string &fileName, std::string &contents)
{
    bool ret = false;
    if (mIsValid)
    {
        size_t size;
        char *p = reinterpret_cast<char *>(mz_zip_reader_extract_file_to_heap(&mZipArchive, fileName.c_str(), &size, 0));

        if (p != nullptr)
        {
            contents.assign(p, size);
            free(p);
            ret = true;
        }
    }
    return ret;
}
/*****************************************************************************/
std::vector<std::string> Zip::ListFiles()
{
    return mFiles;
}

//=============================================================================
// End of file Zip.cpp
//=============================================================================
