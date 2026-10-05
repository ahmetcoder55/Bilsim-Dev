#ifndef USER_H
#define USER_H

#define MAX_USERS 8
#define MAX_USER_LEN 16
#define MAX_PASS_LEN 16

typedef struct {
    char username[MAX_USER_LEN];
    char password[MAX_PASS_LEN];
    int active;
} User;

extern User users[MAX_USERS];
extern int user_count;

void init_users();
void save_users();
void load_users();
void app_register();
void app_login();

#endif