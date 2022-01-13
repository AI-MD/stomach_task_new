#include "Socket.h"
// SWAMI KARUPPASWAMI THUNNAI
#include "Socket.h"
#include <iostream>

Socket::Socket()
{
    // Initialize the winsock
    if ((WSAStartup(MAKEWORD(2, 2), &winsock) != 0))throw winsock_initialize_error();
    buffer = new char[buffer_size];
    memset(buffer, 0, buffer_size);


}

SOCKET Socket::get_socket() // that's fine
{
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) throw socket_error_invalid();

    DWORD timeout = 300;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(DWORD)))
    {
        perror("setsockopt");
        return -1;
    }

    return sock;
}

//void Socket::send_stuct(SOCKET& s, std::string fileName, cv::Mat& image, std::string predictClass)
//{
//     struct socketStruct {
//
//        char* filename;
//        char* predictClass;
//        char* imageData;
//
//    };
//    socketStruct sendData;
//
//    sendData.filename = (char*)fileName.c_str();
//    sendData.predictClass = (char*)predictClass.c_str();
//    sendData.imageData = (char*)image.data;
//
//    int result = send(s, (char*)&sendData, sizeof(sendData), 0);
//    if (result == SOCKET_ERROR) throw socket_error();
//}

void Socket::send_message(SOCKET& s, std::string message)
{
    // Send the message back
    int result = send(s, message.c_str(), message.size(), 0);
    if (result == SOCKET_ERROR) throw socket_error();
}

std::string Socket::receive(SOCKET& client_socket)
{
    this->resetBuffer();
    int size = recv(client_socket, buffer, buffer_size, 0);

    //std::cout << "Socket::receive : " << size << std::endl;


    std::string temp = std::string(buffer);

    return temp;
}
