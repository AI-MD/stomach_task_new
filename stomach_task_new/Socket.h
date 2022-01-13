// SWAMI KARUPPASWAMI THUNNAI
#pragma once
#ifndef SOCKET_H
#define SOCKET_H
#include <windows.h>
#include <winsock.h>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "exceptions.h"

#include <iostream>

#pragma comment(lib, "ws2_32.lib")

/**
DESCRIPTION:
    In networking there are two primary types of socket
        1. TCP
        2. UDP
    This class will give you the above specified socket of your choice
*/


class Socket
{
private:
    WSADATA winsock;
    SOCKET sock;
    char* buffer;
    const int buffer_size = 4096*4;

    unsigned char* imgbuffer;

protected:


public:
    Socket();

    ~Socket()
    {
        std::cout << "test" << std::endl;
        delete[] buffer;
    }

    void resetBuffer()
    {
        memset(buffer, 0, buffer_size);
    }
    // Will return the socket of specified type

    // Throws: socket_error

    SOCKET get_socket();
    /**
    Description:
    -------------
    This method is used to send message on the specified socket
    Throws:
    --------
    socket_error
    */

    void send_message(SOCKET& s, std::string message);
   
    /*void send_stuct(SOCKET& s, std::string fileName, cv::Mat& image, std::string predictClass);*/
    /**
    Description:
    -------------
    Will receive the message from the client with
    the specified buffer size
    */
    std::string receive(SOCKET& client_socket);
};

#endif // SOCKET_H