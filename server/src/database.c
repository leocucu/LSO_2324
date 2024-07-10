#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "database.h"

sqlite3 *db;

DbError connectdb(){
    int err;
    if((err = sqlite3_open("database/mydb.db", &db)) != SQLITE_OK){
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
    char sql[] = "SELECT d.translation AS translation "
                 "FROM dictionary d "
                 "WHERE LOWER(d.word) = LOWER(?) AND d.lan1 = ? AND d.lan2 = ?";
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

DbError getLogin(const char* username, char* password, char* lan){
    char sql[] = "SELECT u.username AS username, u.password AS password, u.language AS language FROM users u WHERE u.username = ?";
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
        strcpy(lan, (const char*)sqlite3_column_text(statement, 2));
        sqlite3_finalize(statement);
        return DB_OK;
    }

    sqlite3_finalize(statement);
    return DB_QUERY_RESULT_EMPTY;
}

DbError insertUser(const char* username, const char* password, const char* language){
    char sql[] = "INSERT INTO users (username, password, language) "
                 "VALUES "
                 "(?, ?, ?)";
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

    if((err = sqlite3_bind_text(statement, 3, language, -1, SQLITE_STATIC)) != SQLITE_OK){
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

DbError getRooms(char *languages[], char *names[], int* max_users){
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
    return DB_OK;
}

DbError insertRoom(const char* name, const char* lang, int max){
    char sql[] = "INSERT INTO rooms (name, language, max) "
                 "VALUES "
                 "(?, ?, ?)";
    sqlite3_stmt *statement;

    int err;
    if((err = sqlite3_prepare_v2(db, sql, -1, &statement, NULL)) != SQLITE_OK){
        printf("Failed to prepare statement: %s\n", sqlite3_errstr(err));
        return DB_PREPARE_FAIL;
    }

    if((err = sqlite3_bind_text(statement, 1, name, -1, SQLITE_STATIC)) != SQLITE_OK){
        printf("Cannot Bind parameter: %s\n", sqlite3_errstr(err));
        return DB_BIND_PARAMETER_FAIL;
    }

    if((err = sqlite3_bind_text(statement, 2, lang, -1, SQLITE_STATIC)) != SQLITE_OK){
        printf("Cannot Bind parameter: %s\n", sqlite3_errstr(err));
        return DB_BIND_PARAMETER_FAIL;
    }

    if((err = sqlite3_bind_int(statement, 3, max)) != SQLITE_OK){
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

DbError getLanguages(char* languages){
    char sql[] = "SELECT l.language AS language FROM languages l";
    sqlite3_stmt *statement;
    languages[0] = '\0';

    int err;
    if((err = sqlite3_prepare_v2(db, sql, -1, &statement, NULL)) != SQLITE_OK){
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return DB_PREPARE_FAIL;
    }

    while(sqlite3_step(statement) == SQLITE_ROW){
        strcat(languages, (const char*)sqlite3_column_text(statement, 0));
    }

    sqlite3_finalize(statement);
    return DB_OK;
}