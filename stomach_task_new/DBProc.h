#pragma once
#include "sqllite3/sqlite3.h"
#include <stdio.h>
#include <iostream>

class DBProc
{
private:
	sqlite3* db;
	char* err_msg = 0;
	int rc = 0;
	int ret_code = 0;
	sqlite3_stmt* stmt = nullptr;
public:
	DBProc(std::string db_name)
	{
		rc = sqlite3_open(db_name.c_str(), &db);

		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
		}

	}
	
	void createtable();
	sqlite3* dbConnection();

	int countSelect(std::string sql);
	int checkSelect(std::string sql);
	void insert(std::string sql);
	void update(std::string sql);

	void getStmt(std::string sqlstatement);
	void errMsg();
	void select_errMsg();
	void dbclose();
};

