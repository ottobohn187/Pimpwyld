/*
 * Pimp Wyld 3.0 Big Ten Edition.
 * Recovered from the 1999 PKLITE-packed Borland C++ DOS executable.
 * Expanded from a readable clean-room C reconstruction; not the lost source.
 */
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#define DRUG_COUNT 10
#define SCHOOL_COUNT 19
#define GUN_COUNT 10
#define DAY_LIMIT 60
#define EVENT_SCREEN_WIDTH 108
#define LOAN_GRACE_VISITS 5
#define LOAN_OVERDUE_DAMAGE 10
#define HOSPITAL_COST_PER_HP 4

#define RESET   "\x1b[0m"
#define BLACK   "\x1b[30m"
#define RED     "\x1b[91m"
#define GREEN   "\x1b[92m"
#define YELLOW  "\x1b[93m"
#define BLUE    "\x1b[94m"
#define MAGENTA "\x1b[95m"
#define CYAN    "\x1b[96m"
#define WHITE   "\x1b[97m"

typedef struct {
    const char *name;
    int low_price;
    int high_price;
} Commodity;

typedef struct {
    const char *name;
    int security;
    int control;
    int riot;
    int academics;
    int lays;
    int anarchy_sell_turns;
} School;

typedef struct {
    int day;
    int location;
    int health;
    int max_health;
    int hold_max;
    int hold[DRUG_COUNT];
    int condoms;
    int lays;
    int status;
    int riots_attempted;
    int riots_won;
    int fights_won;
    int fights_lost;
    int guns[GUN_COUNT];
    double cash;
    double bank;
    double debt;
    int loan_visits_remaining;
    double price[DRUG_COUNT];
    int condom_price;
    int girls_available;
    int girl_cost;
    uint32_t rng;
} Game;

static int interactive_terminal;

static char normalize_key(int ch) {
    if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
    return (char)ch;
}

static void enable_dos_console(void) {
    interactive_terminal =
#ifdef _WIN32
        _isatty(_fileno(stdin));
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleTitleA("Pimp Wyld 3.0");
    {
        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (GetConsoleMode(output, &mode))
            SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#else
        isatty(STDIN_FILENO);
#endif
}

static void clear_screen(void) {
    fputs("\x1b[2J\x1b[H", stdout);
    fflush(stdout);
}

static int utf8_columns(const char *text) {
    int columns = 0;
    const unsigned char *p = (const unsigned char *)text;
    while (*p) {
        if ((*p & 0xc0u) != 0x80u) ++columns;
        ++p;
    }
    return columns;
}

static void repeat_text(const char *text, int count) {
    while (count-- > 0) fputs(text, stdout);
}

static void event_border(const char *color, const char *title, int top) {
    int inside = EVENT_SCREEN_WIDTH - 2;
    fputs(color, stdout);
    if (top) {
        int title_width = utf8_columns(title) + 2;
        int left = (inside - title_width) / 2;
        fputs("╔", stdout);
        repeat_text("═", left);
        printf(" %s ", title);
        repeat_text("═", inside - left - title_width);
        puts("╗");
    } else {
        fputs("╚", stdout);
        repeat_text("═", inside);
        puts("╝" RESET);
    }
}

static void event_row(const char *color, const char *content) {
    int inside = EVENT_SCREEN_WIDTH - 2;
    int width = utf8_columns(content);
    int left = width < inside ? (inside - width) / 2 : 0;
    int right = width < inside ? inside - width - left : 0;
    fputs(color, stdout);
    fputs("║", stdout);
    repeat_text(" ", left);
    fputs(content, stdout);
    repeat_text(" ", right);
    puts("║");
}

static void event_art_row(const char *color, const char *content, int canvas_width) {
    int inside = EVENT_SCREEN_WIDTH - 2;
    int width = utf8_columns(content);
    int canvas = canvas_width < inside ? canvas_width : inside;
    int left = (inside - canvas) / 2;
    int right;
    if (width > canvas) width = canvas;
    right = inside - left - width;
    fputs(color, stdout);
    fputs("║", stdout);
    repeat_text(" ", left);
    fputs(content, stdout);
    repeat_text(" ", right);
    puts("║");
}

#include "ansi_scenes.inc"
#include "coat_only.inc"
#include "gun_dealer.inc"

static int read_key(void) {
    int ch;
#ifdef __EMSCRIPTEN__
    emscripten_sleep(1);
#endif
    if (!interactive_terminal) return getchar();
#ifdef _WIN32
    ch = _getch();
#else
    {
        struct termios old_state, raw_state;
        tcgetattr(STDIN_FILENO, &old_state);
        raw_state = old_state;
        raw_state.c_lflag &= (tcflag_t)~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw_state);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &old_state);
    }
#endif
    return ch;
}

static void wait_for_enter(void) {
    int ch;
    fputs(WHITE "\n                                                Press [Enter]" RESET, stdout);
    fflush(stdout);
    do { ch = read_key(); } while (ch != '\n' && ch != '\r' && ch != EOF);
    putchar('\n');
}

static const Commodity commodities[DRUG_COUNT] = {
    {"Cocaine", 38000, 47000}, {"Crack", 20000, 28000},
    {"Heroin", 10000, 19000}, {"Acid", 1000, 3000},
    {"Angel Dust", 1000, 2000}, {"Crystal", 500, 1100},
    {"Meth", 500, 1000}, {"Grass", 100, 350},
    {"Speed", 40, 120}, {"Ludes", 1, 25}
};

static const char *guns[GUN_COUNT] = {
    ".22 Liberator", ".45 Pistol", "M-16", "9mm Auto Pistol", "Uzi 9mm",
    "Mac-10", "AK-47", ".47mag", "Whip", "M-60"
};

static const int gun_power[GUN_COUNT] = { 8, 15, 38, 20, 30, 28, 40, 24, 5, 55 };

static const char *school_names[SCHOOL_COUNT] = {
    "Illinois", "Indiana", "Iowa", "Maryland", "Michigan", "Michigan State",
    "Minnesota", "Nebraska", "Northwestern", "Ohio State", "Oregon",
    "Penn State", "Purdue", "Rutgers", "UCLA", "USC", "Washington",
    "Wisconsin", "Big Ten Headquarters"
};

/* Approximate main-campus latitude/longitude. */
static const double campus_lat[SCHOOL_COUNT] = {
    40.1020, 39.1682, 41.6627, 38.9869, 42.2780, 42.7018,
    44.9740, 40.8202, 42.0565, 40.0076, 44.0448, 40.7982,
    40.4237, 40.5008, 34.0689, 34.0224, 47.6553, 43.0766, 41.9800
};
static const double campus_lon[SCHOOL_COUNT] = {
    -88.2272, -86.5186, -91.5549, -76.9426, -83.7382, -84.4822,
    -93.2277, -96.7005, -87.6753, -83.0300, -123.0726, -77.8599,
    -86.9212, -74.4474, -118.4452, -118.2851, -122.3035, -89.4125, -87.8600
};

static int travel_cost(int from, int to) {
    const double radians = 3.14159265358979323846 / 180.0;
    double lat1 = campus_lat[from] * radians;
    double lat2 = campus_lat[to] * radians;
    double dlat = (campus_lat[to] - campus_lat[from]) * radians;
    double dlon = (campus_lon[to] - campus_lon[from]) * radians;
    double a = sin(dlat / 2) * sin(dlat / 2) +
               cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);
    double miles = 3958.8 * 2.0 * asin(sqrt(a));
    int cost = 5 + (int)(miles / 125.0 + 0.5);
    if (cost < 5) cost = 5;
    if (cost > 25) cost = 25;
    return cost;
}

static uint32_t next_random(Game *g) {
    /* Explicit generator makes reconstructed builds reproducible. */
    g->rng = g->rng * 1103515245u + 12345u;
    return (g->rng >> 16) & 0x7fffu;
}

static int random_between(Game *g, int low, int high) {
    return low + (int)(next_random(g) % (uint32_t)(high - low + 1));
}

static double security_market_factor(int security) {
    if (security > 100) security = 100;
    if (security < 0) security = 0;
    return 1.25 - security * 0.005;
}

static void update_prices(Game *g, const School *school) {
    int i;
    double security_factor = security_market_factor(school->security);
    for (i = 0; i < DRUG_COUNT; ++i) {
        double base = random_between(g, commodities[i].low_price,
                                    commodities[i].high_price);
        double local_noise = random_between(g, 90, 110) / 100.0;
        g->price[i] = floor(base * security_factor * local_noise);
        if (g->price[i] < 1) g->price[i] = 1;
    }
    g->condom_price = random_between(g, 1, 3);
    if (school->control >= 100) {
        g->girls_available = random_between(g, 5, 9);
        g->girl_cost = random_between(g, 1, 3) * 100;
    } else {
        g->girls_available = random_between(g, 0, 9);
        g->girl_cost = random_between(g, 4, 14) * 100;
    }
}

