/* * DEVELOPER AHMET BILSIM OS/KURUCUK OS 2.0 - PROFESSIONAL OPERATING SYSTEM
 * Coded for x86 Real Mode WITH GCC SAFE OPTIMIZATIONS
 * Her hakki dunya capinda saklidir (C) 2024-2026
 */
#include "user.h"
__asm__(".code16gcc\n");
__asm__(".section .text.start\n");
__asm__(".global _start\n");
__asm__("_start:\n");
__asm__("    cli\n");              
__asm__("    xor %ax, %ax\n");     
__asm__("    mov %ax, %ds\n");
__asm__("    mov %ax, %es\n");
__asm__("    mov %ax, %ss\n");
__asm__("    mov $0x8000, %sp\n"); 
__asm__("    sti\n");              
__asm__("    call main\n");
__asm__("    jmp .\n");

#define MAX_FILES 32
#define MAX_NAME 24
#define SNAKE_MAX 256
#define LOG_MAX 12

unsigned char COL_BG  = 0x1B; 
unsigned char COL_TXT = 0x0F; 
unsigned char COL_HDR = 0x70;
unsigned char COL_ACC = 0x1E; 
unsigned char COL_ERR = 0x4F; 
unsigned char COL_SUC = 0x2E; 
#define FILE_SIZE 512

void put_str(char* s, char color, int r, int col);
void get_line(char* buf, int max, int r, int c);
int str_cmp(char* s1, char* s2);
void clear_screen(char color);
void draw_layout();
void delay(int d);
void add_log(char* msg, char type);


typedef struct
{
    char name[MAX_NAME];
    char content[FILE_SIZE]; 
    int active;
} File;

typedef struct
{
    char message[42];
    char type;
} Log;



File disk[MAX_FILES];
Log sys_logs[LOG_MAX];
int log_count = 0;
int cursor_y = 3;

char* OS_VERSION = "BILSIM OS 2.0.5 - STABLE";
char* COPYRIGHT = " Her hakki dunya capinda saklidir 2026 (C) BILSIM Worldwide ";

void put_char(char c, char color, int r, int col) {
    if (r < 0 || r >= 25 || col < 0 || col >= 80) return;
    volatile char* vmem = (volatile char*) 0xb8000;
    int pos = (r * 80 + col) * 2;
    vmem[pos] = c;
    vmem[pos + 1] = color;
}


unsigned char read_rtc_register(int reg) {
    unsigned char out_byte;
    __asm__ __volatile__ (
        "outb %%al, $0x70\n"
        "inb $0x71, %%al\n"
        "mov %%al, %0"
        : "=m"(out_byte) : "a"(reg)
    );
    return out_byte;
}

