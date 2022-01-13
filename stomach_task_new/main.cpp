#pragma once
#include <WinSock2.h>
#pragma comment( lib, "ws2_32.lib")

#include <stdio.h>

#include "opencv2/opencv.hpp"

#include <iostream> //I/O스트림 헤더
#include <io.h> //파일 구조체 헤더
#include <string>//스트링 객체 사용 헤더
#include <list>//리스트 자료형 헤더


#include "Infer.h"
#include "ImagePreprocessing.h"
#include  <time.h>

#include <fstream>
#include <iostream>

#include"Server_Socket.h"
#include <thread>
#include "ImageLoad.h"
#include "util.h"
#include <Windows.h>
#include <ctime>

using namespace std;
cv::Size _imgSize(224, 224);//딥러닝 모델에 들어가는 입력 이미지의 해상도 


static wstring  _model = L"densenet_0624_10_224.onnx";// onnx 파일
std::vector<const char*> _className = { "C", "D1","D2", "E", "S1", "S2", "S3", "S4", "S5", "S6" };// label 종류 
std::vector<float> _cutOff = { 0.98, 0.98, 0.97, 0.98, 0.98, 0.85 }; // 클래스 분류 임계값  S1~S6
string path = "D:/PACS_raw_data";// 딥러닝 학습에 들어가지 않은 내시경 제품의 이미지.
//string path = "Z:/";
int port = 9999;
string ip = "127.0.0.1";
string db_name = "db_0";

 bool isCreateTableFlag = true; // 파일 중복을 검증하기 위한 db table 생성 플래그(최초 1회)
 bool isRelatimeFileLoadFlag = true;//실시간으로 파일을 읽어오도록 하는 flag 변수 

/*
    thread 관련 flag 변수
*/

static bool _isTaskingFlag = true;
static bool _isPauseFlag = false;
static bool _isTaskCompleteFlag = false;
static bool _isReceiveFlag = true;
static bool _isJoingFlag = true;
static bool _isHeartBeatFlag = true;


/*
    절차지향.
*/

std::pair<std::string, string> inference(cv::Mat& img);
void saveImage(string packet);
void receiveSocketThread(Server_Socket::tcp& server, SOCKET& client);
void taskWorkingThread(Server_Socket::tcp& server, SOCKET& client, ImageLoad imageload, ImagePreprocessing imgProc);
void ServerCheckPartnerDeathThread(SOCKET& client);
void stopChangeFlag();

