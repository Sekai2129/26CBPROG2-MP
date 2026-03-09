#include "auth.h"
#include "dashboard.h"

// ─────────────────────────────────────────────
//  MAIN.C
//  Entry point — connects Part 1 (auth) and
//  Part 2 (dashboard) together.
// ─────────────────────────────────────────────

int main(void) {
    char logged_in_user[50]; // matches Accounts.username[50] in auth.h

    // Loops back here after every logout
    while (1) {
        // Part 1 — auth (partner's code)
        auth_menu(logged_in_user);

        // Part 2 — dashboard (your code)
        dashboard_menu(logged_in_user);
    }

    return 0;
}
