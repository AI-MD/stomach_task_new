#pragma once

#include "sqllite3/sqlite3.h"
#include "DBProc.h"
#include <queue>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <string>

#if defined ( _WIN32 )
#include <sys/stat.h>
#endif

namespace fs = std::filesystem;
using namespace std::chrono_literals;

class ImageLoad
{
private:
    DBProc dbproc;
    sqlite3* db;
    std::queue<std::string> imageFiles;
    std::string path;
    std::time_t startTime;
    bool isRelatimeFileLoadFlag;
   

public:
    ImageLoad(std::string path, bool createTableFlag, bool isRelatimeFileLoadFlag, std::string db_name) : dbproc(db_name)
    {
        if (createTableFlag)
            dbproc.createtable();
        db = dbproc.dbConnection();
        this-> path = path;
        this -> isRelatimeFileLoadFlag = isRelatimeFileLoadFlag;
        startTime = time(NULL);
    };
   
    void processing();
    std::queue<std::string> getImages();
    void updatecheckflag(std::string dir_name, std::string file_name);
    void close();
    void imageCheck(fs::path dirPath, fs::path filePath);
    std::time_t GetFileWriteTime(const fs::path& filename);
   

};