int main(int argc, char* argv[] ) {
    
    bool isCreateTableFlag = true; // 파일 중복을 검증하기 위한 db table 생성 플래그(최초 1회)
    bool isRelatimeFileLoadFlag = true;//실시간으로 파일을 읽어오도록 하는 flag 변수 

    if (argc > 1)
    { 
        ip = argv[1];
        port = stoi(argv[2]);
        path = argv[3];
        db_name = argv[4];
        isRelatimeFileLoadFlag = stoi(argv[5]);
    }

    std::cout << path << std::endl;

    try {

        Server_Socket::tcp serverSock(ip, port);
        //Server_Socket::tcp serverSock("172.23.9.23", 9993);
        //Server_Socket::tcp serverSock("172.23.9.23", 9999);

        serverSock.tcp_listen(1);

        thread* taskThread = nullptr;
        thread* receiveThread = nullptr;
        thread* heartbeatThread = nullptr;
 
        /*
           client 연결
        */

        sockaddr_in from;
        SOCKET clientSock;

        /*
           socket 1개만 돌도록 예외처리 추가
        */
        while (1)
        {
            std::cout << "The server is listening\n";
            int result = serverSock.accept_client(clientSock, from);

            /*
              시간을 확인.
            */

            if (argc > 1 && isRelatimeFileLoadFlag == 0)
            {
               
                path;  
            }
            else if(argc >1)
            {
                path = argv[3] + currentDateTime();
               
            }
            else
            {

                path;
            }
           
            /*
                path 예외처리.
            */
            try {
                if (!fs::exists(path))
                {
                    serverSock.send_message(clientSock, "no folder");
                    continue;
                }
            }
            catch (std::exception& e)
            {
                serverSock.send_message (clientSock, "no share folder");
                continue;
            }

            // false일 때는 기존에 있던 파일 읽어오도록 구현 
            ImageLoad imageload = ImageLoad(path, isCreateTableFlag, isRelatimeFileLoadFlag, db_name);
            isCreateTableFlag = false;

            ImagePreprocessing imgPrc = ImagePreprocessing();

            /*
               클라이언트 서버 접속
             */

            std::cout << "Accepted connection\n";

            /*
                1:1 서버 클라 통신 상황이여서 recevie 스레드 task 스레드 heartbeat 스레드가 프로그램에서 한번만 실행되도록 구현
            */
            if (receiveThread == nullptr)
            {
                /*
                  recevie 스레드  while 종료조건  flag 변수
                */
                _isReceiveFlag = true;
                receiveThread = new thread(receiveSocketThread, std::ref(serverSock), std::ref(clientSock));
            }
            if (taskThread == nullptr)
            {
                _isPauseFlag = false;
                _isTaskingFlag = true;
                _isTaskCompleteFlag = false;
                taskThread = new thread(taskWorkingThread, std::ref(serverSock), std::ref(clientSock), imageload, imgPrc);
            }
            if (heartbeatThread == nullptr)
            {
                /*
                  클라이언트 살아있는지 체크.
                */
                _isHeartBeatFlag = true;
                heartbeatThread = new thread(ServerCheckPartnerDeathThread, std::ref(clientSock));
            }

            _isJoingFlag = true;

            while (_isJoingFlag)
            {
                if (_isTaskCompleteFlag)
                {
                    if (taskThread->joinable())
                    {
                        _isReceiveFlag = false; // recevie thread 종료
                        taskThread->join();
                        delete taskThread;
                        taskThread = nullptr;
                    }
                    if (receiveThread->joinable())
                    {
                        receiveThread->join();
                        delete receiveThread;
                        receiveThread = nullptr;
                    }
                    if (heartbeatThread->joinable())
                    {
                        heartbeatThread->join();
                        delete heartbeatThread;
                        heartbeatThread = nullptr;
                    }
                }
                if (taskThread == nullptr && receiveThread == nullptr)
                {
                    _isJoingFlag = false;
                }
            }
            imageload.close();
            //serverSock.close();
        }
    }
    catch (std::exception& e)
    {
        std::cout << "exception: ";
        std::cout << e.what();
    }

    int a;
    std::cin >> a;
}

std::pair<std::string, string> inference(cv::Mat& img)
{

    cv::resize(img, img, _imgSize);
    
    Infer model(true, _model.c_str(), 0);
    std::vector<float> input_tensor_values = model.Mat2Vec(img, true, true);
   
    //model.PrintInputNode();
    model.SetInputOutputSet();
    model.GetOutput(input_tensor_values, _className.size());
    model.AfterProcessing(_className, _cutOff);
    string pred_class = model.GetPredictClass();
    std::pair<std::string, string> result;
    if (pred_class == "X")
        result = make_pair(pred_class, model.GetErrorPred());
    else 
        result = make_pair(pred_class, model.getProb());

   
    return result;
}

void taskWorkingThread(Server_Socket::tcp& serverSock, SOCKET& clientSock, ImageLoad imageload,ImagePreprocessing imgPrc)
{
    std::cout << "work thread start" << std::endl;

    /*string temp_folder = "E:/ai_predict_new";
    if (!fs::exists(temp_folder))
        fs::create_directory(temp_folder);*/

   /* time_t start, finish;
    double duration;

    start = time(NULL);*/

    while (_isTaskingFlag)
    {
        /*
          공유폴더로부터 이미지 로딩 후 inference
        */ 
        queue<string> images;
        /*
            이미지가 비었다면 다시 공유 폴더 조회
        */
        if (images.empty())
        {
            imageload.processing();
            images = imageload.getImages();
           
        }

        
        while (!_isPauseFlag && images.size() > 0)
        {
            /*
               클라이언트가 일시정지 및 save 요청시 아래 작업이 진행되지 않도록 하기 위한 예외처리.
           */

            std::cout << "current image count : " << images.size() << std::endl;
            
            string imgPath = images.front();
            cv::Mat img = cv::imread(imgPath, cv::IMREAD_COLOR);
           
            imgPrc.procResultImage(img);
            cv::Mat procMat = imgPrc.getImage();
            
            std::pair<std::string, std::string> results = inference(procMat);

            string predictClass = results.first;
            string classProb = results.second;

            std::cout << "classProb  " << classProb << std::endl;

            /*string dstPath = temp_folder + "/" + predictClass;
            if (!fs::exists(dstPath))
                fs::create_directory(dstPath);

            fs::copy(imgPath, dstPath, fs::copy_options::overwrite_existing);*/
            //std::cout << "test " << classProb << std::endl;

            string packetString = "data";
            int totalSize = packetString.size() + imgPath.size() + sizeof(int) + predictClass.size() + classProb.size() + 4;
            char* packet = new char[totalSize];

            sprintf_s(packet, totalSize, "%s\t%s\t%s\t%s", packetString.c_str(), imgPath.c_str(), predictClass.c_str(), classProb.c_str());
           
            /*
                socket이 연결 되어 있을때 수신 후 image 읽은것 처리.
            */
            
            if (_isHeartBeatFlag)
            {
                serverSock.send_message(clientSock, packet);
                
                int find = imgPath.rfind("\\") + 1;
                string filename = imgPath.substr(find, imgPath.length() - find); //경로에서 파일 이름 추출
                
                imageload.updatecheckflag(filename); // 이미지 추론 결과 db에 반영 
                images.pop();

            }
           
            delete[] packet;
            
        }
        //finish = time(NULL);

        //duration = (double)(finish - start);
        //cout << duration << "초" << endl;

        //std::cout << "work thread finish" << std::endl;

        
    }
    _isTaskCompleteFlag = true;
 }

