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



class caseRecording
{
private:
    DBProc dbproc;
    sqlite3* db;

    caseRecording(std::string path, bool createTableFlag, bool isRelatimeFileLoadFlag, std::string db_name) : dbproc(db_name)
    {
        db = dbproc.dbConnection();
        dbproc.createtable();
       
        this->path = path;
        this->isRelatimeFileLoadFlag = isRelatimeFileLoadFlag;
        startTime = time(NULL);
    };

};