static int apply_riot_market(Game *g, const School *school) {
    int i;
    if (school->riot < 96) return 0;
    for (i = 0; i < DRUG_COUNT; ++i)
        g->price[i] *= random_between(g, 4, 7);
    return 1;
}

static void initialize_schools(School schools[SCHOOL_COUNT]) {
    static const int academics[SCHOOL_COUNT] = {
        92, 90, 91, 93, 96, 92, 93, 90, 97, 94, 92, 94, 93, 95, 96, 94, 95, 94, 0
    };
    static const int riots[SCHOOL_COUNT] = {
        15, 20, 10, 25, 30, 15, 20, 10, 35, 25, 15, 30, 20, 25, 20, 30, 15, 20, 0
    };
    static const int security[SCHOOL_COUNT] = {
        55, 45, 50, 65, 70, 60, 55, 40, 80, 75, 35, 60, 55, 70, 65, 75, 40, 50, 100
    };
    int i;
    for (i = 0; i < SCHOOL_COUNT; ++i) {
        schools[i].name = school_names[i];
        schools[i].security = security[i];
        schools[i].control = 0;
        schools[i].riot = riots[i];
        schools[i].academics = academics[i];
        schools[i].lays = 0;
        schools[i].anarchy_sell_turns = 0;
    }
}

static void new_game(Game *g, uint32_t seed, const School schools[SCHOOL_COUNT]) {
    memset(g, 0, sizeof(*g));
    g->rng = seed ? seed : 1;
    g->day = 1;
    g->location = random_between(g, 0, SCHOOL_COUNT - 2);
    g->health = g->max_health = 100;
    g->hold_max = 10;
    g->cash = 500.0;
    g->bank = 250.0;
    g->condoms = 1;
    g->status = 1;
    g->guns[0] = 1;
    update_prices(g, &schools[g->location]);
}

static int used_hold(const Game *g) {
    int i, total = 0;
    for (i = 0; i < DRUG_COUNT; ++i) total += g->hold[i];
    return total;
}

static int owned_guns(const Game *g) {
    int i, total = 0;
    for (i = 0; i < GUN_COUNT; ++i) total += g->guns[i];
    return total;
}

static int hospital_price(int missing_health) {
    return missing_health > 0 ? missing_health * HOSPITAL_COST_PER_HP : 0;
}

static int affordable_units(double cash, double unit_price) {
    if (cash <= 0 || unit_price <= 0) return 0;
    return (int)floor(cash / unit_price);
}

static int purchase_drug(Game *g, int index, int count) {
    int capacity;
    if (index < 0 || index >= DRUG_COUNT || count <= 0) return 0;
    capacity = g->hold_max - used_hold(g);
    if (count > capacity) return -1;
    if (g->price[index] * count > g->cash) return -2;
    g->cash -= g->price[index] * count;
    g->hold[index] += count;
    return 1;
}

static int purchase_guns(Game *g, int index, int count, double total) {
    if (index < 0 || index >= GUN_COUNT || count <= 0 || total < 0) return 0;
    if (g->cash < total) return -1;
    g->cash -= total;
    g->guns[index] += count;
    return 1;
}

static int purchase_lay(Game *g, School *school) {
    if (g->condoms <= 0) return -1;
    if (g->girls_available <= 0) return -2;
    if (g->cash < g->girl_cost) return -3;
    --g->girls_available;
    --g->condoms;
    g->cash -= g->girl_cost;
    ++g->lays;
    ++school->lays;
    ++g->status;
    return 1;
}

static int read_int(const char *prompt) {
    char line[80];
    long value;
    char *end;
    printf("%s", prompt);
    fflush(stdout);
    for (;;) {
#ifdef __EMSCRIPTEN__
        emscripten_sleep(1);
#endif
        if (!fgets(line, sizeof(line), stdin)) return 0;
        value = strtol(line, &end, 10);
        if (end != line) return (int)value;
        /* A menu key followed by Enter can leave a blank line behind when
           single-key input is active. Ignore it instead of buying zero. */
        if (line[0] == '\n' || line[0] == '\r') continue;
        puts(RED "Please enter a number." RESET);
        printf("%s", prompt);
        fflush(stdout);
    }
}

static char read_command(const char *prompt) {
    int ch;
    printf("%s", prompt);
    fflush(stdout);
    do { ch = read_key(); } while (ch == '\n' || ch == '\r');
    if (ch == EOF) return 'q';
    if (!interactive_terminal) {
        int rest;
        do { rest = getchar(); } while (rest != '\n' && rest != EOF);
    }
    printf("%c\n", ch);
    return normalize_key(ch);
}

static void title_bar(void) {
    puts(MAGENTA "╔═══════════════════════" RED " * * Pimp Wyld * " CYAN "Ver. 3.0" GREEN
         " * * " MAGENTA "═══════════════════════╗" RESET);
    puts(MAGENTA "╚════════════════════════════════════════════════════════════════════════════╝" RESET);
}

static void show_splash(void) {
    if (!interactive_terminal) return;
    clear_screen();
    show_neon_splash_raster();
    puts(YELLOW  "                    VERSION 3.0  •  60 DAYS  •  18 CAMPUSES  •  TOTAL ANARCHY" RESET);
    fputs(WHITE  "                                              [ PRESS ANY KEY ]" RESET, stdout);
    fflush(stdout);
    read_key();
    putchar('\n');
}

static void show_street_attack_art(void) {
    clear_screen();
    event_border(RED, "STREET AMBUSH", 1);
    event_row(BLUE,    "▄██▄                ▄▄▄▄▄                 ▄██▄");
    event_row(WHITE,   "█ ▀▀ █        ══════╬█████╬══════         █ ▀▀ █");
    event_row(MAGENTA, "▀██▀              ▄███████▄               ▀██▀");
    event_row(CYAN,    "▄██████▄           ██  YOU  ██            ▄██████▄");
    event_row(RED,     "██ ═╦══ ██   >>>    ██ ▄███▄ ██    <<<    ██ ══╦ ██");
    event_row(YELLOW,  "▄██▄              ▀███████▀              ▄██▄");
    event_row(WHITE,   "NO EXIT LEFT.  NO EXIT RIGHT.  FIGHT THROUGH.");
    event_border(RED, "", 0);
}

static void show_mugging_art(void) {
    clear_screen();
    event_border(RED, "HOLD-UP", 1);
    event_art_row(RED,     "        .-------.                                             \\O/", 78);
    event_art_row(WHITE,   "       /  o   o  \\                                             |", 78);
    event_art_row(RED,     "       |    ^    |__     \\_____[===========================>   |", 78);
    event_art_row(MAGENTA, "       \\  '---'  /                 GIVE IT UP!                / \\", 78);
    event_art_row(CYAN,    "        '---.---'                                             YOU", 78);
    event_art_row(CYAN,    "           /|\\", 78);
    event_art_row(YELLOW,  "          / | \\              CASH OR 20 STATUS", 78);
    event_art_row(WHITE,   "           / \\               DECIDE FAST", 78);
    event_border(RED, "", 0);
}

static void show_lay_art(void) {
    clear_screen();
    event_border(MAGENTA, "AFTER HOURS", 1);
    event_row(BLUE,    ".-''''-.          .-''''-.");
    event_row(CYAN,    "/  .--.  \\        /  .--.  \\");
    event_row(WHITE,   "|  (♥  ♥)  |      |  (♥  ♥)  |");
    event_row(MAGENTA, "\\   ▿   /        \\   ▿   /");
    event_row(RED,     "'.___.'    ♥ ♥    '.___.'");
    event_row(CYAN,    "\\      SAFE NIGHT      /");
    event_row(YELLOW,  "CONDOM USED • IMPACT +1");
    event_border(MAGENTA, "", 0);
}

static void show_boyfriend_art(void) {
    clear_screen();
    event_border(RED, "UNINVITED GUEST", 1);
    event_art_row(YELLOW,  "                            .--------.", 78);
    event_art_row(WHITE,   "                           /  >    <  \\", 78);
    event_art_row(RED,     "                          |     __     |     WHO ARE YOU?!", 78);
    event_art_row(RED,     "                           \\  '---'  /", 78);
    event_art_row(MAGENTA, "                            '---+---'", 78);
    event_art_row(BLUE,    "                               /|\\", 78);
    event_art_row(CYAN,    "                              / | \\", 78);
    event_art_row(CYAN,    "                               / \\", 78);
    event_row(WHITE,       "THE JEALOUS BOYFRIEND BLOCKS YOUR PATH");
    event_border(RED, "", 0);
}

