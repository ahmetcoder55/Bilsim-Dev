__asm__(".code16gcc\n");

#include "user.h"

extern unsigned char COL_BG, COL_TXT, COL_ACC, COL_ERR, COL_SUC;
extern void clear_screen(char color);
extern void draw_layout();
extern void put_str(char* s, char color, int r, int col);
extern void get_line(char* buf, int max, int r, int c);
extern int str_cmp(char* s1, char* s2);
extern void delay(int d);
extern void add_log(char* msg, char type);

extern void write_sector(unsigned char sector, void* buffer);
extern void read_sector(unsigned char sector, void* buffer);

User users[MAX_USERS];
int user_count = 0;
int current_user_idx = -1; 

void save_users() {
    write_sector(10, (void*)users);
}

void load_users() {
    read_sector(10, (void*)users);
    
    user_count = 0;
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].active == 1) {
            user_count++;
        }
    }
}

void init_users() {
    load_users();
    current_user_idx = -1;
}

void app_register() {
    clear_screen(COL_BG);
    draw_layout();
    
    put_str("=== BILSIM OS KULLANICI KAYIT ===", COL_ACC, 3, 18);
    
    if (user_count >= MAX_USERS) {
        put_str("HATA: Maksimum kullanici sinirina ulasildi!", COL_ERR, 5, 2);
        delay(120);
        return;
    }

    char u_name[MAX_USER_LEN];
    char u_pass[MAX_PASS_LEN];

    put_str("Kullanici Adi Girin: ", COL_TXT, 6, 2);
    get_line(u_name, MAX_USER_LEN - 1, 6, 23);

    if (u_name[0] == '\0') return;

    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].active && str_cmp(users[i].username, u_name)) {
            put_str("HATA: Bu kullanici adi zaten mevcut!", COL_ERR, 8, 2);
            delay(120);
            return;
        }
    }

    put_str("Sifre Girin: ", COL_TXT, 8, 2);
    get_line(u_pass, MAX_PASS_LEN - 1, 8, 15);

    if (u_pass[0] == '\0') return;

    int idx = -1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].active) {
            idx = i;
            break;
        }
    }

    if (idx != -1) {
        int j = 0;
        while (u_name[j]) { users[idx].username[j] = u_name[j]; j++; }
        users[idx].username[j] = '\0';

        j = 0;
        while (u_pass[j]) { users[idx].password[j] = u_pass[j]; j++; }
        users[idx].password[j] = '\0';

        users[idx].active = 1;
        user_count++;

        save_users();

        put_str("STATUS: Kullanici basariyla kaydedildi!", COL_SUC, 11, 2);
        add_log("Yeni kullanici kayit oldu", 0);
    } else {
        put_str("HATA: Kayit basarisiz!", COL_ERR, 11, 2);
    }

    delay(120);
}

void app_login() {
    char u_name[MAX_USER_LEN];
    char u_pass[MAX_PASS_LEN];

    while (1) {
        clear_screen(COL_BG);
        draw_layout();
        
        put_str("=== BILSIM OS KULLANICI GIRISI ===", COL_ACC, 3, 22);

        put_str("Kullanici Adi: ", COL_TXT, 7, 20);
        get_line(u_name, MAX_USER_LEN - 1, 7, 35);

        if (u_name[0] == '\0') continue;

        put_str("Sifre: ", COL_TXT, 9, 20);
        get_line(u_pass, MAX_PASS_LEN - 1, 9, 27);

        if (u_pass[0] == '\0') continue;

        // Kullanıcı ve şifre kontrolü
        int found_idx = -1;
        for (int i = 0; i < MAX_USERS; i++) {
            if (users[i].active && 
                str_cmp(users[i].username, u_name) && 
                str_cmp(users[i].password, u_pass)) {
                found_idx = i;
                break;
            }
        }

        // Başarılı girişsa döngüyü kırıp sisteme geç
        if (found_idx != -1) {
            current_user_idx = found_idx;
            put_str("STATUS: Giris basarili! Sistem baslatiliyor...", COL_SUC, 12, 20);
            add_log("Kullanici giris yapti", 0);
            delay(100);
            break; // Doğru şifre girildi, Windows gibi sisteme yönlendir
        } 
        
        // Hatalı giriş durumunda uyarı ver ve ekranı tekrar getir
        put_str("HATA: Kullanici adi veya sifre hatali!", COL_ERR, 12, 20);
        add_log("Hatali giris denemesi", 1);
        delay(120); // Uyarı yazısını okuması için kısa bekleme
    }
}