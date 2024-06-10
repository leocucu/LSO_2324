#ifndef DATABASE_H
#define DATABASE_H

typedef enum{
    DB_OK = 0,
    DB_CONNECT_FAIL = 1,
    DB_QUERY_RESULT_EMPTY,
    DB_PREPARE_FAIL,
    DB_BIND_PARAMETER_FAIL,
    DB_CLOSE_FAIL
} DbError;

DbError connectdb();
DbError closedb();
DbError getTranslation(const char* word, char* translation, const char* lan1, const char* lan2);

#endif