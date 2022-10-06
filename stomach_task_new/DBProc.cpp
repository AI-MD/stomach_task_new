#include "DBProc.h"

sqlite3* DBProc::dbConnection()
{
    return db;
}

void DBProc::createtable()
{
    const char* sql = "DROP TABLE IF EXISTS stomach_img; CREATE TABLE stomach_img(dir Text, filename TEXT, flag boolean);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
       fprintf(stderr, "SQL error: %s\n", err_msg);

       sqlite3_free(err_msg);
       sqlite3_close(db);
    }
}
int DBProc::countSelect(std::string sql)
{

    int count_flag = 0;

    getStmt(sql);

    while ((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        count_flag = sqlite3_column_int(stmt, 0);

    }

    select_errMsg();

    return count_flag;
}

int DBProc::checkSelect(std::string sql)
{

    int check_flag = 0;
    
    getStmt(sql);

    while ((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        check_flag = sqlite3_column_int(stmt, 0);

    }
    select_errMsg();
  
    return check_flag;
}

void DBProc::insert(std::string sql)
{

    rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err_msg);
    errMsg();
    
}

void DBProc::update(std::string sql)
{
      
    // compile sql statement to binary

    rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err_msg);
  
    errMsg();
   
}
void DBProc::getStmt(std::string sqlstatement)
{
    
    if (sqlite3_prepare_v2(db, sqlstatement.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        printf("ERROR: while compiling sql: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        sqlite3_finalize(stmt);
    }
}
void DBProc::errMsg()
{
    if (rc != SQLITE_OK) {
        //this error handling could be done better, but it works
        printf("ERROR: while performing sql: %s\n", sqlite3_errmsg(db));
        printf("ret_code = %d\n", rc);

        sqlite3_free(err_msg);
        sqlite3_close(db);

    }
}

void DBProc::select_errMsg()
{
    if (ret_code != SQLITE_DONE) {
        //this error handling could be done better, but it works
        printf("ERROR: while performing sql: %s\n", sqlite3_errmsg(db));
        printf("ret_code = %d\n", ret_code);

        sqlite3_free(err_msg);
        sqlite3_close(db);
    }
}

void  DBProc::dbclose()
{
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}