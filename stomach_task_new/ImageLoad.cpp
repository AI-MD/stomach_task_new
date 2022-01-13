#include "ImageLoad.h"



std::time_t ImageLoad::GetFileWriteTime(const fs::path& filename)
{
#if defined ( _WIN32 )
     {
         struct _stat64 fileInfo;
         if (_wstati64(filename.wstring().c_str(), &fileInfo) != 0)
         {
             throw std::runtime_error("Failed to get last write time.");
         }
         return fileInfo.st_mtime;
     }
#else
     {
         auto fsTime = std::filesystem::last_write_time(filename);
         return decltype (fsTime)::clock::to_time_t(fsTime);
     }
#endif
}


void ImageLoad::processing()
{
    std::queue<std::string> empty;
    std::swap(imageFiles, empty);
    
    fs::path dirPath;


    if (!fs::exists(path))
    {
        return;
    }
    for (auto& p : fs::recursive_directory_iterator(path))
    {
        if (fs::is_directory(p))
        {
            dirPath = p.path();
        }
        else { 
            if (isRelatimeFileLoadFlag && GetFileWriteTime(p.path()) > startTime)
            {
                //std::cout << "check current" << std::endl;
                imageCheck(dirPath, p.path());
            }
              
                /*
                    실시간 (클라이언트가 start 누른 시점 이후에 저장되는 파일 로딩) 
                */ 
            else 
            {
              
                 imageCheck(dirPath, p.path());
                
                /*
                    쌓여 있는 파일 처리할때.
                    
                    imageCheck(dirPath, p.path());
                */
               
            }
        }
    }
}



void ImageLoad::imageCheck(fs::path dirPath, fs::path filePath)
{
    if (filePath.extension() == ".bmp" || filePath.extension() == ".jpg" ||
        filePath.extension() == ".tif" || filePath.extension() == ".png")
    {
        std::string dir_name = dirPath.filename().string();
        std::string file_name = filePath.filename().string();

        std::string sqlstatement = "select count(*) from stomach_img where filename = '" + file_name + "';";

        if (dbproc.countSelect(sqlstatement) == 0)
        {

            sqlstatement = "INSERT INTO stomach_img Values ('" + dir_name + "','" + file_name + "','" + std::to_string(0) + "');";
            dbproc.insert(sqlstatement);
        }
        else
        {
            sqlstatement = "select flag from stomach_img where filename = '" + file_name + "';";
            int checkFlag = dbproc.checkSelect(sqlstatement);
                
            if (checkFlag == 0)
            {
                imageFiles.push(filePath.string());
            }
        }
    }
}

std::queue<std::string> ImageLoad::getImages()
{
    return imageFiles;
}

void ImageLoad::updatecheckflag(std::string file_name)
{
  std::string sqlstatement = "UPDATE stomach_img set flag = " + std::to_string(1) + " where filename = '" + file_name + "';";
  
  dbproc.update(sqlstatement); // 추론이 끝난 이미지는 db 업데이트 
}

void ImageLoad::close()
{
    dbproc.dbclose();
}