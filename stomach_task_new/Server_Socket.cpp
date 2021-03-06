#include "Server_Socket.h"
#include<iostream>

Server_Socket::tcp::tcp(std::string ip_address, int port_no)
{
    // Initialize the socket to tcp
    socket = get_socket();
    // Initialize the server's ip family to IP version 4
    server.sin_family = AF_INET;
    // Server's ip address
    server.sin_addr.S_un.S_addr = inet_addr(ip_address.c_str());
    // Server's port no
    server.sin_port = htons(port_no);
    // Now bind the server on the specific port
    if (bind(socket, (struct sockaddr*)&server, sizeof(server)) < 0) throw bind_error();
}
void Server_Socket::tcp::tcp_listen(int backlog)
{
    if (listen(socket, backlog) != 0) throw listen_error();
}
int Server_Socket::tcp::accept_client(SOCKET& client, sockaddr_in& from)
{
    int size = sizeof(from);
    client = accept(socket, (sockaddr*)&from, &size);

    if (client == INVALID_SOCKET)
    {
        closesocket(socket);;
        WSACleanup();
        return -1;
    }
}
void Server_Socket::tcp::close()
{
    closesocket(socket);
}
void Server_Socket::tcp::close(SOCKET& socket)
{
    closesocket(socket);
}