static void show_riot_crowd_art(void) {
    clear_screen();
    event_border(RED, "RIOT RISING", 1);
    event_art_row(MAGENTA, "       [ A ]   [ N ]   [ A ]   [ R ]   [ C ]   [ H ]   [ Y ]", 78);
    event_art_row(YELLOW,  "         |       |       |       |       |       |       |", 78);
    event_art_row(WHITE,   "        \\O/     \\O/     \\O/     \\O/     \\O/     \\O/     \\O/", 78);
    event_art_row(CYAN,    "         |       |       |       |       |       |       |", 78);
    event_art_row(CYAN,    "        / \\     / \\     / \\     / \\     / \\     / \\     / \\", 78);
    event_art_row(WHITE,   "   ----------------------------------------------------------------------", 78);
    event_row(RED,     "THE CROWD IS LOUD.  THE SECURITY LINE IS FORMING.");
    event_row(YELLOW,  "BUILD 60 RIOT POINTS — THEN FIGHT");
    event_border(RED, "", 0);
}

static void show_market_panic_art(void) {
    clear_screen();
    event_border(RED, "MARKET PANIC", 1);
    event_art_row(YELLOW,  "       COCAINE  $$$$$$$ ↑↑              HEROIN   $$$$$ ↑↑", 78);
    event_art_row(MAGENTA, "       CRACK    $$$$$$  ↑↑              CRYSTAL  $$$$  ↑↑", 78);
    event_art_row(CYAN,    "                         ╔══════════════╗", 78);
    event_art_row(WHITE,   "                         ║    SOLD!     ║      SUPPLY", 78);
    event_art_row(RED,     "                         ║    SOLD!     ║      COLLAPSED", 78);
    event_art_row(YELLOW,  "                         ╚══════════════╝      PRICES EXPLODE", 78);
    event_border(RED, "", 0);
}

static void left_market_row(const Game *g, int row) {
    const char *edge = (row == 5 || row == 10) ? "├" : "│";
    if (row < DRUG_COUNT) {
        printf(WHITE "│  %-14s %8.0f  " YELLOW "%4d     " WHITE "%s", commodities[row].name,
               g->price[row], g->hold[row], edge);
    } else if (row == 10) {
        printf(WHITE "│  " RED "%-14s" WHITE " %8d  " YELLOW "%4d     " WHITE "%s",
               "Condoms", g->condom_price, g->condoms, edge);
    } else if (row == 11) {
        printf(WHITE "│  " GREEN "Money: %-27.0f" WHITE "│", g->cash);
    } else {
        printf(WHITE "│  " CYAN "Bank:  %-27.0f" WHITE "│", g->bank);
    }
}

static void right_status_row(const Game *g, const School schools[SCHOOL_COUNT], int row) {
    const School *s = &schools[g->location];
    int i, gun_total = 0;
    char value[32];
    for (i = 0; i < GUN_COUNT; ++i) gun_total += g->guns[i];
    switch (row) {
        case 0: printf(WHITE "  " CYAN "Campus: " WHITE "%-29s│", s->name); break;
        case 1: snprintf(value, sizeof(value), "%d%%", s->security);
                printf(WHITE "  " CYAN "Security: " WHITE "%-27s│", value); break;
        case 2: printf(WHITE "  " CYAN "Control: " WHITE "%-28s│",
                       s->control ? "ANARCHY" : "ORDER"); break;
        case 3: snprintf(value, sizeof(value), "%d%%", s->riot);
                printf(WHITE "  " CYAN "Riot Points: " WHITE "%-24s│", value); break;
        case 4: printf(WHITE "  " CYAN "Academics: " WHITE "%-26d│", s->academics); break;
        case 5: printf(WHITE "──STATS────────────────────────────────┤"); break;
        case 6: printf(WHITE "     " CYAN "Health: " WHITE "%-4d      " CYAN "Max Hold: " WHITE "%-6d│",
                       g->health, g->hold_max); break;
        case 7: printf(WHITE "       " CYAN "Lays: " WHITE "%-4d       " CYAN "Condoms: " WHITE "%-6d│",
                       g->lays, g->condoms); break;
        case 8: printf(WHITE "    " CYAN "In Debt: " WHITE "%-9.0f                 │", g->debt); break;
        case 9: printf(WHITE "     " CYAN "Status: " YELLOW "%-4d          " CYAN "Guns: " WHITE "%-6d│",
                       g->status, gun_total); break;
        case 10: printf(WHITE "──GIRLS────────────────────────────────┤"); break;
        case 11: printf(WHITE "  " CYAN "Available: " WHITE "%-26d│", g->girls_available); break;
        default: printf(WHITE "    " CYAN "Cost per: " WHITE "$%-24d│", g->girl_cost); break;
    }
}

static void draw_dashboard(const Game *g, const School schools[SCHOOL_COUNT]) {
    int row;
    clear_screen();
    title_bar();
    printf(YELLOW "Days Left: %d" RESET "\n", DAY_LIMIT - g->day + 1);
    if (g->debt > 0) {
        if (g->loan_visits_remaining > 0)
            printf(YELLOW "Loan ticker: %d campus visit%s until health penalties begin.\n" RESET,
                   g->loan_visits_remaining,
                   g->loan_visits_remaining == 1 ? "" : "s");
        else
            printf(RED "Loan ticker: OVERDUE - each campus visit costs %d health.\n" RESET,
                   LOAN_OVERDUE_DAMAGE);
    }
    if (schools[g->location].anarchy_sell_turns > 0)
        printf(MAGENTA "Anarchy market: drug sales pay double for %d more turn%s.\n" RESET,
               schools[g->location].anarchy_sell_turns,
               schools[g->location].anarchy_sell_turns == 1 ? "" : "s");
    puts(WHITE "┌── DRUGS ─────────── PRICES ── Hold ─┬── CONDITIONS ────────────────────────┐");
    for (row = 0; row < 13; ++row) {
        left_market_row(g, row);
        right_status_row(g, schools, row);
        putchar('\n');
    }
    puts(WHITE "└─────────────────────────────────────┴──────────────────────────────────────┘" RESET);
}

static void show_main_menu(const char *message) {
    if (message && *message)
        printf(RED "%-78s\n" RESET, message);
    else
        putchar('\n');
    printf(GREEN " (B)uy       (V)isit Bank       " YELLOW "(H)ospital       (C)ampuses\n");
    printf(GREEN " (S)ell      (L)oan Shark       " YELLOW "(D)rive     (R)iot     "
           CYAN "(T)est     " RED "(Q)uit\n" RESET);
}

static void show_trade_choices(const char *heading) {
    puts(GREEN "\n==================* DRUGS *==================");
    printf(MAGENTA " (A) " WHITE "Cocaine            " MAGENTA "(B) " WHITE "Crack\n");
    printf(MAGENTA " (C) " WHITE "Heroin             " MAGENTA "(D) " WHITE "Acid\n");
    printf(MAGENTA " (E) " WHITE "Angel Dust         " MAGENTA "(F) " WHITE "Crystal\n");
    printf(MAGENTA " (G) " WHITE "Meth               " MAGENTA "(H) " WHITE "Grass\n");
    printf(MAGENTA " (I) " WHITE "Speed              " MAGENTA "(J) " WHITE "Ludes\n");
    printf(MAGENTA " (K) " CYAN "Condom             " MAGENTA "(L) " CYAN "Girl\n" RESET);
    printf(YELLOW " (Esc) Back\n%s" RESET, heading);
}

static void jealous_boyfriend_attack(Game *g);

static void buy(Game *g, School schools[SCHOOL_COUNT]) {
    int index, count, capacity, affordable, maximum, result;
    show_trade_choices("Watcha need? ");
    index = read_command("") - 'a';
    if (index == 10) {
        affordable = affordable_units(g->cash, g->condom_price);
        printf(YELLOW "Condoms: %d       Cash: $%.0f       Price: $%d       You can buy: %d\n" RESET,
               g->condoms, g->cash, g->condom_price, affordable);
        count = read_int("How many? ");
        if (count <= 0) return;
        if (count * g->condom_price <= g->cash) {
            g->condoms += count;
            g->cash -= count * g->condom_price;
            printf(GREEN "Bought %d condom%s for $%d. Cash remaining: $%.0f.\n" RESET,
                   count, count == 1 ? "" : "s", count * g->condom_price, g->cash);
        } else {
            puts(RED "You don't have enough cash for all of that!!!" RESET);
        }
        wait_for_enter();
        return;
    }
    if (index == 11) {
        printf(YELLOW "Cash: $%.0f       Cost: $%d       You can afford: %s\n" RESET,
               g->cash, g->girl_cost, g->cash >= g->girl_cost ? "1" : "0");
        int lay_result = purchase_lay(g, &schools[g->location]);
        if (lay_result == -1) {
            puts(RED "No condom, no lay. Buy condoms first." RESET);
        } else if (lay_result == 1) {
            show_lay_art();
            printf(GREEN "Lay completed safely. One condom used. Cash remaining: $%.0f.\n" RESET,
                   g->cash);
            jealous_boyfriend_attack(g);
        } else if (lay_result == -2) {
            puts(RED "No one is available right now." RESET);
        } else {
            puts(RED "You don't have enough cash for that!!!" RESET);
        }
        wait_for_enter();
        return;
    }
    if (index < 0 || index >= DRUG_COUNT) return;
    capacity = g->hold_max - used_hold(g);
    affordable = affordable_units(g->cash, g->price[index]);
    maximum = affordable < capacity ? affordable : capacity;
    printf(YELLOW "Cash: $%.0f       Price: $%.0f       Hold space: %d\n" RESET,
           g->cash, g->price[index], capacity);
    printf(YELLOW "You can buy: %d %s\n" RESET, maximum, commodities[index].name);
    count = read_int("Buy how many? ");
    result = purchase_drug(g, index, count);
    if (result == 1)
        printf(GREEN "Bought %d %s for $%.0f. Cash remaining: $%.0f.\n" RESET,
               count, commodities[index].name, g->price[index] * count, g->cash);
    else if (result == -1)
        printf(RED "Your hold is too full! Only %d space remains.\n" RESET, capacity);
    else if (result == -2)
        puts(RED "You don't have enough cash for all of that!!!" RESET);
    if (result != 0) wait_for_enter();
}

