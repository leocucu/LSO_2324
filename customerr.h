#ifndef CUSTOMERR_H
#define CUSTOMERR_H

typedef enum{
    LOG_USERNAME_NOT_EXISTS = 1,
    LOG_WRONG_PASSWORD,
    REG_USERNAME_ALREDY_EXISTS,
} LoginError;

#endif