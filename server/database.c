#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "database.h"

sqlite3 *db;

DbError connectdb(){
    int err;
    if((err = sqlite3_open("mydb.db", &db)) != SQLITE_OK){
        printf("Cant Open DB: %s\n", sqlite3_errmsg(db));
        return DB_CONNECT_FAIL;
    }

    printf("Db connected\n");
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

DbError getLogin(const char* username, char* password){
    char sql[] = "SELECT u.username AS username, u.password AS password FROM users u WHERE u.username = ?";
    sqlite3_stmt *statement;

    int err;
    if((err = sqlite3_prepare_v2(db, sql, -1, &statement, NULL)) != SQLITE_OK){
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return DB_PREPARE_FAIL;
    }

    if((err = sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC)) != SQLITE_OK){
        printf("Cannot Bind parameter: %s\n", sqlite3_errstr(err));
        return DB_BIND_PARAMETER_FAIL;
    }

    if(sqlite3_step(statement) == SQLITE_ROW){
        strcpy(password, (const char*)sqlite3_column_text(statement, 1));
        sqlite3_finalize(statement);
        return DB_OK;
    }

    sqlite3_finalize(statement);
    return DB_QUERY_RESULT_EMPTY;
}

DbError insertUser(const char* username, const char* password){
    char sql[] = "INSERT INTO users (username, password) "
                 "VALUES "
                 "(?, ?)";
    sqlite3_stmt *statement;

    int err;
    if((err = sqlite3_prepare_v2(db, sql, -1, &statement, NULL)) != SQLITE_OK){
        printf("Failed to prepare statement: %s\n", sqlite3_errstr(err));
        return DB_PREPARE_FAIL;
    }

    if((err = sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC)) != SQLITE_OK){
        printf("Cannot Bind parameter: %s\n", sqlite3_errstr(err));
        return DB_BIND_PARAMETER_FAIL;
    }

        if((err = sqlite3_bind_text(statement, 2, password, -1, SQLITE_STATIC)) != SQLITE_OK){
        printf("Cannot Bind parameter: %s\n", sqlite3_errstr(err));
        return DB_BIND_PARAMETER_FAIL;
    }

    if(sqlite3_step(statement) == SQLITE_DONE){
        sqlite3_finalize(statement);
        return DB_OK;
    }

    sqlite3_finalize(statement);
    return DB_INSERT_FAIL;
}

DbError getRoomsCount(int* count){
    char sql[] = "SELECT COUNT(*) AS count FROM rooms r";
    sqlite3_stmt *statement;

    int err;
    if((err = sqlite3_prepare_v2(db, sql, -1, &statement, NULL)) != SQLITE_OK){
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return DB_PREPARE_FAIL;
    }

    if(sqlite3_step(statement) == SQLITE_ROW){
        *(count) = sqlite3_column_int(statement, 0);
    }

    return DB_OK;
}

DbError getRooms(char *languages[], char *names[], int* max_users, int *n_room){
    char sql[] = "SELECT r.name AS name, r.language AS language, r.max AS max FROM rooms r";
    sqlite3_stmt *statement;
    int n = 0;

    int err;
    if((err = sqlite3_prepare_v2(db, sql, -1, &statement, NULL)) != SQLITE_OK){
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return DB_PREPARE_FAIL;
    }

    while(sqlite3_step(statement) == SQLITE_ROW){
        strcpy(names[n], (const char*)sqlite3_column_text(statement, 0));
        strcpy(languages[n], (const char*)sqlite3_column_text(statement, 1));
        max_users[n] = sqlite3_column_int(statement, 2);
        n++;
    }

    sqlite3_finalize(statement);
    return DB_QUERY_RESULT_EMPTY;
}