static void sell(Game *g, School schools[SCHOOL_COUNT]) {
    int index, count;
    double multiplier = schools[g->location].anarchy_sell_turns > 0 ? 2.0 : 1.0;
    show_trade_choices("Watcha selling? ");
    index = read_command("") - 'a';
    if (index == 10) {
        count = read_int("Sell how many condoms? ");
        if (count > 0 && count <= g->condoms) {
            g->condoms -= count;
            g->cash += count * g->condom_price;
            printf(GREEN "Sold %d condom%s for $%d. Cash now: $%.0f.\n" RESET,
                   count, count == 1 ? "" : "s", count * g->condom_price, g->cash);
        } else if (count > g->condoms) {
            puts(RED "Hey!! You don't have that many!!!" RESET);
        }
        if (count > 0) wait_for_enter();
        return;
    }
    if (index < 0 || index >= DRUG_COUNT) return;
    count = read_int("Sell how many? ");
    if (count <= 0) return;
    if (count > g->hold[index]) {
        puts(RED "Hey!! You don't have that much!!!" RESET);
        wait_for_enter();
        return;
    }
    g->hold[index] -= count;
    g->cash += g->price[index] * count * multiplier;
    printf(GREEN "Sold %d %s for $%.0f. Cash now: $%.0f.\n" RESET,
           count, commodities[index].name, g->price[index] * count * multiplier, g->cash);
    if (multiplier > 1.0)
        puts(MAGENTA "ANARCHY MARKET: sale paid double!" RESET);
    wait_for_enter();
}

static void bank(Game *g) {
    char command = read_command("Bank Clerk: (D)eposit or (W)ithdraw? ");
    int amount = read_int(command == 'd' ? "Deposit: " : "Withdraw: ");
    if (amount <= 0) return;
    if (command == 'd') {
        if (amount > g->cash) puts("Hey!!! You don't have that!!");
        else { g->cash -= amount; g->bank += amount; }
    } else if (command == 'w') {
        if (amount > g->bank) puts("Hey!!! You don't have that!!");
        else { g->bank -= amount; g->cash += amount; }
    }
}

static void loan_shark(Game *g) {
    char command = read_command("Loan Shark: (R)eceive loan or (P)ay back? ");
    int amount = read_int(command == 'r' ? "Take: " : "Pay: ");
    double ceiling = 500.0 + g->status * 250.0;
    if (amount <= 0) return;
    if (command == 'r') {
        if (g->debt > 0) puts("I've already given you some money!");
        else if (amount > ceiling) puts("No way. That's too much!!!");
        else {
            g->cash += amount;
            g->debt += amount;
            g->loan_visits_remaining = LOAN_GRACE_VISITS;
            printf(YELLOW "You have %d campus visits to repay before health penalties begin.\n" RESET,
                   LOAN_GRACE_VISITS);
        }
    } else if (command == 'p') {
        if (amount > g->cash) puts("Hey you don't have that cash!!");
        else {
            if (amount > g->debt) amount = (int)g->debt;
            g->cash -= amount;
            g->debt -= amount;
            if (g->debt <= 0) {
                g->debt = 0;
                g->loan_visits_remaining = 0;
                puts(GREEN "Debt cleared. The loan ticker is gone." RESET);
            }
        }
    }
}

static int apply_loan_visit(Game *g) {
    if (g->debt <= 0) {
        g->loan_visits_remaining = 0;
        return 0;
    }
    if (g->loan_visits_remaining > 0) {
        --g->loan_visits_remaining;
        return 0;
    }
    g->health -= LOAN_OVERDUE_DAMAGE;
    return LOAN_OVERDUE_DAMAGE;
}

static void hospital(Game *g) {
    int missing = g->max_health - g->health;
    int full_price = hospital_price(missing);
    int heal_amount = missing;
    int price;
    clear_screen();
    puts(CYAN "==================== CAMPUS HOSPITAL ====================" RESET);
    printf(WHITE "Health: %d/%d     Cash: $%.0f\n" RESET,
           g->health, g->max_health, g->cash);
    if (missing <= 0) {
        puts(GREEN "Nurse: You're already at full health." RESET);
        wait_for_enter();
        return;
    }
    if (g->cash < HOSPITAL_COST_PER_HP) {
        printf(RED "Treatment costs $%d per health point. You need at least $%d.\n" RESET,
               HOSPITAL_COST_PER_HP, HOSPITAL_COST_PER_HP);
        wait_for_enter();
        return;
    }
    if (full_price > g->cash) heal_amount = (int)g->cash / HOSPITAL_COST_PER_HP;
    price = hospital_price(heal_amount);
    if (heal_amount == missing)
        printf(YELLOW "Full treatment: restore %d health for $%d.\n" RESET,
               heal_amount, price);
    else
        printf(YELLOW "You cannot afford the full $%d treatment.\n"
               "Affordable treatment: restore %d health for $%d.\n" RESET,
               full_price, heal_amount, price);
    if (read_command("Take the treatment? (Y/N)? ") == 'y') {
        g->cash -= price;
        g->health += heal_amount;
        if (g->health > g->max_health) g->health = g->max_health;
        printf(GREEN "Treatment complete. Health: %d/%d. Cash remaining: $%.0f.\n" RESET,
               g->health, g->max_health, g->cash);
    } else {
        puts(YELLOW "You leave without treatment." RESET);
    }
    wait_for_enter();
}

static void mugging(Game *g) {
    if (g->cash < 1500) return;
    show_mugging_art();
    puts(RED "\nA pimple-ass freak comes up with a 9mm to your head..." RESET);
    puts(WHITE "Freak: Give me all your money!!!! NOW!!!" RESET);
    if (g->status >= 20 && read_command("Use 20 Status points to get out? (Y/N)? ") == 'y') {
        g->status -= 20;
        ++g->fights_won;
        puts(GREEN "You grab the attacker, kick him, and get away." RESET);
    } else {
        ++g->fights_lost;
        g->cash = 0;
        if (random_between(g, 1, 4) == 1) {
            int damage = random_between(g, 5, 25);
            g->health -= damage;
            printf(RED "*BLAM* You lose %d health.\n" RESET, damage);
        } else {
            puts(RED "The freak takes your cash and runs." RESET);
        }
    }
    wait_for_enter();
}

static void gun_dealer(Game *g) {
    static const char *dealers[] = {"Harvey Bohn", "Derek", "A. Bosch", "Ruiner"};
    const char *dealer = dealers[random_between(g, 0, 3)];
    int count = random_between(g, 1, 3);
    double total = count * 3500.0;
    clear_screen();
    show_rainy_gun_dealer();
    printf(MAGENTA "\n%s, the cool anarchist, comes up with a duffel bag...\n" RESET,
           dealer);
    printf(WHITE "%s: I have %d AK-47%s for only $%.0f. Deal?\n" RESET,
           dealer, count, count == 1 ? "" : "s", total);
    printf(CYAN "You currently own %d AK-47%s and have $%.0f.\n" RESET,
           g->guns[6], g->guns[6] == 1 ? "" : "s", g->cash);
    if (read_command(YELLOW "Buy it? (Y/N)? " RESET) == 'y') {
        if (purchase_guns(g, 6, count, total) < 0) {
            puts(RED "Hey!! You don't have that money!!" RESET);
        } else {
            printf(GREEN "You bought %d AK-47%s. Cash remaining: $%.0f.\n" RESET,
                   count, count == 1 ? "" : "s", g->cash);
        }
    }
    wait_for_enter();
}

