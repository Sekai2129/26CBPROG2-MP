#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// ─────────────────────────────────────────────
//  DASHBOARD.H  —  Part 2: SDG Dashboard
//  Functions: display, filter, search
//  Data source: sdg_provinces.txt
// ─────────────────────────────────────────────

#define MAX_PROVINCES   84
#define MAX_FIELDS      19
#define MAX_LEN         64
#define DATA_FILE       "sdg_provinces.txt"
#define BAR_WIDTH       40      // max bar chart width in characters

// ── SDG metadata ─────────────────────────────
#define SDG_COUNT 16  // SDG14 excluded

typedef struct {
    int    number;
    char   code[8];
    char   name[64];
    char   indicator[64];
    char   unit[32];
    int    higher_is_worse; // 1 = high value = bad (e.g. poverty), 0 = low value = bad
} SDGInfo;

static const SDGInfo SDG_LIST[SDG_COUNT] = {
    { 1,  "SDG1",  "No Poverty",
      "Poverty Incidence",               "%",       1 },
    { 2,  "SDG2",  "Zero Hunger",
      "Food Insecurity Rate",            "%",       1 },
    { 3,  "SDG3",  "Good Health & Well-Being",
      "Vulnerable Health Status Score",  "score",   1 },
    { 4,  "SDG4",  "Quality Education",
      "Literacy Rate",                   "%",       0 },
    { 5,  "SDG5",  "Gender Equality",
      "Gender Inequality Score",         "score",   1 },
    { 6,  "SDG6",  "Clean Water & Sanitation",
      "Clean Water Vulnerability Score", "score",   1 },
    { 7,  "SDG7",  "Affordable & Clean Energy",
      "Energy Capacity Score",           "score",   0 },
    { 8,  "SDG8",  "Decent Work & Economic Growth",
      "Labor Force Participation Rate",  "%",       0 },
    { 9,  "SDG9",  "Industry, Innovation & Infrastructure",
      "Infrastructure Capacity Score",   "score",   0 },
    { 10, "SDG10", "Reduced Inequalities",
      "Economic Constraints Score",      "score",   1 },
    { 11, "SDG11", "Sustainable Cities",
      "Informal Settlers Rate",          "%",       1 },
    { 12, "SDG12", "Responsible Consumption",
      "Garbage Pickup Rate",             "%",       0 },
    { 13, "SDG13", "Climate Action",
      "Multi-Hazard Exposure Score",     "score",   1 },
    { 15, "SDG15", "Life on Land",
      "Environmental Stress Score",      "score",   1 },
    { 16, "SDG16", "Peace, Justice & Strong Institutions",
      "Organized Violence Rate",         "per 100k",1 },
    { 17, "SDG17", "Partnerships for the Goals",
      "Internet Access Rate",            "%",       0 },
};

// ── Province struct ───────────────────────────
typedef struct {
    char fields[MAX_FIELDS][MAX_LEN];
} Province;

static Province provinces[MAX_PROVINCES];
static int      province_count = 0;

// ─────────────────────────────────────────────
//  UTILITY FUNCTIONS
// ─────────────────────────────────────────────

// Clears terminal screen cross-platform
void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Pause until user presses Enter
void pause_prompt(void) {
    printf("\n  Press Enter to continue...");
    getchar();
}

// Strip trailing newline/space from string
void trim(char *s) {
    int len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' || s[len-1] == ' '))
        s[--len] = '\0';
}

// Convert a field string to double; returns -1.0 if N/A or EXCL.
double parse_value(const char *s) {
    if (strcmp(s, "N/A") == 0 || strcmp(s, "EXCL.") == 0 || strlen(s) == 0)
        return -1.0;
    return atof(s);
}

// Lowercase string for case-insensitive search
void to_lower(char *dest, const char *src, int maxlen) {
    int i;
    for (i = 0; i < maxlen - 1 && src[i]; i++)
        dest[i] = (src[i] >= 'A' && src[i] <= 'Z') ? src[i] + 32 : src[i];
    dest[i] = '\0';
}

