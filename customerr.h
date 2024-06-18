#ifndef CUSTOMERR_H
#define CUSTOMERR_H

typedef enum{
    LOGIN_OK = 0,
    LOG_USERNAME_NOT_EXISTS = 1,
    LOG_WRONG_PASSWORD,
    REG_USERNAME_ALREDY_EXISTS,
} LoginError;

#endif