static void trenchcoat_dealer(Game *g) {
    int extra = random_between(g, 5, 15);
    double cost = extra * 175.0;
    clear_screen();
    show_coat_only_raster();
    puts(MAGENTA "\nA shadowy coat dealer steps out beneath a dead streetlight." RESET);
    printf(WHITE "It adds %d spaces to your hold. Yours for $%.0f.\n" RESET,
           extra, cost);
    if (read_command(YELLOW "Buy it? (Y/N)? " RESET) == 'y') {
        if (g->cash < cost) {
            puts(RED "Hey!! You don't have that money!!" RESET);
        } else {
            g->cash -= cost;
            g->hold_max += extra;
            printf(GREEN "New maximum hold: %d. Cash remaining: $%.0f.\n" RESET,
                   g->hold_max, g->cash);
        }
    }
    wait_for_enter();
}

static void street_attack(Game *g, const School *school) {
    int attackers = random_between(g, 1, 2 + school->security / 25);
    int i, gun_count = owned_guns(g), defense, enemy;
    char action;
    show_street_attack_art();
    printf(RED "\n%d hostile %s jump%s you near %s!\n" RESET,
           attackers, attackers == 1 ? "person" : "people",
           attackers == 1 ? "s" : "", school->name);
    if (gun_count > 0) {
        printf(CYAN "You have %d gun%s.\n" RESET, gun_count, gun_count == 1 ? "" : "s");
        action = read_command(YELLOW "(F)ight with a gun or (E)scape? " RESET);
    } else {
        puts(YELLOW "You have no gun. Your only chance is to escape." RESET);
        action = 'e';
    }
    if (action != 'f') {
        int escape_chance = 65 + g->status - school->security / 2;
        if (escape_chance < 20) escape_chance = 20;
        if (escape_chance > 85) escape_chance = 85;
        if (random_between(g, 1, 100) <= escape_chance) {
            puts(GREEN "You break away and escape the ambush." RESET);
            wait_for_enter();
            return;
        }
        puts(RED "They cut off your escape. You have to defend yourself!" RESET);
    }
    defense = g->status + g->health / 4 + random_between(g, 0, 35);
    enemy = attackers * 12 + school->security / 2 + random_between(g, 0, 25);
    if (action == 'f') {
        for (i = 0; i < GUN_COUNT; ++i) defense += g->guns[i] * gun_power[i];
        puts(CYAN "You draw your weapon and fight back!" RESET);
    }
    if (defense >= enemy) {
        int reward = random_between(g, 50, 250) * attackers;
        g->cash += reward;
        g->status += attackers;
        ++g->fights_won;
        printf(GREEN "You fight them off and recover $%d.\n" RESET, reward);
        if (random_between(g, 1, 5) == 1) {
            int found = random_between(g, 0, 5);
            ++g->guns[found];
            printf(GREEN "One dropped a %s. You keep it.\n" RESET, guns[found]);
        }
    } else {
        int damage = random_between(g, 8, 18) * attackers;
        double stolen = g->cash < 500 ? g->cash : random_between(g, 100, 500);
        g->health -= damage;
        ++g->fights_lost;
        g->cash -= stolen;
        printf(RED "They overpower you: -%d health and -$%.0f.\n" RESET,
               damage, stolen);
    }
    wait_for_enter();
}

static void jealous_boyfriend_attack(Game *g) {
    int i, defense, enemy, chance = 12 + g->lays * 3;
    if (chance > 45) chance = 45;
    if (random_between(g, 1, 100) > chance) return;
    defense = g->status + g->health / 4 + random_between(g, 0, 30);
    for (i = 0; i < GUN_COUNT; ++i) defense += g->guns[i] * gun_power[i];
    enemy = 30 + g->lays * 3 + random_between(g, 0, 35);
    show_boyfriend_art();
    puts(RED "\nHer jealous adult boyfriend storms in looking for a fight!" RESET);
    if (defense >= enemy) {
        int cash = random_between(g, 75, 300);
        ++g->fights_won;
        g->status += 2;
        g->cash += cash;
        printf(GREEN "You send him packing. Status +2, cash recovered: $%d.\n" RESET,
               cash);
    } else {
        int damage = random_between(g, 8, 28);
        ++g->fights_lost;
        g->health -= damage;
        printf(RED "He gets the better of you. Health -%d.\n" RESET, damage);
    }
}

static void random_travel_event(Game *g, const School *school) {
    int roll = random_between(g, 1, 100);
    int attack_chance = 8 + g->riots_attempted * 2;
    if (attack_chance > 30) attack_chance = 30;
    if (roll <= attack_chance) street_attack(g, school);
    else if (roll <= attack_chance + 11) gun_dealer(g);
    else if (roll <= attack_chance + 20) trenchcoat_dealer(g);
    else if (roll <= attack_chance + 30 && g->cash >= 1500) mugging(g);
}

static void riot(Game *g, School schools[SCHOOL_COUNT]);

static void drive(Game *g, School schools[SCHOOL_COUNT]) {
    int i, choice, cost;
    char command;
    clear_screen();
    printf(WHITE "  Place                         How Much?    Control    Riot %%    Security\n");
    puts(GREEN "-==-{DRIVE}-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-" RESET);
    for (i = 0; i < SCHOOL_COUNT; ++i)
        printf(RED "(%c) " MAGENTA "%-27s " CYAN "%3d       " RED "%-9s %5d       %3d\n",
               'A' + i, schools[i].name, travel_cost(g->location, i),
               schools[i].control ? "ANARCHY" : "ORDER", schools[i].riot,
               schools[i].security);
    command = read_command(RED "                         Drive to what area? " RESET);
    choice = command - 'a';
    if (choice < 0 || choice >= SCHOOL_COUNT || choice == g->location) return;
    cost = travel_cost(g->location, choice);
    if (g->cash < cost) {
        puts("Sorry!! You need the cash to flash!!");
        return;
    }
    g->cash -= cost;
    g->location = choice;
    ++g->day;
    if (g->debt > 0) g->debt = floor(g->debt * 1.10);
    {
        int loan_damage = apply_loan_visit(g);
        if (loan_damage > 0)
            printf(RED "The loan shark collects in blood: -%d health. Pay the debt to stop it.\n" RESET,
                   loan_damage);
        else if (g->debt > 0 && g->loan_visits_remaining == 0)
            puts(RED "Your loan is now overdue. Future campus visits will cost health." RESET);
    }
    schools[choice].riot += random_between(g, 0, 8);
    if (schools[choice].riot > 100) schools[choice].riot = 100;
    update_prices(g, &schools[choice]);
    if (apply_riot_market(g, &schools[choice])) {
        show_market_panic_art();
        puts(RED "\nThe campus is on the edge of a riot—drug prices have gone through the roof!" RESET);
        if (schools[choice].riot < 100) wait_for_enter();
    } else if (schools[choice].riot >= 90) {
        puts(GREEN "\nA crowd of rowdy kids is beginning to gather!!" RESET);
        wait_for_enter();
    }
    if (schools[choice].riot >= 100 && schools[choice].control < 100) {
        puts(RED "The campus crowd explodes into a full riot the moment you arrive!" RESET);
        riot(g, schools);
    } else {
        random_travel_event(g, &schools[choice]);
    }
}

typedef struct {
    const char *name;
    int hp;
    int max_hp;
    int guard;
    char action;
} Fighter;

static void hp_bar_string(char bar[9], int hp, int max_hp) {
    int i, filled = max_hp > 0 ? hp * 6 / max_hp : 0;
    if (filled < 0) filled = 0;
    if (filled > 6) filled = 6;
    bar[0] = '[';
    for (i = 0; i < 6; ++i) bar[i + 1] = i < filled ? '#' : '.';
    bar[7] = ']';
    bar[8] = '\0';
}

static void draw_enemy_art(void) {
    event_art_row(RED,     "       ▄████▄              ▄████████▄              ▄████▄", 78);
    event_art_row(YELLOW,  "      █ ▄  ▄ █            █ ▄      ▄ █            █ ▄  ▄ █", 78);
    event_art_row(WHITE,   "      █ ▀  ▀ █            █ ▀      ▀ █            █ ▀  ▀ █", 78);
    event_art_row(RED,     "       ▀█▄▄█▀              ▀█▄ ▀▀ ▄█▀              ▀█▄▄█▀", 78);
    event_art_row(MAGENTA, "      ▄██████▄            ▄██████████▄            ▄██████▄", 78);
    event_art_row(BLUE,    "     ██ ████ ██          ██  ██████  ██          ██ ████ ██", 78);
    event_art_row(CYAN,    "    ██  ████  ██══╦      ██▄██ ●  ██▄██      ╦══██  ████  ██", 78);
    event_art_row(WHITE,   "       ▄█  █▄              ▄██    ██▄              ▄█  █▄", 78);
}