int is_rtc_updating() {
    return (read_rtc_register(0x0A) & 0x80);
}
void app_theme() {
    put_str("Tema Secin (dark, matrix, hacker, default): ", COL_ACC, cursor_y, 2);
    char t_name[16];
    get_line(t_name, 15, cursor_y, 45);
    cursor_y++;

    if (str_cmp(t_name, "matrix")) {
        COL_BG  = 0x00; 
        COL_TXT = 0x0A; 
        COL_HDR = 0x02; 
        COL_ACC = 0x0A;
        add_log("Tema degistirildi: Matrix", 0);
    } 
    else if (str_cmp(t_name, "dark")) {
        COL_BG  = 0x00; 
        COL_TXT = 0x0B; 
        COL_HDR = 0x1F; 
        COL_ACC = 0x0B;
        add_log("Tema degistirildi: Dark Neon", 0);
    } 
    else if (str_cmp(t_name, "hacker")) {
        COL_BG  = 0x00; 
        COL_TXT = 0x0C; 
        COL_HDR = 0x40; 
        COL_ACC = 0x0C;
        add_log("Tema degistirildi: Hacker Red", 0);
    } 
    else if (str_cmp(t_name, "default")) {
        COL_BG  = 0x1B;
        COL_TXT = 0x0F;
        COL_HDR = 0x70;
        COL_ACC = 0x1E;
        add_log("Tema degistirildi: Varsayilan", 0);
    } 
    else {
        put_str("HATA: Bilinmeyen tema ismi!", COL_ERR, cursor_y++, 2);
        delay(80);
        return;
    }

    clear_screen(COL_BG);
    draw_layout();
}
void get_system_time(char* time_str) {
    while (is_rtc_updating());

    unsigned char rtc_sec = read_rtc_register(0x00);
    unsigned char rtc_min = read_rtc_register(0x02);
    unsigned char rtc_hour = read_rtc_register(0x04);
    unsigned char rtc_status_b = read_rtc_register(0x0B);

    int hour, min, sec;

    if (!(rtc_status_b & 0x04)) {
        sec = ((rtc_sec & 0xF0) >> 4) * 10 + (rtc_sec & 0x0F);
        min = ((rtc_min & 0xF0) >> 4) * 10 + (rtc_min & 0x0F);
        hour = ((rtc_hour & 0xF0) >> 4) * 10 + (rtc_hour & 0x0F);
    } 
    else {
        sec = rtc_sec;
        min = rtc_min;
        hour = rtc_hour;
    }

    if (!(rtc_status_b & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    time_str[0] = (hour / 10) + '0';
    time_str[1] = (hour % 10) + '0';
    time_str[2] = ':';
    time_str[3] = (min / 10) + '0';
    time_str[4] = (min % 10) + '0';
    time_str[5] = ':';
    time_str[6] = (sec / 10) + '0';
    time_str[7] = (sec % 10) + '0';
    time_str[8] = '\0';
}

void put_str(char* s, char color, int r, int col) {
    int i = 0;
    while (s[i]) { put_char(s[i], color, r, col + i); i++; }
}

int init_mouse() {
    int status = 0;
    __asm__ __volatile__ (
        "xor %%ax, %%ax\n"      
        "int $0x33\n"
        "mov %%ax, %0"          
        : "=m"(status) : : "ax"
    );
    if (status == 0) return 0;  

    __asm__ __volatile__ (
        "mov $0x01, %%ax\n"    
        "int $0x33\n"
        : : : "ax"
    );
    return 1;
}

void hide_mouse() {
    __asm__ __volatile__ (
        "mov $0x02, %%ax\n"    
        "int $0x33\n"
        : : : "ax"
    );
}

void get_mouse_status(int* x_pos, int* y_pos, int* button) {
    short m_x = 0, m_y = 0, m_b = 0;
    __asm__ __volatile__ (
        "mov $0x03, %%ax\n"     
        "int $0x33\n"
        "mov %%bx, %0\n"        
        "mov %%dx, %2\n"        
        : "=m"(m_b), "=m"(m_x), "=m"(m_y) : : "ax", "bx", "cx", "dx"
    );
    
    *x_pos = (int)(m_x / 8);
    *y_pos = (int)(m_y / 8);
    *button = (int)m_b;
}

void fill_rect(int r, int c, int h, int w, char color) {
    for(int i = r; i < r + h; i++) {
        for(int j = c; j < c + w; j++) put_char(' ', color, i, j);
    }
}

void clear_screen(char color) { fill_rect(0, 0, 25, 80, color); }

int get_key_full() {
    short key;
    __asm__ __volatile__ (
        "xor %%ax, %%ax\n"
        "int $0x16\n"
        "mov %%ax, %0"
        : "=m"(key) : : "ax"
    );
    return (int)key;
}
void write_sector(unsigned char sector, void* buffer) {
    __asm__ __volatile__ (
        "pusha\n"
        "mov $0x0301, %%ax\n" 
        "mov $0x0000, %%ch\n" 
        "movzbl %0, %%ecx\n"  
        "mov $0x0000, %%dh\n" 
        "mov $0x0000, %%dl\n" 
        "mov %1, %%ebx\n"     
        "int $0x13\n"
        "popa\n"
        :
        : "m"(sector), "m"(buffer)
        : "memory"
    );
}

void read_sector(unsigned char sector, void* buffer) {
    __asm__ __volatile__ (
        "pusha\n"
        "mov $0x0201, %%ax\n" 
        "mov $0x0000, %%ch\n" 
        "movzbl %0, %%ecx\n"  
        "mov $0x0000, %%dh\n" 
        "mov $0x0000, %%dl\n" 
        "mov %1, %%ebx\n"     
        "int $0x13\n"
        "popa\n"
        :
        : "m"(sector), "m"(buffer)
        : "memory", "cc"
    );
}

char get_key() {
    char ascii = 0;
    __asm__ __volatile__ (
        "mov $0x00, %%ah\n"
        "int $0x16\n"
        "mov %%al, %0\n"
        : "=r"(ascii)
        :
        : "ax"
    );
    return ascii;
}

void get_line(char* buf, int max, int r, int c) {
    int pos = 0;
    while (1) {
        char ch = get_key();

        if (ch == '\r' || ch == '\n') {
            buf[pos] = '\0';
            break;
        }
        else if (ch == 0x08) {
            if (pos > 0) {
                pos--;
                put_str(" ", COL_TXT, r, c + pos); 
            }
        }
        else if (ch >= 32 && ch <= 126) {
            if (pos < max) {
                buf[pos] = ch;
                char tmp[2] = {ch, '\0'};
                put_str(tmp, COL_TXT, r, c + pos);
                pos++;
            }
        }
    }
}

int str_cmp(char* s1, char* s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return 0;
        i++;
    }
    return (s1[i] == '\0' && s2[i] == '\0');
}

void itoa(int n, char* s) {
    int i = 0, j = 0; char t[12];
    if(n == 0) { s[i++] = '0'; s[i] = '\0'; return; }
    if(n < 0) { s[i++] = '-'; n = -n; }
    while(n > 0) { t[j++] = (n % 10) + '0'; n /= 10; }
    while(j > 0) { s[i++] = t[--j]; }
    s[i] = '\0';
}

int atoi(char* s) {
    int r = 0, i = 0;
    while(s[i] >= '0' && s[i] <= '9') { r = r * 10 + (s[i] - '0'); i++; }
    return r;
}

void delay(int d) { for(volatile int i = 0; i < d * 18000; i++) __asm__("nop"); }

void add_log(char* msg, char type) {
    if(log_count >= LOG_MAX) {
        for(int i = 0; i < LOG_MAX-1; i++) sys_logs[i] = sys_logs[i+1];
        log_count = LOG_MAX - 1;
    }
    int j = 0;
    while(msg[j] && j < 41) { sys_logs[log_count].message[j] = msg[j]; j++; }
    sys_logs[log_count].message[j] = '\0';
    sys_logs[log_count].type = type;
    log_count++;
}

void draw_layout() {
    fill_rect(0, 0, 1, 80, COL_HDR);
    
    put_str(" KURUCUK OS [PROFESSIONAL] ", COL_HDR, 0, 1);
    
    put_str("BILSIM OS 2.0.5 - STABLE", COL_HDR, 0, 56); 

    fill_rect(24, 0, 1, 80, COL_HDR);
    
    put_str(" Her hakki dunya capinda saklidir 2026 (C) BILSIM ", COL_HDR, 24, 15);
}

void app_calc() {
    clear_screen(0x2E); 
    fill_rect(5, 15, 12, 50, 0x70);
    put_str("=== BILSIM CALCULATOR PRO ===", 0x70, 6, 25);
    
    char n1[12], n2[12];
    put_str("Sayi A: ", 0x70, 9, 18); get_line(n1, 10, 9, 27);
    put_str("Sayi B: ", 0x70, 10, 18); get_line(n2, 10, 10, 27);
    
    int v1 = atoi(n1);
    int v2 = atoi(n2);
    char r_str[16];
    
    itoa(v1 + v2, r_str);
    put_str("TOPLAM SONUC: ", 0x72, 13, 18); put_str(r_str, 0x72, 13, 35);
    
    put_str("ESC ile geri donun...", 0x78, 15, 18);
    while(get_key() != 27);
}

void app_snake() {
    clear_screen(0x00);
    int sx[SNAKE_MAX], sy[SNAKE_MAX];
    int len = 8, dir = 3, score = 0, fx = 35, fy = 10;
    
    for(int i = 0; i < len; i++) { sx[i] = 20 - i; sy[i] = 10; }
    
    while(1) {
        fill_rect(1, 0, 23, 80, 0x00);
        put_str("SKOR: ", 0x0E, 0, 2);
        char s_buf[10]; itoa(score, s_buf); put_str(s_buf, 0x0E, 0, 8);
        put_str("YON: OK TUSLARI/WASD | ESC: CIKIS", 0x0E, 0, 40);

        put_char('*', 0x0C, fy, fx); 
        for(int i = 0; i < len; i++) put_char('#', (i==0 ? 0x0A:0x02), sy[i], sx[i]);

        short k_raw = 0;
        __asm__ __volatile__ (
            "mov $0x01, %%ah\n"
            "int $0x16\n"
            "jz 1f\n"
            "xor %%ah, %%ah\n"
            "int $0x16\n"
            "mov %%ax, %0\n"
            "1:"
            : "=m"(k_raw) : : "ax"
        );
        
        char k = (char)(k_raw & 0xFF);
        char scan = (char)(k_raw >> 8);

        if((k == 'w' || scan == 0x48) && dir != 1) dir = 0;
        else if((k == 's' || scan == 0x50) && dir != 0) dir = 1;
        else if((k == 'a' || scan == 0x4B) && dir != 3) dir = 2;
        else if((k == 'd' || scan == 0x4D) && dir != 2) dir = 3;
        else if(k == 27) break;

        for(int i = len-1; i > 0; i--) { sx[i] = sx[i-1]; sy[i] = sy[i-1]; }
        if(dir == 0) sy[0]--; if(dir == 1) sy[0]++; if(dir == 2) sx[0]--; if(dir == 3) sx[0]++;

        if(sx[0] < 0 || sx[0] >= 80 || sy[0] < 1 || sy[0] >= 24) break;
        for(int i = 1; i < len; i++) if(sx[0] == sx[i] && sy[0] == sy[i]) goto game_over;

        if(sx[0] == fx && sy[0] == fy) {
            score += 10; if(len < SNAKE_MAX) len++;
            fx = (fx * 17 + 7) % 78 + 1; fy = (fy * 11 + 3) % 22 + 1;
        }
        delay(8);
    }
    game_over:
    put_str(" OYUN BITTI! ", 0x4F, 12, 33);
    delay(100);
}

void app_files() {
    clear_screen(0x14);
    draw_layout();
    put_str("=== SISTEM DOSYALARI (VFS) ===", 0x1F, 3, 25);
    int r = 5;
    for(int i = 0; i < MAX_FILES; i++) {
        if(disk[i].active) {
            put_str(" [FILE] ", 0x1E, r, 5);
            put_str(disk[i].name, 0x1F, r++, 15);
        }
    }
    put_str("Devam etmek icin bir tusa basin...", 0x17, 22, 23);
    get_key();
}

void touch_file() {
    put_str("Dosya Adi Olustur: ", 0x1F, cursor_y, 2);
    char n[MAX_NAME];
    get_line(n, MAX_NAME-1, cursor_y, 21);
    cursor_y++;
    for(int i = 0; i < MAX_FILES; i++) {
        if(!disk[i].active) {
            int j = 0; while(n[j]) { disk[i].name[j] = n[j]; j++; }
            disk[i].name[j] = '\0';
            disk[i].active = 1;
            put_str("STATUS: Dosya basariyla yazildi.", COL_SUC, cursor_y++, 2);
            add_log("Dosya olusturuldu", 0);
            return;
        }
    }
}

void desktop() {
    int sel = 0;
    while(1) {
        clear_screen(0x1B);
        draw_layout();
        
        char* menu[] = {
            " [1] SNAKE ULTIMATE 2.0  ", 
            " [2] HESAP MAKINESI PRO  ", 
            " [3] DOSYA YONETICISI    ", 
            " [4] SISTEM KAYITLARI    ",
            " [5] TERMINALE DON       "
        };
        
        for(int i = 0; i < 5; i++) {
            char c = (sel == i) ? 0x2F : 0x4F;
            fill_rect(7 + (i * 3), 25, 2, 30, c);
            put_str(menu[i], c, 7 + (i * 3), 28);
        }

        int k_full = get_key_full();
        char k = (char)(k_full & 0xFF);
        char scan = (char)(k_full >> 8);

        if(k == 'w' || scan == 0x48) sel = (sel > 0) ? sel - 1 : 4;
        else if(k == 's' || scan == 0x50) sel = (sel < 4) ? sel + 1 : 0;
        else if(k == '1') { sel = 0; goto activate; }
        else if(k == '2') { sel = 1; goto activate; } 
        else if(k == '3') { sel = 2; goto activate; }
        else if(k == '4') { sel = 3; goto activate; }
        else if(k == 4) { sel = 4; goto activate; }
        else if(k == 13) {
            activate:
            if(sel == 0) app_snake();
            else if(sel == 1) app_calc();
            else if(sel == 2) app_files();
            else if(sel == 3) {
                clear_screen(0x19); draw_layout();
                put_str("SISTEM LOGLARI VE DURUM:", 0x1F, 3, 10);
                for(int i=0; i<log_count; i++) {
                    put_str("> ", 0x1E, 5+i, 10);
                    put_str(sys_logs[i].message, 0x1F, 5+i, 13);
                }
                put_str("Geri donmek icin tusa basin...", 0x17, 22, 23);
                get_key();
            }
            else break;
        }
    }
}
void app_paint() {
    clear_screen(0x00);
    
    if (!init_mouse()) {
        put_str("HATA: Fare donanimi algilanamadi! Cikmak icin bir tusa basin.", 0x4F, 12, 10);
        get_key();
        return;
    }

    int mx = 0, my = 0, mb = 0;
    char current_color = 0x4F; 

    while(1) {
        hide_mouse(); 
        fill_rect(0, 0, 1, 80, 0x70);
        put_str(" KURUCUK MOUSE PAINT | SOL TIK: Ciz | SAG TIK: Renk Degistir | ESC: Cikis ", 0x70, 0, 1);
        init_mouse(); 
        get_mouse_status(&mx, &my, &mb);

        if (mb == 1 && my > 0 && my < 25 && mx >= 0 && mx < 80) {
            hide_mouse(); 
            put_char(' ', current_color, my, mx); 
            init_mouse();
        }
        
        else if (mb == 2) {
            current_color++;
            if (current_color > 0x7F) current_color = 0x1F; 
            delay(15); 
        }

        short k_check = 0;
        __asm__ __volatile__ (
            "mov $0x01, %%ah\n" 
            "int $0x16\n"
            "jz 1f\n"           
            "xor %%ah, %%ah\n"  
            "int $0x16\n"
            "mov %%ax, %0\n"
            "1:"
            : "=m"(k_check) : : "ax"
        );
        
        if ((char)(k_check & 0xFF) == 27) { 
            hide_mouse(); 
            break;
        }

        delay(1); 
    }
}
void app_education() {
    int sel = 0;
    char* dersler[] = {
        " [1] MATEMATIK ",
        " [2] EDEBIYAT  ",
        " [3] BIYOLOJI  ",
        " [4] TARIH     ",
        " [5] FIZIK     "
    };

    while(1) {
        clear_screen(0x1B); 
        draw_layout();
        
        put_str("=== BILSIM OS EGITIM PANELI ===", 0x1F, 2, 24);
        put_str("Incelemek istediginiz dersi secip Enter'a basin (ESC: Cikis):", 0x17, 4, 10);

        for(int i = 0; i < 5; i++) {
            char c = (sel == i) ? 0x2F : 0x4F; 
            fill_rect(6 + (i * 3), 5, 2, 20, c);
            put_str(dersler[i], c, 6 + (i * 3), 7);
        }

        fill_rect(6, 28, 14, 48, 0x70); // Gri arka plan kutusu
        put_str(" SECILI DERSIN DETAYLI BILGISI ", 0x7F, 6, 36);

        if (sel == 0) {
            put_str("DERS: MATEMATIK (Bilimin Dili)", 0x71, 9, 30);
            put_str("- Temel Alanlar: Cebir, Geometri, Analiz", 0x70, 11, 30);
            put_str("- Onemli Konu: Fonksiyonlar ve Limit", 0x70, 12, 30);
            put_str("- Altin Bilgi: Altin oran (1.618) dogadaki", 0x70, 14, 30);
            put_str("  kusursuz matematigi temsil eder.", 0x70, 15, 30);
        } 
        else if (sel == 1) {
            put_str("DERS: EDEBIYAT (Kulturun Aynasi)", 0x75, 9, 30);
            put_str("- Temel Alanlar: Siir, Roman, Destan, Tiyatro", 0x70, 11, 30);
            put_str("- Onemli Donem: Divan ve Tanzimat Edebiyati", 0x70, 12, 30);
            put_str("- Altin Bilgi: Dil, bir toplumun hafizasidir;", 0x70, 14, 30);
            put_str("  edebiyat ise o hafizanin sanatidir.", 0x70, 15, 30);
        } 
        else if (sel == 2) {
            put_str("DERS: BIYOLOJI (Hayat Bilimi)", 0x72, 9, 30);
            put_str("- Temel Alanlar: Sitoloji, Genetik, Anatomi", 0x70, 11, 30);
            put_str("- Onemli Konu: DNA Sarmali ve Hucre Bolunmesi", 0x70, 12, 30);
            put_str("- Altin Bilgi: Mitokondri hucrenin enerji", 0x70, 14, 30);
            put_str("  santralidir ve ATP uretir.", 0x70, 15, 30);
        } 
        else if (sel == 3) {
            put_str("DERS: TARIH (Gecmisin Izleri)", 0x76, 9, 30);
            put_str("- Temel Alanlar: Kronoloji, Arkeoloji, Siyaset", 0x70, 11, 30);
            put_str("- Onemli Donem: Dunya ve Osmanli Tarihi", 0x70, 12, 30);
            put_str("- Altin Bilgi: Gecmisini bilmeyen bir millet,", 0x70, 14, 30);
            put_str("  gelecegine yon veremez.", 0x70, 15, 30);
        } 
        else if (sel == 4) {
            put_str("DERS: FIZIK (Evrenin Kurallari)", 0x73, 9, 30);
            put_str("- Temel Alanlar: Mekanik, Optik, Termodinamik", 0x70, 11, 30);
            put_str("- Onemli Konu: Newton Kanunlari ve Elektrik", 0x70, 12, 30);
            put_str("- Altin Bilgi: Enerji asla yok olmaz,", 0x70, 14, 30);
            put_str("  sadece baska formlara donusur.", 0x70, 15, 30);
        }

        int k_full = get_key_full();
        char k = (char)(k_full & 0xFF);
        char scan = (char)(k_full >> 8);

        if (k == 'w' || scan == 0x48) sel = (sel > 0) ? sel - 1 : 4;
        else if (k == 's' || scan == 0x50) sel = (sel < 4) ? sel + 1 : 0;
        
        else if (k == '1') sel = 0;
        else if (k == '2') sel = 1;
        else if (k == '3') sel = 2;
        else if (k == '4') sel = 3;
        else if (k == '5') sel = 4;
        
        else if (k == 13) {
            add_log("Egitim panelinde ders incelendi", 0);
        }
        
        else if (k == 27) {
            break;
        }
    }
}

void welcome() {
    clear_screen(COL_ERR);
    for(int i=0; i<10; i++) {
        put_str("************************************************", (i%2==0?0x0B:0x03), 7+i, 16);
    }
    put_str("* BILSIM DEV 2.0.5 STABLE      *", 0x0E, 10, 16);
    put_str("* WORLDWIDE RIGHTS RESERVED (C) 2026     *", 0x0E, 11, 16);
    put_str("* Sistem Yukleniyor... Lutfen Bekleyin   *", 0x0F, 13, 16);
    delay(150);
}
void app_edit() {
    clear_screen(COL_BG);
    draw_layout();
    
    put_str("Duzenlenecek Dosya Adini Girin: ", 0x1F, 3, 2);
    char f_name[MAX_NAME];
    get_line(f_name, 10, 3, 34); 

    int file_idx = -1;
    for(int i = 0; i < MAX_FILES; i++) {
        if(disk[i].active) {
            int match = 1;
            for(int j = 0; f_name[j] != '\0'; j++) {
                if(disk[i].name[j] != f_name[j]) { match = 0; break; }
            }
            if(match) { file_idx = i; break; }
        }
    }

    if(file_idx == -1) {
        for(int i = 0; i < MAX_FILES; i++) {
            if(!disk[i].active) {
                int j = 0; 
                while(f_name[j]) { disk[i].name[j] = f_name[j]; j++; }
                disk[i].name[j] = ':'; 
                disk[i].name[j+1] = '\0';
                disk[i].active = 1;
                file_idx = i;
                add_log("Yeni dosya editorle olusturuldu", 0);
                break;
            }
        }
    }

    if(file_idx == -1) {
        put_str("HATA: Disk dolu!", COL_ERR, 5, 2);
        delay(100);
        return;
    }

    clear_screen(0x1F); 
    fill_rect(0, 0, 1, 80, 0x70);
    put_str(" KURUCUK NOTEPAD | METNINIZI YAZIN VE ENTER'A BASIN ", 0x70, 0, 1);
    
    put_str("Dosya icerigini girin: ", 0x1F, 3, 2);

    char input_text[MAX_NAME];
    get_line(input_text, MAX_NAME - 12, 5, 2); 

    int start_idx = 0;
    while(disk[file_idx].name[start_idx] != ':' && disk[file_idx].name[start_idx] != '\0') {
        start_idx++;
    }
    disk[file_idx].name[start_idx] = ':'; 
    start_idx++;

    int k = 0;
    while(input_text[k]) {
        disk[file_idx].name[start_idx + k] = input_text[k];
        k++;
    }
    disk[file_idx].name[start_idx + k] = '\0';

    add_log("Dosya icerigi guncellendi", 0);
    put_str("STATUS: Basariyla kaydedildi! Donuluyor...", 0x2E, 22, 2);
    delay(120);
}

void app_cat() {
    put_str("Okunacak Dosya Adi: ", 0x1F, cursor_y, 2);
    char f_name[MAX_NAME];
    get_line(f_name, 10, cursor_y, 22);
    cursor_y++;

    for(int i = 0; i < MAX_FILES; i++) {
        if(disk[i].active) {
            int match = 1;
            for(int j = 0; f_name[j] != '\0'; j++) {
                if(disk[i].name[j] != f_name[j]) { match = 0; break; }
            }
            
            if(match) {
                int sep = 0;
                while(disk[i].name[sep] != ':' && disk[i].name[sep] != '\0') sep++;
                
                put_str("--- DOSYA ICERIGI ---", 0x1E, cursor_y++, 2);
                if(disk[i].name[sep] == '\0' || disk[i].name[sep+1] == '\0') {
                    put_str("[Dosya bos]", 0x17, cursor_y++, 4);
                } else {
                    put_str(&disk[i].name[sep+1], 0x1F, cursor_y++, 4); // Sadece yazı kısmını bas
                }
                put_str("---------------------", 0x1E, cursor_y++, 2);
                add_log("Dosya icerigi okundu", 0);
                return;
            }
        }
    }
    put_str("HATA: Dosya bulunamadi!", COL_ERR, cursor_y++, 2);
}


void main() {
    COL_BG  = 0x1B; 
    COL_TXT = 0x0F; 
    COL_HDR = 0x70; 
    COL_ACC = 0x1E; 
    COL_ERR = 0x4F; 
    COL_SUC = 0x2E; 
    for(int i = 0; i < MAX_FILES; i++) disk[i].active = 0;
    init_users();
    add_log("Sistem baslatildi", 0);
add_log("BILSIM OS v2.0.5 Cekirdegi yuklendi.", 0);
add_log("[NOT] x86 Assembly baglantilari optimize edildi.", 0);
add_log("[NOT] Bilsim egitim ve Paint modulu entegre edildi.", 0);
add_log("[VIZYON] Hedef: Yerli imkanlarla mikrocekirdek mimarisi.", 0);
add_log("[CEO] Coded by Developer Ahmet ile gelecege dogru...", 0);
    int active_user_exists = 0;
    for(int i = 0; i < MAX_USERS; i++) {
        if(users[i].active) { active_user_exists = 1; break; }
    }

    if (!active_user_exists) {
        app_register();
    }
    app_login(); 

  
   while(1) {
        clear_screen(COL_BG); 
        draw_layout(); cursor_y = 2;
        put_str("Terminal Hazir. 'help' komutu ile listeleyin.", 0x1F, cursor_y++, 2);
        while(1) {
            char t_buf[12]; get_system_time(t_buf); put_str(t_buf, COL_HDR, 0, 70);

            if(cursor_y >= 23) { clear_screen(COL_BG); draw_layout(); cursor_y = 2; }
            put_str("BILSIM@dev:~$ ", 0x0A, cursor_y, 0);
            
            char cmd[64]; 
            get_line(cmd, 60, cursor_y, 16); 
            cursor_y++;
            

            if(str_cmp(cmd, "desktop")) { desktop(); break; }
            else if(str_cmp(cmd,"lessons")){
             put_str("Lessons:Matematik,Turkish,Biyologie", 0x1F, cursor_y++, 2);
            }
            else if(str_cmp(cmd, "ls")) app_files();
            else if(str_cmp(cmd, "touch")) touch_file();
            else if(str_cmp(cmd, "calc")) app_calc();
            else if(str_cmp(cmd, "cls")) { clear_screen(COL_BG); draw_layout(); cursor_y = 2; }
            else if(str_cmp(cmd, "sys")) {
                put_str("CPU: x86 Real | VER: 2.0.5 | BUILD: RELEASE", 0x1D, cursor_y++, 2);
            }
            else if(str_cmp(cmd, "help")) {
    put_str("Komutlar: desktop, ls, touch, calc, cls, sys, logs, register, reboot, about, bilsim, paint, theme", 0x1E, cursor_y++, 2);
}
else if(str_cmp(cmd, "register")) {
    app_register();
    clear_screen(COL_BG);
    draw_layout();
    cursor_y = 2;
    break;
}
            else if(str_cmp(cmd, "theme")) {
    app_theme();
    break; 
}
            else if(str_cmp(cmd, "edit")) {
    app_edit();
    break; 
}
else if(str_cmp(cmd, "cat")) {
    app_cat();
}
            else if(str_cmp(cmd, "logs") || str_cmp(cmd, "systemlog") || str_cmp(cmd, "syslog")) {
    put_str("--- KURUCUK OS GELISTIRICI NOTLARI & LIVE LOGS ---", 0x1E, cursor_y++, 2);
    
    for(int i = 0; i < log_count; i++) { 
        put_str("- ", 0x1F, cursor_y, 2); 
        put_str(sys_logs[i].message, 0x1F, cursor_y++, 4); 
    }
    
    put_str("--------------------------------------------------", 0x1E, cursor_y++, 2);
    add_log("Gelistirici notlari incelendi", 0); 
}
            else if(str_cmp(cmd, "bilsim") || str_cmp(cmd, "edu")) {
    app_education();
    break; 
}
            else if(str_cmp(cmd, "paint")) {
    app_paint();
    break; 
}
            else if(str_cmp(cmd, "about")) {
    put_str("--- BILSIM OS GELISTIRICI BILGISI ---", 0x1E, cursor_y++, 2);
    put_str("Gelistiriciler: Developer Ahmet,Abdul Aziz ve Mehmet)", 0x1F, cursor_y++, 2);
    put_str("Gorev: BILSIM - CEO & Lead OS Dev", 0x1F, cursor_y++, 2);
    put_str("Sistem Amaci: Saf x86 Real Mode mimarisinde yerli OS.", 0x1F, cursor_y++, 2);
    put_str("Sistem Amaci:MILLI Imkanlarla Ders uzerine bir isletim sistemi gelistirmek.", 0x1F, cursor_y++, 2);
    put_str("-------------------------------------", 0x1E, cursor_y++, 2);
    
    add_log("About komutu calistirildi", 0);
}
            else if(str_cmp(cmd, "logs")) {
                for(int i=0; i<log_count; i++) { put_str("- ", 0x1F, cursor_y, 2); put_str(sys_logs[i].message, 0x1F, cursor_y++, 4); }
            }
            else if(str_cmp(cmd, "reboot")) __asm__("int $0x19");
            else if(cmd[0] != '\0') {
                put_str("Hata: Gecersiz komut!", COL_ERR, cursor_y++, 2);
                add_log("Gecersiz komut denendi", 1);
            }
        }
    }
}
