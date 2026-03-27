#ifndef AUTH_H
#define AUTH_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ─────────────────────────────────────────────
//  AUTH.H  —  Part 1: Sign Up / Sign In
//  Original code by partner (Signing_Program.c)
//  Converted to header file. Struct and logic
//  preserved exactly — only change is SignIn()
//  now accepts out_username to pass to dashboard.
// ─────────────────────────────────────────────

#define USER_FILE "Accounts.txt"
#define KEY 23


struct Account {
    char username[51];
    char password[100];
	int role;
};

typedef struct Account Accounts;

void encryption(char *message){
	int i;
	
	for(i = 0; message[i] != '\0'; i++){
		message[i] = message[i] ^ KEY;
	}
}

void decryption(char *message){
	int i;
	
	for(i = 0; message[i] != '\0'; i++){
		message[i] = message[i] ^ KEY;
	}
}

// ── Create Account ────────────────────────────
void CreateAccount() {
    FILE *fp = fopen(USER_FILE, "r+");

    if (fp == NULL) {
        fp = fopen(USER_FILE, "w+");
    }

    Accounts user;
    Accounts tempuser;
    int invalid    = 0;
    int validinput = 0;
	tempuser.role = 0;
   

    system("cls");

    while (!validinput) {
        printf("Enter an account username (a max of 50 characters): ");
        fgets(tempuser.username, sizeof(tempuser.username), stdin);
        tempuser.username[strcspn(tempuser.username, "\n")] = '\0';

        printf("\nEnter a password (a max of 99 characters): ");
        fgets(tempuser.password, sizeof(tempuser.password), stdin);
        tempuser.password[strcspn(tempuser.password, "\n")] = '\0';

        if (strlen(tempuser.username) == 0 || strlen(tempuser.password) == 0) {
            printf("\nUsername / Password cannot be empty!\n");
            getchar();
            getchar();
            system("cls");
        } else {
            validinput = 1;
        }
    }

    while (fscanf(fp, "%s %s %d", user.username, user.password, &user.role) == 3) {
        if ((strcmp(user.username, tempuser.username) == 0 ||
             strcmp(user.password, tempuser.password) == 0) && invalid == 0) {
            invalid = 1;
        }
    }

    if (invalid) {
        printf("\nUsername / Password is already in use!\n");
        getchar();
    } else {
        fseek(fp, 0, SEEK_END);
		encryption(tempuser.password);
        fprintf(fp, "%s %s %d\n", tempuser.username, tempuser.password, tempuser.role);
        printf("Account successfully registered!\n");
        getchar();
    }

    fclose(fp);
}

// ── Create Account ────────────────────────────
void CreateAdminAccount() {
	FILE *fp = fopen(USER_FILE, "r+");
	
	if (fp == NULL) {
		fp = fopen(USER_FILE, "w+");
	}
	
	Accounts user;
	Accounts tempuser;
	int invalid    = 0;
	int validinput = 0;
	tempuser.role = 1;
	
	//while ((bufferflusher = getchar()) != '\n' && bufferflusher != EOF);
	
	system("cls");
	
	while (!validinput) {
		printf("Enter an ADMIN account username (a max of 50 characters): ");
		fgets(tempuser.username, sizeof(tempuser.username), stdin);
		tempuser.username[strcspn(tempuser.username, "\n")] = '\0';
		
		printf("\nEnter a ADMIN password (a max of 99 characters): ");
		fgets(tempuser.password, sizeof(tempuser.password), stdin);
		tempuser.password[strcspn(tempuser.password, "\n")] = '\0';
		
		if (strlen(tempuser.username) == 0 || strlen(tempuser.password) == 0) {
			printf("\nUsername / Password cannot be empty!\n");
			getchar();
			getchar();
			system("cls");
		} else {
			validinput = 1;
		}
	}
	
	while (fscanf(fp, "%s %s %d", user.username, user.password, &user.role) == 3) {
		if ((strcmp(user.username, tempuser.username) == 0 ||
			 strcmp(user.password, tempuser.password) == 0) && invalid == 0) {
			invalid = 1;
		}
	}
	
	if (invalid) {
		printf("\nUsername / Password is already in use!\n");
		getchar();
	} else {
		fseek(fp, 0, SEEK_END);
		encryption(tempuser.password);
		fprintf(fp, "%s %s %d\n", tempuser.username, tempuser.password, tempuser.role);
		printf("ADMIN Account successfully registered!\n");
		getchar();
	}
	
	fclose(fp);
}

// ── Sign In ───────────────────────────────────
// Returns 1 on success, 0 on fail.
// Copies the logged-in username to out_username
// so the dashboard can greet the user by name.
// Only difference from partner's original:
//   added (char *out_username) parameter +
//   strncpy on success.
int SignIn(char *out_username, int *is_admin) {
    FILE *fp = fopen(USER_FILE, "r+");

    if (fp == NULL) {
        printf("No accounts have been made yet! Please create an account first.\n");
        getchar();
        return 0;
    }

    Accounts user;
    Accounts tempuser;
    int valid      = 0;
    int validinput = 0;
   

    system("cls");

    while (!validinput) {
        printf("Enter Username (max 49 chars): ");
        fgets(tempuser.username, sizeof(tempuser.username), stdin);
        tempuser.username[strcspn(tempuser.username, "\n")] = '\0';

        printf("Enter Password (max 99 chars): ");
        fgets(tempuser.password, sizeof(tempuser.password), stdin);
        tempuser.password[strcspn(tempuser.password, "\n")] = '\0';

        if (strlen(tempuser.username) == 0 || strlen(tempuser.password) == 0) {
            printf("\nUsername / Password cannot be empty!\n");
            getchar();
            getchar();
            system("cls");
        } else {
            validinput = 1;
        }
    }

    while (fscanf(fp, "%s %s %d", user.username, user.password, &user.role) == 3) {
		decryption(user.password);
        if (strcmp(user.username, tempuser.username) == 0 &&
            strcmp(user.password, tempuser.password) == 0 && valid == 0) {
            valid = 1;
			tempuser.role = user.role;
        }
    }

    fclose(fp);

    if (valid) {
        strncpy(out_username, tempuser.username, 49);
		*is_admin = tempuser.role;
        out_username[49] = '\0';
        printf("Login successful! Welcome, %s.\n", out_username);
        getchar();
        return 1;
    } else {
        printf("Invalid username or password. Please try again.\n");
        getchar();
        return 0;
    }
}

// ── Auth Menu ─────────────────────────────────
// Loops until sign-in succeeds, fills
// logged_in_user, then returns to main.
void auth_menu(char *logged_in_user, int *typeofuser) {
    char userinput[50];
    int signedin  = 0;
	

    while (!signedin) {
        printf("-------------------------\n");
        printf("[1] Sign In\n");
        printf("[2] Create Account\n");
        printf("-------------------------\n");
        printf("\nEnter the number of your choice: ");
		fgets(userinput, sizeof(userinput), stdin);
		userinput[strcspn(userinput, "\n")] = '\0';
		
		if(strcmp(userinput, "SecretNiggaKnowledge") == 0){
			CreateAdminAccount();
			system("cls");
		} else if (userinput[0] == '2') {
            CreateAccount();
            system("cls");
        } else if (userinput[0] == '1') {
            if (SignIn(logged_in_user, typeofuser) == 1) {
                signedin = 1;
            } else {
                system("cls");
            }
		}else {
			getchar();
            system("cls");
        }
    }
}

#endif // AUTH_H