static void draw_party_backs(void) {
    event_art_row(BLUE,    "          ▄██▄                 ▄██▄                 ▄██▄", 78);
    event_art_row(CYAN,    "         ██████               ██████               ██████", 78);
    event_art_row(MAGENTA, "           ██                   ██                   ██", 78);
    event_art_row(BLUE,    "        ▄██████▄             ▄██████▄             ▄██████▄", 78);
    event_art_row(CYAN,    "       ██  ██  ██           ██  ██  ██           ██  ██  ██", 78);
}

static void gallery_caption(int slide, int total, const char *title,
                            const char *description) {
    printf(YELLOW "\n  TEST %02d/%02d — %s\n" RESET, slide, total, title);
    printf(CYAN "  %s\n" RESET, description);
    puts(GREEN "  PREVIEW ONLY — game state and random sequence are unchanged." RESET);
    wait_for_enter();
}

static void show_event_gallery(const Game *g, const School schools[SCHOOL_COUNT]) {
    Game before = *g;
    School school_before[SCHOOL_COUNT];
    int unchanged;
    memcpy(school_before, schools, sizeof(school_before));

    clear_screen();
    show_rainy_gun_dealer();
    puts(WHITE "\n  AK GUY: Two AK-47s for $7000.  (Y)es / (N)o" RESET);
    gallery_caption(1, 9, "AK-47 PURCHASE", "Duffel-bag dealer and purchase offer.");

    clear_screen();
    show_coat_only_raster();
    puts(WHITE "\n  COAT DEALER: +12 hold spaces for $2100.  (Y)es / (N)o" RESET);
    gallery_caption(2, 9, "TRENCH-COAT DEALER", "Permanent cargo upgrade offer.");

    show_market_panic_art();
    puts(RED "\n  Riot pressure breaks supply. Every drug price shoots upward." RESET);
    gallery_caption(3, 9, "MARKET PANIC", "High riot points create a temporary price explosion.");

    show_riot_crowd_art();
    puts(MAGENTA "\n  You organize the crowd and build toward 60 riot points." RESET);
    gallery_caption(4, 9, "RIOT RISING", "The organizing stage before riot combat.");

    clear_screen();
    event_border(RED, "RIOT COMBAT", 1);
    draw_enemy_art();
    event_row(WHITE, "──────────────────────────────────────────────────────────────────");
    draw_party_backs();
    event_row(GREEN, "YOU [######] 100/100    ACE [######] 55/55    NYX [######] 45/45");
    event_row(YELLOW, "FIGHT:  (A)ttack  (G)un  (D)efend  (R)ally  (E)scape");
    event_border(RED, "", 0);
    gallery_caption(5, 9, "RIOT BATTLE", "Turn-based crew combat; nobody takes damage in Test mode.");

    show_street_attack_art();
    puts(RED "\n  Three hostiles box you in near campus." RESET);
    gallery_caption(6, 9, "STREET ATTACK", "Random travel ambush; weapons and status normally decide it.");

    show_mugging_art();
    puts(WHITE "\n  Give up the cash—or burn 20 Status to break free." RESET);
    gallery_caption(7, 9, "ARMED MUGGING", "A high-cash random travel event.");

    show_lay_art();
    puts(GREEN "\n  Successful night: one condom used and one Lay recorded." RESET);
    gallery_caption(8, 9, "GETTING LAID", "The successful protected encounter scene.");

    show_boyfriend_art();
    puts(RED "\n  Her jealous adult boyfriend storms in looking for a fight!" RESET);
    gallery_caption(9, 9, "BOYFRIEND ATTACK", "The surprise fight that can follow a successful lay.");

    unchanged = memcmp(&before, g, sizeof(before)) == 0 &&
                memcmp(school_before, schools, sizeof(school_before)) == 0;
    clear_screen();
    event_border(MAGENTA, "EVENT TEST COMPLETE", 1);
    event_row(WHITE, "AK • COAT • PANIC • RIOT • COMBAT • ATTACK • MUGGING • LAY • BOYFRIEND");
    event_border(MAGENTA, "", 0);
    puts(unchanged ? GREEN "Verified: no cash, inventory, health, day, campus, or RNG changes."
                   RESET
                   : RED "Warning: gallery state changed; restoring on return." RESET);
    wait_for_enter();
}

static void draw_battle_screen(const School *school, const Fighter party[3],
                               int enemy_hp, int enemy_max, int round) {
    char title[96], enemy[96], crew[160];
    char enemy_bar[9], you_bar[9], ace_bar[9], nyx_bar[9];
    clear_screen();
    snprintf(title, sizeof(title), "RIOT BATTLE — %s — ROUND %02d", school->name, round);
    event_border(RED, title, 1);
    draw_enemy_art();
    hp_bar_string(enemy_bar, enemy_hp, enemy_max);
    snprintf(enemy, sizeof(enemy), "CAMPUS ENFORCERS  %s %3d/%-3d", enemy_bar, enemy_hp, enemy_max);
    event_row(RED, enemy);
    event_row(WHITE, "──────────────────────────────────────────────────────────────────────────────");
    draw_party_backs();
    hp_bar_string(you_bar, party[0].hp, party[0].max_hp);
    hp_bar_string(ace_bar, party[1].hp, party[1].max_hp);
    hp_bar_string(nyx_bar, party[2].hp, party[2].max_hp);
    snprintf(crew, sizeof(crew),
             "YOU %s %d/%d    ACE %s %d/%d    NYX %s %d/%d",
             you_bar, party[0].hp, party[0].max_hp,
             ace_bar, party[1].hp, party[1].max_hp,
             nyx_bar, party[2].hp, party[2].max_hp);
    event_row(GREEN, crew);
    event_row(YELLOW, "FIGHT:  (A)ttack  (G)un  (D)efend  (R)ally  (E)scape");
    event_border(RED, "", 0);
}

static char choose_fighter_action(const Fighter *fighter, int can_escape) {
    char action;
    for (;;) {
        printf(YELLOW "%-4s: (A)ttack (G)un (D)efend (R)ally%s " RESET,
               fighter->name, can_escape ? " (E)scape" : "");
        action = read_command("");
        if (action == 'a' || action == 'g' || action == 'd' || action == 'r' ||
            (can_escape && action == 'e')) return action;
        puts(RED "Choose A, G, D, R, or E." RESET);
    }
}