void ServerCheckPartnerDeathThread(SOCKET& clientSock)
{
    try
    {
        while (_isHeartBeatFlag)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            int result = send(clientSock, NULL, 0, 0);

            if (result == -1)
            {
                std::cout << "dis connected " << std::endl;
               
                stopChangeFlag();

            }

            // 상대방 client가 connected에서 갑자기 꺼져 버리면
            // Closed로 바꾸는 방법이 정기적인 검사밖에 없음.
            // 상황보다 약간 늦게 알게되지만 상관없음.
        }
    }
    catch (std::exception& e)
    {
        std::cout << "exception: ";
        std::cout << e.what();
    }
}

void receiveSocketThread(Server_Socket::tcp& serverSock, SOCKET& clientSock)
{
    std::cout << "receive thread start" << std::endl;

    while (_isReceiveFlag)
    {
        
        try {
            std::string tmp_str = serverSock.receive(clientSock);

            if (!tmp_str.compare("pause\r\n"))
            {
                std::cout << "pause " << std::endl;
                _isPauseFlag = true;
            }
            else if (!tmp_str.compare("continue\r\n"))
            {
                std::cout << "continue " << std::endl;
                _isPauseFlag = false;   
            }
            else if (tmp_str.rfind("save\t", 0) == 0)
            {
                std::cout << "save " << std::endl;
                //saveImage(tmp_str);// 클래스 별로 폴더를 만들어 이미지 파일 저장.
                //stopChangeFlag();
            }
            else if (!tmp_str.compare("stop\r\n"))
            {
                std::cout << "stop " << std::endl;
                
                stopChangeFlag();
            }
        }
        catch (std::exception& e)
        {
            std::cout << "exception: ";
            std::cout << e.what();
        }
    }

    std::cout << "receive thread finish" << std::endl;
}

void stopChangeFlag()
{
    _isPauseFlag = true;
    _isTaskingFlag = false;
    _isHeartBeatFlag = false;

}
void saveImage(string packet)
{


}
//
//void pre_saveImage(string packet)
//{
//  /*
//  * 
//  * 케이스를 확인했다는 내용 데이터에 쓰기
//  * 
//  * 
//  * */
//
//
//    vector <string> tokens;
//
//    packet = packet.substr(5, packet.length() - 1); //save\t 제거
//    
//    /*
//        packet을 tab으로 split 하여 경로와 label 분리.
//    */
//    stringstream check1(packet);
//
//    string intermediate;
//
//    // Tokenizing w.r.t. space '\t' 
//    while (getline(check1, intermediate, '\t'))
//    {
//        tokens.push_back(intermediate);
//    }
//
//    string temp_folder = "./" + currentDateTime();
//
//    if (!fs::exists(temp_folder))
//        fs::create_directory(temp_folder);
//
//    for (int i = 0; i < tokens.size(); i = i + 2)
//    {
//        int tmp_i = i;
//        tmp_i = tmp_i + 1;
//        string dstPath = temp_folder + "/" + tokens[tmp_i];
//        if (!fs::exists(dstPath))
//            fs::create_directory(dstPath);
//       
//        fs::copy(tokens[i], dstPath, fs::copy_options::overwrite_existing);
//    }
//}