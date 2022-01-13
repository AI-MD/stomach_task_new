#pragma once
#include <WinSock2.h>
#pragma comment( lib, "ws2_32.lib")

#include <stdio.h>

#include "opencv2/opencv.hpp"

#include <iostream> //I/O��Ʈ�� ���
#include <io.h> //���� ����ü ���
#include <string>//��Ʈ�� ��ü ��� ���
#include <list>//����Ʈ �ڷ��� ���


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
cv::Size _imgSize(224, 224);//������ �𵨿� ���� �Է� �̹����� �ػ� 


static wstring  _model = L"densenet_0624_10_224.onnx";// onnx ����
std::vector<const char*> _className = { "C", "D1","D2", "E", "S1", "S2", "S3", "S4", "S5", "S6" };// label ���� 
std::vector<float> _cutOff = { 0.98, 0.98, 0.97, 0.98, 0.98, 0.85 }; // Ŭ���� �з� �Ӱ谪  S1~S6
string path = "D:/PACS_raw_data";// ������ �н��� ���� ���� ���ð� ��ǰ�� �̹���.
//string path = "Z:/";
int port = 9999;
string ip = "127.0.0.1";
string db_name = "db_0";

 bool isCreateTableFlag = true; // ���� �ߺ��� �����ϱ� ���� db table ���� �÷���(���� 1ȸ)
 bool isRelatimeFileLoadFlag = true;//�ǽð����� ������ �о������ �ϴ� flag ���� 

/*
    thread ���� flag ����
*/

static bool _isTaskingFlag = true;
static bool _isPauseFlag = false;
static bool _isTaskCompleteFlag = false;
static bool _isReceiveFlag = true;
static bool _isJoingFlag = true;
static bool _isHeartBeatFlag = true;


/*
    ��������.
*/

std::pair<std::string, string> inference(cv::Mat& img);
void saveImage(string packet);
void receiveSocketThread(Server_Socket::tcp& server, SOCKET& client);
void taskWorkingThread(Server_Socket::tcp& server, SOCKET& client, ImageLoad imageload, ImagePreprocessing imgProc);
void ServerCheckPartnerDeathThread(SOCKET& client);
void stopChangeFlag();

int main(int argc, char* argv[] ) {
    
    bool isCreateTableFlag = true; // ���� �ߺ��� �����ϱ� ���� db table ���� �÷���(���� 1ȸ)
    bool isRelatimeFileLoadFlag = true;//�ǽð����� ������ �о������ �ϴ� flag ���� 

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
           client ����
        */

        sockaddr_in from;
        SOCKET clientSock;

        /*
           socket 1���� ������ ����ó�� �߰�
        */
        while (1)
        {
            std::cout << "The server is listening\n";
            int result = serverSock.accept_client(clientSock, from);

            /*
              �ð��� Ȯ��.
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
                path ����ó��.
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

            // false�� ���� ������ �ִ� ���� �о������ ���� 
            ImageLoad imageload = ImageLoad(path, isCreateTableFlag, isRelatimeFileLoadFlag, db_name);
            isCreateTableFlag = false;

            ImagePreprocessing imgPrc = ImagePreprocessing();

            /*
               Ŭ���̾�Ʈ ���� ����
             */

            std::cout << "Accepted connection\n";

            /*
                1:1 ���� Ŭ�� ��� ��Ȳ�̿��� recevie ������ task ������ heartbeat �����尡 ���α׷����� �ѹ��� ����ǵ��� ����
            */
            if (receiveThread == nullptr)
            {
                /*
                  recevie ������  while ��������  flag ����
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
                  Ŭ���̾�Ʈ ����ִ��� üũ.
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
                        _isReceiveFlag = false; // recevie thread ����
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
          ���������κ��� �̹��� �ε� �� inference
        */ 
        queue<string> images;
        /*
            �̹����� ����ٸ� �ٽ� ���� ���� ��ȸ
        */
        if (images.empty())
        {
            imageload.processing();
            images = imageload.getImages();
           
        }

        
        while (!_isPauseFlag && images.size() > 0)
        {
            /*
               Ŭ���̾�Ʈ�� �Ͻ����� �� save ��û�� �Ʒ� �۾��� ������� �ʵ��� �ϱ� ���� ����ó��.
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
                socket�� ���� �Ǿ� ������ ���� �� image ������ ó��.
            */
            
            if (_isHeartBeatFlag)
            {
                serverSock.send_message(clientSock, packet);
                
                int find = imgPath.rfind("\\") + 1;
                string filename = imgPath.substr(find, imgPath.length() - find); //��ο��� ���� �̸� ����
                
                imageload.updatecheckflag(filename); // �̹��� �߷� ��� db�� �ݿ� 
                images.pop();

            }
           
            delete[] packet;
            
        }
        //finish = time(NULL);

        //duration = (double)(finish - start);
        //cout << duration << "��" << endl;

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

            // ���� client�� connected���� ���ڱ� ���� ������
            // Closed�� �ٲٴ� ����� �������� �˻�ۿ� ����.
            // ��Ȳ���� �ణ �ʰ� �˰Ե����� �������.
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
                //saveImage(tmp_str);// Ŭ���� ���� ������ ����� �̹��� ���� ����.
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
//  * ���̽��� Ȯ���ߴٴ� ���� �����Ϳ� ����
//  * 
//  * 
//  * */
//
//
//    vector <string> tokens;
//
//    packet = packet.substr(5, packet.length() - 1); //save\t ����
//    
//    /*
//        packet�� tab���� split �Ͽ� ��ο� label �и�.
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