static int combat(Game *g, School *school) {
    Fighter party[3];
    int i, rounds = 0, escaped = 0;
    int original_security = school->security;
    int weapon_strength = 0;
    int crowd_support = school->riot / 5;
    int enemy_hp = 55 + original_security * 2 - crowd_support;
    int enemy_max;
    party[0] = (Fighter){"YOU", g->health, g->max_health, 0, 'a'};
    party[1] = (Fighter){"ACE", 55 + g->status / 2, 55 + g->status / 2, 0, 'g'};
    party[2] = (Fighter){"NYX", 45 + g->riots_attempted * 2,
                         45 + g->riots_attempted * 2, 0, 'a'};
    if (enemy_hp < 40) enemy_hp = 40;
    enemy_max = enemy_hp;
    for (i = 0; i < GUN_COUNT; ++i) weapon_strength += g->guns[i] * gun_power[i];

    while (party[0].hp > 0 && enemy_hp > 0 && rounds < 12) {
        int enemy_actions = 1 + original_security / 40;
        int rally_bonus = 0;
        ++rounds;
        draw_battle_screen(school, party, enemy_hp, enemy_max, rounds);
        printf(CYAN "Security %d  Crowd %d  Weapon power %d  Status %d\n" RESET,
               original_security, crowd_support, weapon_strength, g->status);
        for (i = 0; i < 3; ++i) {
            party[i].guard = 0;
            if (party[i].hp > 0) party[i].action = choose_fighter_action(&party[i], i == 0);
        }
        puts(WHITE "\n── YOUR CREW MOVES ──" RESET);
        for (i = 0; i < 3 && enemy_hp > 0; ++i) {
            int damage = 0;
            if (party[i].hp <= 0) continue;
            if (party[i].action == 'e') {
                int chance = 25 + g->status + weapon_strength / 8;
                if (chance > 85) chance = 85;
                if (random_between(g, 1, 100) <= chance) {
                    puts(GREEN "YOU signal the retreat. The crew breaks free!" RESET);
                    escaped = 1;
                    break;
                }
                puts(RED "The enforcers cut off your escape!" RESET);
            } else if (party[i].action == 'd') {
                party[i].guard = 1;
                printf(CYAN "%s braces behind cover.\n" RESET, party[i].name);
            } else if (party[i].action == 'r') {
                if (g->status >= 3) {
                    g->status -= 3;
                    rally_bonus += 5 + crowd_support / 2;
                    damage = 5 + crowd_support + random_between(g, 1, 8);
                    printf(GREEN "%s rallies the crowd! Status -3.\n" RESET,
                           party[i].name);
                } else {
                    damage = random_between(g, 2, 6);
                    printf(RED "%s cannot rally—Status is too low.\n" RESET,
                           party[i].name);
                }
            } else if (party[i].action == 'g') {
                damage = 5 + weapon_strength / 7 + random_between(g, 3, 12);
                printf(MAGENTA "%s fires into the opposition line!\n" RESET,
                       party[i].name);
            } else {
                damage = 7 + g->status / 5 + random_between(g, 4, 14);
                printf(YELLOW "%s charges forward!\n" RESET, party[i].name);
            }
            damage += rally_bonus;
            enemy_hp -= damage;
            if (enemy_hp < 0) enemy_hp = 0;
            if (damage > 0) printf(GREEN "  Enforcers take %d damage.\n" RESET, damage);
        }
        if (escaped) break;
        if (enemy_hp > 0) {
            puts(WHITE "── ENEMY TURN ──" RESET);
            for (i = 0; i < enemy_actions && party[0].hp > 0; ++i) {
                int target, tries = 0;
                int damage = 5 + original_security / 14 + random_between(g, 2, 10);
                do {
                    target = random_between(g, 0, 2);
                    ++tries;
                } while (party[target].hp <= 0 && tries < 10);
                if (party[target].hp <= 0) target = 0;
                if (party[target].guard) damage = (damage + 1) / 2;
                party[target].hp -= damage;
                if (party[target].hp < 0) party[target].hp = 0;
                printf(RED "Gunfire hits %s for %d!%s\n" RESET, party[target].name,
                       damage, party[target].hp == 0 ? "  DOWN!" : "");
            }
        }
        g->health = party[0].hp;
        wait_for_enter();
    }
    if (escaped) return -1;
    if (enemy_hp <= 0) {
        school->control = 100;
        school->riot = 0;
        school->security = 0;
        school->academics = 0;
        school->anarchy_sell_turns = 2;
        update_prices(g, school);
        g->status += 10 + original_security / 5;
        g->cash += original_security * 25;
        ++g->fights_won;
        puts(GREEN "You WIN!! The university falls into ANARCHY." RESET);
        printf(CYAN "Security collapses from %d to 0 and academics plummet to 0.\n" RESET,
               original_security);
        puts(MAGENTA "Girls now cost $100-$300 here, and drug sales pay double for 2 turns." RESET);
        return 1;
    }
    ++g->fights_lost;
    if (party[0].hp <= 0)
        puts(RED "The opposition overwhelms you. You go down in the riot." RESET);
    else
        puts(RED "The riot stalls after twelve brutal rounds. You retreat." RESET);
    return 0;
}

static void riot(Game *g, School schools[SCHOOL_COUNT]) {
    School *s = &schools[g->location];
    if (s->control >= 100) {
        puts("You've taken over that university already.");
        wait_for_enter();
        return;
    }
    if (s->riot < 60) {
        int gain = random_between(g, 10, 20) + g->status / 20;
        ++g->riots_attempted;
        ++g->day;
        s->riot += gain;
        if (s->riot > 100) s->riot = 100;
        show_riot_crowd_art();
        printf(MAGENTA "You organize a campus protest. Riot points +%d (now %d%%).\n" RESET,
               gain, s->riot);
        puts(YELLOW "The riot action spends one campaign day." RESET);
        if (s->riot >= 60)
            puts(YELLOW "The crowd is ready. Choose (R)iot again to begin combat." RESET);
        printf(YELLOW "Riot actions: %d. Future street-fight risk has increased.\n" RESET,
               g->riots_attempted);
        wait_for_enter();
        return;
    }
    ++g->riots_attempted;
    ++g->day;
    puts(YELLOW "The riot action spends one campaign day." RESET);
    printf("Time to riot and take over %s!!\n", s->name);
    if (combat(g, s) == 1) ++g->riots_won;
    printf(YELLOW "Riots attempted: %d. Future street-fight risk has increased.\n" RESET,
           g->riots_attempted);
    wait_for_enter();
}

static void show_schools(const School schools[SCHOOL_COUNT]) {
    int i;
    clear_screen();
    printf(YELLOW "  %-28s %-8s %8s %8s %10s %7s\n",
           "Campuses", "Control", "Academics", "Riot %", "Security", "Lays");
    fputs(MAGENTA, stdout);
    for (i = 0; i < 78; ++i) putchar("-=="[i % 3]);
    puts(RESET);
    for (i = 0; i < SCHOOL_COUNT - 1; ++i)
        printf(CYAN "  %-28s %-8s %8d %8d %10d %7d\n", schools[i].name,
               schools[i].control ? "ANARCHY" : "ORDER", schools[i].academics,
               schools[i].riot, schools[i].security, schools[i].lays);
    fputs(MAGENTA, stdout);
    for (i = 0; i < 78; ++i) putchar("-=="[i % 3]);
    puts(RESET);
    wait_for_enter();
}

static int controlled_count(const School schools[SCHOOL_COUNT]) {
    int i, count = 0;
    for (i = 0; i < SCHOOL_COUNT - 1; ++i) if (schools[i].control >= 100) ++count;
    return count;
}

static long impact_score(const Game *g, const School schools[SCHOOL_COUNT]) {
    double wealth = g->cash + g->bank - g->debt;
    long score;
    if (wealth < 0) wealth = 0;
    score = controlled_count(schools) * 10000L;
    score += g->riots_won * 1000L;
    score += g->lays * 400L;
    score += g->status * 50L;
    score += g->fights_won * 300L;
    score -= g->fights_lost * 250L;
    score += (g->health > 0 ? g->health : 0) * 20L;
    score += (long)(sqrt(wealth) * 20.0);
    return score > 0 ? score : 0;
}

static long reputation_score(const Game *g, const School schools[SCHOOL_COUNT]) {
    double cash = g->cash > 0 ? g->cash : 0;
    long cash_bonus = (long)(sqrt(cash) * 10.0);
    if (cash_bonus > 3000) cash_bonus = 3000;
    return controlled_count(schools) * 1000L + g->lays * 125L + cash_bonus;
}

static const char *impact_rank(long reputation) {
    if (reputation >= 18000) return "TOTAL ANARCHY";
    if (reputation >= 15000) return "BIG TEN BOGEYMAN";
    if (reputation >= 12000) return "CONFERENCE WARLORD";
    if (reputation >= 9000) return "CAMPUS CRIME LORD";
    if (reputation >= 6000) return "REGIONAL MENACE";
    if (reputation >= 3000) return "QUAD-RULING OUTLAW";
    if (reputation >= 1000) return "CAMPUS TROUBLEMAKER";
    return "BARELY A RUMOR";
}

static void show_day_60_finale(void) {
    clear_screen();
    event_border(RED, "DAY 60 — THE LAST NIGHT", 1);
    event_row(BLUE,    "│█│█│█│█│█│█│█│█│█│█│█│█│█│█│█│█│█│█│");
    event_row(YELLOW,  "✦       \\|/                         \\|/       ✦");
    event_row(WHITE,   "|        |        CAMPUS SECURITY        |        |");
    event_row(BLUE,    "▄██▄        ▄██▄        ▄██▄        ▄██▄        ▄██▄");
    event_row(WHITE,   "█▀▀█        █▀▀█        █▀▀█        █▀▀█        █▀▀█");
    event_row(CYAN,    "▄████▄      ▄████▄      ▄████▄      ▄████▄      ▄████▄");
    event_row(RED,     "╱│╲          ╱│╲          ╱│╲          ╱│╲          ╱│╲");
    event_row(MAGENTA, "·  \\O/  ·  \\O/  ·  \\O/  ·  \\O/  ·  \\O/  ·  \\O/  ·");
    event_row(MAGENTA, "\\O/  |  \\O/  |  \\O/  |  \\O/  |  \\O/  |  \\O/  |  \\O/");
    event_row(YELLOW,  "THE CLOCK HITS MIDNIGHT. THE CAMPUSES REMEMBER YOUR NAME.");
    event_border(RED, "", 0);
    puts(RED "\n  Sixty days are gone. Order and anarchy face each other at dawn." RESET);
    puts(CYAN "  Your campuses, lays, and cash in hand now decide the legend." RESET);
    wait_for_enter();
}

