#pragma once

#include <string>

class winsock_initialize_error
{

private:
	std::string m_s;

public:
	winsock_initialize_error() :m_s("winsock_initialize_error") {};
	~winsock_initialize_error() {};

	std::string description() { return m_s; }

};


class socket_error_invalid
{

private:
	std::string m_s;

public:
	socket_error_invalid() :m_s("socket_error_invalid") {};
	~socket_error_invalid() {};

	std::string description() { return m_s; }

};

class socket_error
{

private:
	std::string m_s;

public:
	socket_error() :m_s("socket_error") {};
	~socket_error() {};

	std::string description() { return m_s; }

};


class bind_error
{

private:
	std::string m_s;

public:
	bind_error() :m_s("bind_error") {};
	~bind_error() {};

	std::string description() { return m_s; }

};


class listen_error
{

private:
	std::string m_s;

public:
	listen_error() :m_s("listen_error") {};
	~listen_error() {};

	std::string description() { return m_s; }

};