// ─────────────────────────────────────────────
//  LOAD DATA FROM TXT FILE
// ─────────────────────────────────────────────
int load_provinces(void) {
    FILE *f = fopen(DATA_FILE, "r");
    if (!f) {
        printf("\n  ERROR: Cannot open '%s'.\n", DATA_FILE);
        printf("  Make sure the file is in the same folder as the program.\n\n");
        return 0;
    }

    char line[1024];
    province_count = 0;

    while (fgets(line, sizeof(line), f)) {
        trim(line);
        // Skip comment lines and blank lines
        if (line[0] == '#' || line[0] == '\0') continue;
        if (province_count >= MAX_PROVINCES) break;

        Province p;
        int col = 0;
        char *token = strtok(line, "|");
        while (token && col < MAX_FIELDS) {
            strncpy(p.fields[col], token, MAX_LEN - 1);
            p.fields[col][MAX_LEN - 1] = '\0';
            col++;
            token = strtok(NULL, "|");
        }
        // Fill remaining fields as N/A if line was short
        while (col < MAX_FIELDS) {
            strcpy(p.fields[col++], "N/A");
        }
        provinces[province_count++] = p;
    }

    fclose(f);
    return province_count;
}

// ─────────────────────────────────────────────
//  BAR CHART RENDERER
// ─────────────────────────────────────────────

// Finds min and max of a column across all loaded provinces (skipping N/A)
void get_col_range(int col, double *out_min, double *out_max) {
    *out_min =  1e9;
    *out_max = -1e9;
    for (int i = 0; i < province_count; i++) {
        double v = parse_value(provinces[i].fields[col]);
        if (v < 0) continue;
        if (v < *out_min) *out_min = v;
        if (v > *out_max) *out_max = v;
    }
    if (*out_min > *out_max) { *out_min = 0; *out_max = 1; }
}

// Finds min and max of a column for a specific list of provinces
void get_col_range_subset(int col, int *indices, int count,
                          double *out_min, double *out_max) {
    *out_min =  1e9;
    *out_max = -1e9;
    for (int i = 0; i < count; i++) {
        double v = parse_value(provinces[indices[i]].fields[col]);
        if (v < 0) continue;
        if (v < *out_min) *out_min = v;
        if (v > *out_max) *out_max = v;
    }
    if (*out_min > *out_max) { *out_min = 0; *out_max = 1; }
}

// Prints a single horizontal bar for a province
void print_bar(const char *province, const char *region,
               double value, double min_val, double max_val,
               const char *unit, int higher_is_worse) {
    if (value < 0) {
        printf("  %-25s %-6s  [  N/A  ]\n", province, region);
        return;
    }

    // Normalize to bar width
    int bar_len = (max_val == min_val) ? BAR_WIDTH :
        (int)((value - min_val) / (max_val - min_val) * BAR_WIDTH);
    if (bar_len < 1)  bar_len = 1;
    if (bar_len > BAR_WIDTH) bar_len = BAR_WIDTH;

    // Choose bar character based on severity
    char bar_char = higher_is_worse ?
        (bar_len > BAR_WIDTH * 0.66 ? '#' :
         bar_len > BAR_WIDTH * 0.33 ? '=' : '-') :
        (bar_len > BAR_WIDTH * 0.66 ? '#' :
         bar_len > BAR_WIDTH * 0.33 ? '=' : '-');

    printf("  %-25s %-6s |", province, region);
    for (int i = 0; i < bar_len; i++) putchar(bar_char);
    for (int i = bar_len; i < BAR_WIDTH; i++) putchar(' ');
    printf("| %6.2f %s\n", value, unit);
}

// Prints a full bar chart for one SDG across given province indices
void print_bar_chart(const SDGInfo *sdg, int col,
                     int *indices, int count) {
    double min_val, max_val;
    get_col_range_subset(col, indices, count, &min_val, &max_val);

    printf("\n");
    printf("  ┌─────────────────────────────────────────────────────────────────────┐\n");
    printf("  │  %s — %s                 \n", sdg->code, sdg->name);
    printf("  │  Indicator : %s (%s)\n", sdg->indicator, sdg->unit);
    if (sdg->higher_is_worse)
        printf("  │  Note      : Higher value = more at-risk\n");
    else
        printf("  │  Note      : Higher value = better performance\n");
    printf("  ├──────────────────────────────────────────────────────────────────────\n");
    printf("  │  %-25s %-6s   %-*s   Value\n",
           "Province", "Region", BAR_WIDTH + 2, "Bar Chart");
    printf("  ├──────────────────────────────────────────────────────────────────────\n");

    for (int i = 0; i < count; i++) {
        Province *p = &provinces[indices[i]];
        double v = parse_value(p->fields[col]);
        print_bar(p->fields[0], p->fields[1],
                  v, min_val, max_val, sdg->unit, sdg->higher_is_worse);
    }

    printf("  └──────────────────────────────────────────────────────────────────────\n");
    printf("  Range: %.2f (min) — %.2f (max)\n", min_val, max_val);
}