static void show_final_impact(const Game *g, const School schools[SCHOOL_COUNT], int quit) {
    int taken = controlled_count(schools);
    double wealth = g->cash + g->bank - g->debt;
    long score = impact_score(g, schools);
    long reputation = reputation_score(g, schools);
    long cash_bonus = reputation - taken * 1000L - g->lays * 125L;
    if (g->health > 0 && g->day > DAY_LIMIT && !quit) show_day_60_finale();
    clear_screen();
    title_bar();
    puts(MAGENTA "\n===================== FINAL IMPACT REPORT =====================" RESET);
    if (g->health <= 0) puts(RED "You did not survive the campaign." RESET);
    else if (g->day > DAY_LIMIT) puts(YELLOW "The 60-day campaign is over." RESET);
    else if (quit) puts(YELLOW "You ended the campaign early." RESET);
    printf(CYAN "Campuses in anarchy: " WHITE "%d of %d\n", taken, SCHOOL_COUNT - 1);
    printf(CYAN "Riots              : " WHITE "%d won / %d attempted\n",
           g->riots_won, g->riots_attempted);
    printf(CYAN "Street fights      : " WHITE "%d won / %d lost\n",
           g->fights_won, g->fights_lost);
    printf(CYAN "Lays               : " WHITE "%d\n", g->lays);
    printf(CYAN "Cash in hand       : " WHITE "$%.0f\n", g->cash);
    printf(CYAN "Status             : " WHITE "%d\n", g->status);
    printf(CYAN "Health remaining   : " WHITE "%d\n", g->health > 0 ? g->health : 0);
    printf(CYAN "Net worth          : " WHITE "$%.0f\n", wealth);
    puts(MAGENTA "---------------------------------------------------------------" RESET);
    printf(YELLOW "IMPACT SCORE: %ld\n", score);
    printf(YELLOW "LEGEND SCORE: %ld  " WHITE "(schools %d + lays %d + cash %ld)\n",
           reputation, taken * 1000, g->lays * 125, cash_bonus);
    printf(GREEN "FINAL RANK:   %s\n" RESET, impact_rank(reputation));
    puts(WHITE "Each campus in anarchy is worth 1,000 Legend points. Every lay" RESET);
    puts(WHITE "adds 125, and cash in hand adds up to a 3,000-point bonus." RESET);
}

static int self_test(void) {
    Game g;
    School s[SCHOOL_COUNT];
    int i;
    initialize_schools(s);
    new_game(&g, 1, s);
    if (strcmp(guns[0], ".22 Liberator") != 0 || g.cash != 500 || g.bank != 250 ||
        g.guns[0] != 1 || g.health != 100 ||
        g.hold_max != 10 || g.condoms != 1 || g.status != 1) return 1;
    g.price[0] = 100; g.cash = 500; g.hold[0] = 2;
    g.hold[0] -= 2; g.cash += 200;
    if (g.hold[0] != 0 || g.cash != 700) return 2;
    g.cash = 1000; g.bank = 250; g.cash -= 200; g.bank += 200;
    if (g.cash != 800 || g.bank != 450) return 3;
    new_game(&g, 123, s);
    g.cash = 1000000;
    g.hold_max = DRUG_COUNT;
    for (i = 0; i < DRUG_COUNT; ++i) {
        g.price[i] = 100 + i;
        if (purchase_drug(&g, i, 1) != 1 || g.hold[i] != 1) return 10 + i;
    }
    if (used_hold(&g) != DRUG_COUNT || purchase_drug(&g, 5, 1) != -1) return 20;
    g.hold_max = DRUG_COUNT + 1;
    g.cash = 0;
    if (purchase_drug(&g, 5, 1) != -2) return 21;
    if (affordable_units(500, 100) != 5 || affordable_units(99, 100) != 0 ||
        affordable_units(500, 0) != 0) return 37;
    if (hospital_price(25) != 100 || hospital_price(0) != 0 || owned_guns(&g) != 1)
        return 38;
    if (normalize_key('A') != 'a' || normalize_key('a') != 'a' ||
        normalize_key('G') != 'g') return 39;
    s[0].control = 100; s[0].security = 0; s[0].academics = 0;
    update_prices(&g, &s[0]);
    if (g.girl_cost < 100 || g.girl_cost > 300 || g.girls_available < 5 ||
        g.girls_available > 9) return 40;
    g.cash = 10000;
    if (purchase_guns(&g, 6, 2, 7000) != 1 || g.guns[6] != 2 || g.cash != 3000)
        return 22;
    s[0].riot = 96;
    for (i = 0; i < DRUG_COUNT; ++i) g.price[i] = 100;
    if (!apply_riot_market(&g, &s[0])) return 23;
    for (i = 0; i < DRUG_COUNT; ++i) if (g.price[i] < 400) return 24;
    g.riots_won = 2; g.riots_attempted = 3; g.lays = 4; g.fights_won = 2;
    s[0].control = 100;
    if (impact_score(&g, s) < 10000) return 25;
    if (travel_cost(14, 15) != 5) return 26;
    for (i = 0; i < SCHOOL_COUNT - 1; ++i) {
        int j;
        for (j = 0; j < SCHOOL_COUNT - 1; ++j) {
            int fare = travel_cost(i, j);
            if (fare < 5 || fare > 25 || fare != travel_cost(j, i)) return 27;
        }
    }
    if (security_market_factor(90) >= security_market_factor(10)) return 28;
    g.condoms = 0; g.girls_available = 1; g.cash = 1000; g.girl_cost = 500;
    if (purchase_lay(&g, &s[0]) != -1 || g.lays != 4) return 29;
    g.condoms = 1;
    if (purchase_lay(&g, &s[0]) != 1 || g.condoms != 0 || g.lays != 5) return 30;
    initialize_schools(s);
    new_game(&g, 1, s);
    g.cash = 0; g.lays = 0;
    if (reputation_score(&g, s) != 0 || strcmp(impact_rank(0), "BARELY A RUMOR") != 0)
        return 31;
    s[0].control = 100; g.lays = 8; g.cash = 10000;
    if (reputation_score(&g, s) != 3000 ||
        strcmp(impact_rank(reputation_score(&g, s)), "QUAD-RULING OUTLAW") != 0)
        return 32;
    for (i = 0; i < SCHOOL_COUNT - 1; ++i) s[i].control = 100;
    if (strcmp(impact_rank(reputation_score(&g, s)), "TOTAL ANARCHY") != 0)
        return 33;
    new_game(&g, 1, s);
    g.debt = 500;
    g.loan_visits_remaining = LOAN_GRACE_VISITS;
    for (i = 0; i < LOAN_GRACE_VISITS; ++i) {
        if (apply_loan_visit(&g) != 0 || g.health != 100) return 34;
    }
    if (g.loan_visits_remaining != 0 ||
        apply_loan_visit(&g) != LOAN_OVERDUE_DAMAGE ||
        g.health != 100 - LOAN_OVERDUE_DAMAGE) return 35;
    g.debt = 0;
    if (apply_loan_visit(&g) != 0 || g.loan_visits_remaining != 0) return 36;
    puts("self-test: PASS");
    return 0;
}

int main(int argc, char **argv) {
    Game game;
    School schools[SCHOOL_COUNT];
    int quit = 0;
    uint32_t seed = (uint32_t)time(NULL);
    if (argc > 1 && strcmp(argv[1], "--self-test") == 0) return self_test();
    if (argc > 2 && strcmp(argv[1], "--seed") == 0) seed = (uint32_t)strtoul(argv[2], NULL, 10);
    enable_dos_console();
    initialize_schools(schools);
    new_game(&game, seed, schools);
    show_splash();
    while (!quit && game.day <= DAY_LIMIT && game.health > 0) {
        char command;
        int bonus_location = game.location;
        int bonus_before = schools[bonus_location].anarchy_sell_turns;
        const char *street_message = "";
        draw_dashboard(&game, schools);
        if (schools[game.location].riot >= 96)
            street_message = "MARKET PANIC: Drug prices have gone through the roof!";
        else if (schools[game.location].riot >= 90)
            street_message = "A crowd of rowdy kids is beginning to gather!!";
        else if (game.health <= 30)
            street_message = "You are badly hurt. The hospital may be a smart stop.";
        show_main_menu(street_message);
        command = read_command(WHITE "                        Command: " RESET);
        switch (command) {
            case 'b': buy(&game, schools); break;
            case 's': sell(&game, schools); break;
            case 'v': bank(&game); break;
            case 'h': hospital(&game); break;
            case 'l': loan_shark(&game); break;
            case 'd': drive(&game, schools); break;
            case 'r': riot(&game, schools); break;
            case 'c': show_schools(schools); break;
            case 't': show_event_gallery(&game, schools); break;
            case 'q': quit = 1; break;
            default: break;
        }
        if (bonus_before > 0 && schools[bonus_location].anarchy_sell_turns == bonus_before)
            --schools[bonus_location].anarchy_sell_turns;
        if (controlled_count(schools) == SCHOOL_COUNT - 1) {
            puts("You have thrown the entire Big Ten into anarchy. You WIN!!");
            break;
        }
    }
    show_final_impact(&game, schools, quit);
    return 0;
}
