#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "database.h"

sqlite3 *db;

DbError initdb(){
    int err;
    if((err = sqlite3_open("mydb.db", &db)) != SQLITE_OK){
        printf("Cant Open DB: %s\n", sqlite3_errstr(err));
        return DB_CONNECT_FAIL;
    }

    return DB_OK;
}

DbError closedb(){
    if(sqlite3_close(db) == SQLITE_OK){
        return DB_OK;
    } else {
        return DB_CLOSE_FAIL;
    }
}

DbError getTranslation(const char* word, char* translation, const char* lan1, const char* lan2){
    char sql[] = "SELECT w2.word AS translation "
                 "FROM translations t "
                 "JOIN words w1 ON t.word_id = w1.id "
                 "JOIN words w2 ON t.translation_id = w2.id "
                 "WHERE w1.word = ? AND w1.language = ? AND w2.language = ?";
    sqlite3_stmt *statement;

    int err;
    if((err = sqlite3_prepare_v2(db, sql, -1, &statement, NULL)) != SQLITE_OK){
        printf("Failed to prepare statement: %s\n", sqlite3_errstr(err));
        return DB_PREPARE_FAIL;
    }

    if((err = sqlite3_bind_text(statement, 1, word, -1, SQLITE_STATIC)) != SQLITE_OK){
        printf("Cannot Bind parameter: %s\n", sqlite3_errstr(err));
        return DB_BIND_PARAMETER_FAIL;
    }

    if((err = sqlite3_bind_text(statement, 2, lan1, -1, SQLITE_STATIC)) != SQLITE_OK){
        printf("Cannot Bind parameter: %s\n", sqlite3_errstr(err));
        return DB_BIND_PARAMETER_FAIL;
    }

    if((err = sqlite3_bind_text(statement, 3, lan2, -1, SQLITE_STATIC)) != SQLITE_OK){
        printf("Cannot Bind parameter: %s\n", sqlite3_errstr(err));
        return DB_BIND_PARAMETER_FAIL;
    }

    if(sqlite3_step(statement) == SQLITE_ROW){
        const char* tr = (const char*)sqlite3_column_text(statement, 0);
        strcpy(translation, tr);

        sqlite3_finalize(statement);
        return DB_OK;
    }

    sqlite3_finalize(statement);
    return DB_QUERY_RESULT_EMPTY;
}