// ─────────────────────────────────────────────
//  SDG INDEX LOOKUP
//  SDG numbers are 1-13,15,16,17 (14 excluded)
//  Returns index in SDG_LIST[], or -1 if invalid
// ─────────────────────────────────────────────
int sdg_number_to_index(int sdg_num) {
    for (int i = 0; i < SDG_COUNT; i++)
        if (SDG_LIST[i].number == sdg_num) return i;
    return -1;
}

// SDG field column in the province data array (col 2 = SDG1, col 3 = SDG2...)
// SDG14 is col 15 (EXCL.) — we skip it
int sdg_number_to_col(int sdg_num) {
    if (sdg_num >= 1 && sdg_num <= 13)  return sdg_num + 1;  // SDG1=col2, SDG2=col3...
    if (sdg_num == 15) return 16;
    if (sdg_num == 16) return 17;
    if (sdg_num == 17) return 18;
    return -1;
}

// ─────────────────────────────────────────────
//  WELCOME SCREEN
// ─────────────────────────────────────────────
void show_welcome(const char *username) {
    clear_screen();
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════════════════╗\n");
    printf("  ║                                                                      ║\n");
    printf("  ║         SDG PHILIPPINES REGIONAL DASHBOARD                          ║\n");
    printf("  ║         Sustainable Development Goals — Province-Level Data         ║\n");
    printf("  ║                                                                      ║\n");
    printf("  ╚══════════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("  Welcome, %s!\n\n", username);
    printf("  ABOUT THIS DASHBOARD\n");
    printf("  ─────────────────────────────────────────────────────────────────────\n");
    printf("  This dashboard presents provincial-level data for the Philippines\n");
    printf("  across 16 of the 17 United Nations Sustainable Development Goals\n");
    printf("  (SDG 14 — Life Below Water — is excluded due to insufficient data).\n");
    printf("\n");
    printf("  GOAL\n");
    printf("  ─────────────────────────────────────────────────────────────────────\n");
    printf("  To help users identify which provinces need the most attention for\n");
    printf("  each SDG, enabling more targeted programs, policies, and interventions\n");
    printf("  at the local government level.\n");
    printf("\n");
    printf("  DATA SOURCES\n");
    printf("  ─────────────────────────────────────────────────────────────────────\n");
    printf("  - PSA Provincial Poverty Statistics\n");
    printf("  - IPC Philippines Chronic Food Insecurity Analysis (2015-2020)\n");
    printf("  - NDPBA Province Risk Profiles 2021 (Pacific Disaster Center)\n");
    printf("\n");
    printf("  ─────────────────────────────────────────────────────────────────────\n");
    printf("  Would you like to proceed to the dashboard? (y/n): ");

    char ans;
    scanf(" %c", &ans);
    getchar();

    if (ans != 'y' && ans != 'Y') {
        printf("\n  Thank you for using the SDG Dashboard. Goodbye!\n\n");
        exit(0);
    }
}

// ─────────────────────────────────────────────
//  SDG GUIDE SCREEN
// ─────────────────────────────────────────────
void show_sdg_guide(void) {
    clear_screen();
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════════════════╗\n");
    printf("  ║  AVAILABLE SDGs — INDICATORS & DESCRIPTIONS                        ║\n");
    printf("  ╚══════════════════════════════════════════════════════════════════════╝\n\n");

    for (int i = 0; i < SDG_COUNT; i++) {
        const SDGInfo *s = &SDG_LIST[i];
        printf("  [%2d] %s — %s\n", s->number, s->code, s->name);
        printf("       Indicator : %s\n", s->indicator);
        printf("       Unit      : %s\n", s->unit);
        printf("       Direction : %s\n\n",
               s->higher_is_worse ? "Higher = more at-risk (worse)"
                                  : "Higher = better performance");
    }

    printf("  ─────────────────────────────────────────────────────────────────────\n");
    printf("  NOTE: SDG 14 (Life Below Water) is excluded — insufficient data.\n");
    printf("  ─────────────────────────────────────────────────────────────────────\n");
    pause_prompt();
}

// ─────────────────────────────────────────────
//  FUNCTION 1: DISPLAY ALL PROVINCES
//  Shows bar chart for selected SDGs,
//  all 84 provinces
// ─────────────────────────────────────────────
void display_all(int *sdg_nums, int sdg_count) {
    // Build index array for all provinces
    int indices[MAX_PROVINCES];
    for (int i = 0; i < province_count; i++) indices[i] = i;

    for (int s = 0; s < sdg_count; s++) {
        int sdg_idx = sdg_number_to_index(sdg_nums[s]);
        int col     = sdg_number_to_col(sdg_nums[s]);
        if (sdg_idx < 0 || col < 0) {
            printf("  [!] SDG %d is not available.\n", sdg_nums[s]);
            continue;
        }
        print_bar_chart(&SDG_LIST[sdg_idx], col, indices, province_count);
        if (s < sdg_count - 1) pause_prompt();
    }
}

// ─────────────────────────────────────────────
//  FUNCTION 2: FILTER BY REGION
//  Narrows display to provinces in a region
// ─────────────────────────────────────────────
void filter_by_region(int *sdg_nums, int sdg_count) {
    printf("\n  Available Regions:\n");
    printf("  NCR  | CAR | I  | II  | III | IV-A | IV-B\n");
    printf("  V    | VI  | VII| VIII| IX  | X    | XI\n");
    printf("  XII  | XIII| BARMM\n");
    printf("\n  Enter region code: ");

    char region[16];
    fgets(region, sizeof(region), stdin);
    region[strcspn(region, "\n")] = 0;

    // Build filtered index list
    int indices[MAX_PROVINCES];
    int count = 0;
    for (int i = 0; i < province_count; i++) {
        if (strcasecmp(provinces[i].fields[1], region) == 0)
            indices[count++] = i;
    }

    if (count == 0) {
        printf("\n  No provinces found for region '%s'.\n", region);
        pause_prompt();
        return;
    }

    printf("\n  Found %d province(s) in region %s.\n", count, region);

    for (int s = 0; s < sdg_count; s++) {
        int sdg_idx = sdg_number_to_index(sdg_nums[s]);
        int col     = sdg_number_to_col(sdg_nums[s]);
        if (sdg_idx < 0 || col < 0) {
            printf("  [!] SDG %d is not available.\n", sdg_nums[s]);
            continue;
        }
        print_bar_chart(&SDG_LIST[sdg_idx], col, indices, count);
        if (s < sdg_count - 1) pause_prompt();
    }
}

// ─────────────────────────────────────────────
//  FUNCTION 3: SEARCH BY PROVINCE NAME
//  Case-insensitive partial match search
// ─────────────────────────────────────────────
void search_province(int *sdg_nums, int sdg_count) {
    printf("\n  Enter province name (or part of it): ");
    char query[MAX_LEN];
    fgets(query, sizeof(query), stdin);
    query[strcspn(query, "\n")] = 0;

    char q_lower[MAX_LEN], p_lower[MAX_LEN];
    to_lower(q_lower, query, MAX_LEN);

    int indices[MAX_PROVINCES];
    int count = 0;

    for (int i = 0; i < province_count; i++) {
        to_lower(p_lower, provinces[i].fields[0], MAX_LEN);
        if (strstr(p_lower, q_lower)) {
            indices[count++] = i;
        }
    }

    if (count == 0) {
        printf("\n  No province found matching '%s'.\n", query);
        pause_prompt();
        return;
    }

    printf("\n  Found %d match(es) for '%s':\n", count, query);
    for (int i = 0; i < count; i++)
        printf("    - %s (%s)\n",
               provinces[indices[i]].fields[0],
               provinces[indices[i]].fields[1]);

    for (int s = 0; s < sdg_count; s++) {
        int sdg_idx = sdg_number_to_index(sdg_nums[s]);
        int col     = sdg_number_to_col(sdg_nums[s]);
        if (sdg_idx < 0 || col < 0) {
            printf("  [!] SDG %d is not available.\n", sdg_nums[s]);
            continue;
        }
        print_bar_chart(&SDG_LIST[sdg_idx], col, indices, count);
        if (s < sdg_count - 1) pause_prompt();
    }
}

// ─────────────────────────────────────────────
//  SDG SELECTOR
//  Prompts user to type SDG numbers (e.g. 1 3 8)
//  Returns count of selected SDGs
// ─────────────────────────────────────────────
int select_sdgs(int *out_sdg_nums) {
    printf("\n  ─────────────────────────────────────────────────────────────────────\n");
    printf("  SELECT SDGs (type numbers separated by spaces, e.g.: 1 3 8)\n");
    printf("  Available: 1 2 3 4 5 6 7 8 9 10 11 12 13 15 16 17\n");
    printf("  Enter 0 to cancel\n");
    printf("  ─────────────────────────────────────────────────────────────────────\n");
    printf("  SDGs: ");

    char line[256];
    fgets(line, sizeof(line), stdin);

    int count = 0;
    char *token = strtok(line, " \t\n");
    while (token && count < SDG_COUNT) {
        int num = atoi(token);
        if (num == 0) return 0;
        if (num == 14) {
            printf("  [!] SDG 14 is excluded. Skipping.\n");
        } else if (sdg_number_to_index(num) >= 0) {
            // Avoid duplicates
            int dup = 0;
            for (int i = 0; i < count; i++)
                if (out_sdg_nums[i] == num) { dup = 1; break; }
            if (!dup) out_sdg_nums[count++] = num;
        } else {
            printf("  [!] SDG %d not recognized. Skipping.\n", num);
        }
        token = strtok(NULL, " \t\n");
    }

    if (count == 0) {
        printf("  No valid SDGs selected.\n");
        return 0;
    }

    printf("  Selected SDGs: ");
    for (int i = 0; i < count; i++)
        printf("%d%s", out_sdg_nums[i], i < count - 1 ? ", " : "\n");

    return count;
}

// ─────────────────────────────────────────────
//  MAIN DASHBOARD MENU
// ─────────────────────────────────────────────
void dashboard_menu(const char *username) {
    // Load data first
    if (!load_provinces()) {
        pause_prompt();
        return;
    }

    // Welcome screen
    show_welcome(username);

    // SDG guide
    show_sdg_guide();

    // Main loop
    int choice;
    while (1) {
        clear_screen();
        printf("\n");
        printf("  ╔══════════════════════════════════════════════════════════════════════╗\n");
        printf("  ║  SDG PHILIPPINES DASHBOARD  —  Main Menu                           ║\n");
        printf("  ╚══════════════════════════════════════════════════════════════════════╝\n");
        printf("\n");
        printf("  [1] Display all provinces (bar chart by SDG)\n");
        printf("  [2] Filter by region\n");
        printf("  [3] Search by province name\n");
        printf("  [4] Show SDG guide again\n");
        printf("  [5] Log out\n");
        printf("\n  Choice: ");
        scanf("%d", &choice);
        getchar();

        int sdg_nums[SDG_COUNT];
        int sdg_count = 0;

        switch (choice) {
            case 1:
                sdg_count = select_sdgs(sdg_nums);
                if (sdg_count > 0) {
                    clear_screen();
                    display_all(sdg_nums, sdg_count);
                    pause_prompt();
                }
                break;

            case 2:
                sdg_count = select_sdgs(sdg_nums);
                if (sdg_count > 0) {
                    clear_screen();
                    filter_by_region(sdg_nums, sdg_count);
                }
                break;

            case 3:
                sdg_count = select_sdgs(sdg_nums);
                if (sdg_count > 0) {
                    clear_screen();
                    search_province(sdg_nums, sdg_count);
                }
                break;

            case 4:
                show_sdg_guide();
                break;

            case 5:
                printf("\n  Logged out. Returning to sign-in menu...\n");
                pause_prompt();
                return;

            default:
                printf("\n  Invalid choice. Try again.\n");
                pause_prompt();
        }
    }
}

#endif // DASHBOARD_H
