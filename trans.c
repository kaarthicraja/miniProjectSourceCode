// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define MAX_ACCOUNTS 100
#define LAST_NAME_SIZE 15
#define FIRST_NAME_SIZE 10
#define ACCT_NUM_MIN 1
#define ACCT_NUM_MAX 100

// clientData structure definition
struct clientData
{
    unsigned int acctNum;          // account number
    char lastName[LAST_NAME_SIZE]; // account last name
    char firstName[FIRST_NAME_SIZE]; // account first name
    double balance;                // account balance
};                                 // end structure clientData

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
int isValidAccountNum(unsigned int acctNum);
void initializeFile(FILE *fPtr);
void clearInputBuffer(void);
void transferMoney(FILE *fPtr);
void viewAccountStats(FILE *fPtr);
void viewAllAccounts(FILE *fPtr);
void applyInterest(FILE *fPtr);
void searchAccountByName(FILE *fPtr);

int main(void)
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    // fopen opens the file; exits if file cannot be opened
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("credit.dat: File could not be opened for reading/updating.\n");
        printf("Attempting to create new file...\n");
        
        // Try to create the file
        if ((cfPtr = fopen("credit.dat", "w+b")) == NULL)
        {
            printf("Could not create credit.dat file.\n");
            exit(-1);
        }
        
        // Initialize the file with 100 blank records
        initializeFile(cfPtr);
        printf("New credit.dat file created and initialized.\n");
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 9)
    {
        switch (choice)
        {
        // create text file from record file
        case 1:
            textFile(cfPtr);
            break;
        // update record
        case 2:
            updateRecord(cfPtr);
            break;
        // create record
        case 3:
            newRecord(cfPtr);
            break;
        // delete existing record
        case 4:
            deleteRecord(cfPtr);
            break;
        // NEW: transfer money
        case 5:
            transferMoney(cfPtr);
            break;
        // NEW: view account statistics
        case 6:
            viewAccountStats(cfPtr);
            break;
        // NEW: view all accounts
        case 7:
            viewAllAccounts(cfPtr);
            break;
        // NEW: apply interest
        case 8:
            applyInterest(cfPtr);
            break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice. Please enter 1-9.");
            break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
    printf("Program terminated. Goodbye!\n");
} // end main

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    int result;     // used to test whether fread read any bytes
    int recordCount = 0; // count of records written
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("accounts.txt could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "----", "---------", "----------", "-------");

        // copy all records from random-access file into text file
        // FIX: Use fread return value instead of feof()
        while ((result = fread(&client, sizeof(struct clientData), 1, readPtr)) == 1)
        {
            // write single record to text file
            if (client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.balance);
                recordCount++;
            } // end if
        }     // end while

        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "----", "---------", "----------", "-------");
        fprintf(writePtr, "Total accounts: %d\n", recordCount);
        
        fclose(writePtr); // fclose closes the file
        printf("Accounts file created successfully with %d records.\n", recordCount);
    }                     // end else
} // end function textFile

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    int scanResult;       // scanf return value for validation
    // create clientData with no information
    struct clientData client = {0, "", "", 0.0};

    // obtain number of account to update
    printf("%s", "Enter account to update ( 1 - 100 ): ");
    scanResult = scanf("%u", &account);
    clearInputBuffer();
    
    // Validate account number
    if (scanResult != 1 || !isValidAccountNum(account))
    {
        printf("Invalid account number. Please enter a number between %d and %d.\n", ACCT_NUM_MIN, ACCT_NUM_MAX);
        return;
    }

    // move file pointer to correct record in file
    if (fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET) != 0)
    {
        printf("Error seeking in file.\n");
        return;
    }
    
    // read record from file
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        printf("Error reading from file.\n");
        return;
    }
    
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
    }
    else
    { // update record
        printf("\nCurrent Account: %-6u %-16s %-11s Balance: $%10.2f\n\n", 
               client.acctNum, client.lastName, client.firstName, client.balance);

        // request transaction amount from user
        printf("%s", "Enter charge (+) or payment (-): ");
        scanResult = scanf("%lf", &transaction);
        clearInputBuffer();
        
        if (scanResult != 1)
        {
            printf("Invalid transaction amount.\n");
            return;
        }
        
        client.balance += transaction; // update record balance
        
        if (client.balance < 0)
        {
            printf("Warning: Account balance is negative ($%.2f)\n", client.balance);
        }

        printf("Updated Account:  %-6u %-16s %-11s Balance: $%10.2f\n", 
               client.acctNum, client.lastName, client.firstName, client.balance);

        // move file pointer back to correct record in file to overwrite
        // FIX: Changed from +sizeof to -sizeof to go BACK one record
        if (fseek(fPtr, -sizeof(struct clientData), SEEK_CUR) != 0)
        {
            printf("Error seeking in file.\n");
            return;
        }
        
        // write updated record over old record in file
        if (fwrite(&client, sizeof(struct clientData), 1, fPtr) != 1)
        {
            printf("Error writing to file.\n");
            return;
        }
        
        printf("Record updated successfully.\n");
    } // end else
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;                       // stores record read from file
    struct clientData blankClient = {0, "", "", 0}; // blank client
    unsigned int accountNum;                        // account number
    int scanResult;                                 // scanf return value

    // obtain number of account to delete
    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    scanResult = scanf("%u", &accountNum);
    clearInputBuffer();
    
    // Validate account number
    if (scanResult != 1 || !isValidAccountNum(accountNum))
    {
        printf("Invalid account number. Please enter a number between %d and %d.\n", ACCT_NUM_MIN, ACCT_NUM_MAX);
        return;
    }

    // move file pointer to correct record in file
    if (fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET) != 0)
    {
        printf("Error seeking in file.\n");
        return;
    }
    
    // read record from file
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        printf("Error reading from file.\n");
        return;
    }
    
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%u does not exist.\n", accountNum);
    } // end if
    else
    { // delete record
        printf("Deleting Account: %-6u %-16s %-11s Balance: $%10.2f\n", 
               client.acctNum, client.lastName, client.firstName, client.balance);
        
        // move file pointer to correct record in file
        if (fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET) != 0)
        {
            printf("Error seeking in file.\n");
            return;
        }
        
        // replace existing record with blank record
        if (fwrite(&blankClient, sizeof(struct clientData), 1, fPtr) != 1)
        {
            printf("Error writing to file.\n");
            return;
        }
        
        printf("Account deleted successfully.\n");
    } // end else
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum; // account number
    int scanResult;          // scanf return value

    // obtain number of account to create
    printf("%s", "Enter new account number ( 1 - 100 ): ");
    scanResult = scanf("%u", &accountNum);
    clearInputBuffer();
    
    // Validate account number
    if (scanResult != 1 || !isValidAccountNum(accountNum))
    {
        printf("Invalid account number. Please enter a number between %d and %d.\n", ACCT_NUM_MIN, ACCT_NUM_MAX);
        return;
    }

    // move file pointer to correct record in file
    if (fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET) != 0)
    {
        printf("Error seeking in file.\n");
        return;
    }
    
    // read record from file
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        printf("Error reading from file.\n");
        return;
    }
    
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%u already contains information.\n", client.acctNum);
    } // end if
    else
    { // create record
        // user enters last name, first name and balance
        printf("%s", "Enter lastname, firstname, balance\n? ");
        scanResult = scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance);
        clearInputBuffer();
        
        if (scanResult != 3)
        {
            printf("Invalid input. Please enter last name, first name, and balance.\n");
            return;
        }
        
        if (client.balance < 0)
        {
            printf("Warning: Initial balance is negative ($%.2f)\n", client.balance);
        }

        client.acctNum = accountNum;
        
        // move file pointer to correct record in file
        if (fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET) != 0)
        {
            printf("Error seeking in file.\n");
            return;
        }
        
        // insert record in file
        if (fwrite(&client, sizeof(struct clientData), 1, fPtr) != 1)
        {
            printf("Error writing to file.\n");
            return;
        }
        
        printf("Account #%u created successfully for %s %s with balance $%.2f\n",
               client.acctNum, client.firstName, client.lastName, client.balance);
    } // end else
} // end function newRecord

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    int scanResult;          // scanf return value
    
    // display available options
    printf("%s", "\n===== BANK ACCOUNT MANAGEMENT SYSTEM =====\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account balance\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - transfer money between accounts (NEW)\n"
                 "6 - view account statistics and search (NEW)\n"
                 "7 - view all active accounts (NEW)\n"
                 "8 - apply monthly interest (NEW)\n"
                 "9 - end program\n? ");

    scanResult = scanf("%u", &menuChoice); // receive choice from user
    clearInputBuffer();
    
    if (scanResult != 1)
    {
        printf("Invalid input. Please enter a number between 1 and 9.\n");
        return 0; // Return 0 for invalid choice to continue loop
    }
    
    return menuChoice;
} // end function enterChoice

// Validate account number is in range
int isValidAccountNum(unsigned int acctNum)
{
    return (acctNum >= ACCT_NUM_MIN && acctNum <= ACCT_NUM_MAX);
} // end function isValidAccountNum

// Clear remaining characters from input buffer
void clearInputBuffer(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ; // Discard characters until newline or EOF
} // end function clearInputBuffer

// Initialize file with blank records
void initializeFile(FILE *fPtr)
{
    struct clientData blankClient = {0, "", "", 0.0};
    
    // Write 100 blank records
    for (int i = 0; i < MAX_ACCOUNTS; i++)
    {
        if (fwrite(&blankClient, sizeof(struct clientData), 1, fPtr) != 1)
        {
            printf("Error initializing file at record %d.\n", i + 1);
            return;
        }
    }
    
    fflush(fPtr); // Flush to ensure data is written
} // end function initializeFile

// =============== NEW UNIQUE FEATURES ===============

// Transfer money from one account to another
void transferMoney(FILE *fPtr)
{
    unsigned int fromAcct, toAcct;
    double amount;
    int scanResult;
    struct clientData fromClient = {0, "", "", 0.0};
    struct clientData toClient = {0, "", "", 0.0};
    
    printf("\n--- TRANSFER MONEY BETWEEN ACCOUNTS ---\n");
    
    // Get source account
    printf("Enter source account number (1-100): ");
    scanResult = scanf("%u", &fromAcct);
    clearInputBuffer();
    
    if (scanResult != 1 || !isValidAccountNum(fromAcct))
    {
        printf("Invalid account number.\n");
        return;
    }
    
    // Get destination account
    printf("Enter destination account number (1-100): ");
    scanResult = scanf("%u", &toAcct);
    clearInputBuffer();
    
    if (scanResult != 1 || !isValidAccountNum(toAcct))
    {
        printf("Invalid account number.\n");
        return;
    }
    
    if (fromAcct == toAcct)
    {
        printf("Error: Source and destination must be different.\n");
        return;
    }
    
    // Get transfer amount
    printf("Enter transfer amount: $");
    scanResult = scanf("%lf", &amount);
    clearInputBuffer();
    
    if (scanResult != 1 || amount <= 0)
    {
        printf("Invalid amount. Must be positive.\n");
        return;
    }
    
    // Read source account
    fseek(fPtr, (fromAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&fromClient, sizeof(struct clientData), 1, fPtr);
    
    if (fromClient.acctNum == 0)
    {
        printf("Error: Source account #%u does not exist.\n", fromAcct);
        return;
    }
    
    if (fromClient.balance < amount)
    {
        printf("Error: Insufficient funds. Available: $%.2f\n", fromClient.balance);
        return;
    }
    
    // Read destination account
    fseek(fPtr, (toAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&toClient, sizeof(struct clientData), 1, fPtr);
    
    if (toClient.acctNum == 0)
    {
        printf("Error: Destination account #%u does not exist.\n", toAcct);
        return;
    }
    
    // Perform transfer
    fromClient.balance -= amount;
    toClient.balance += amount;
    
    // Write updated source account
    fseek(fPtr, (fromAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&fromClient, sizeof(struct clientData), 1, fPtr);
    
    // Write updated destination account
    fseek(fPtr, (toAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&toClient, sizeof(struct clientData), 1, fPtr);
    
    printf("\n✓ Transfer successful!\n");
    printf("  From: %s %s (Account #%u) - New Balance: $%.2f\n", 
           fromClient.firstName, fromClient.lastName, fromClient.acctNum, fromClient.balance);
    printf("  To:   %s %s (Account #%u) - New Balance: $%.2f\n", 
           toClient.firstName, toClient.lastName, toClient.acctNum, toClient.balance);
} // end function transferMoney

// View account statistics and search by name
void viewAccountStats(FILE *fPtr)
{
    char searchName[LAST_NAME_SIZE];
    struct clientData client = {0, "", "", 0.0};
    int found = 0;
    double totalBalance = 0.0;
    int totalAccounts = 0;
    
    printf("\n--- ACCOUNT STATISTICS & SEARCH ---\n");
    printf("Enter last name to search (or press Enter to show all): ");
    
    fgets(searchName, sizeof(searchName), stdin);
    
    // Remove newline
    size_t len = strlen(searchName);
    if (len > 0 && searchName[len - 1] == '\n')
        searchName[len - 1] = '\0';
    
    rewind(fPtr);
    
    printf("\n%-6s %-15s %-10s %-15s\n", "Acct#", "Last Name", "First Name", "Balance");
    printf("%-6s %-15s %-10s %-15s\n", "-----", "-----------", "---------", "--------");
    
    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            if (strlen(searchName) == 0 || strstr(client.lastName, searchName) != NULL)
            {
                printf("%-6u %-15s %-10s $%14.2f\n", 
                       client.acctNum, client.lastName, client.firstName, client.balance);
                totalBalance += client.balance;
                found++;
            }
            totalAccounts++;
        }
    }
    
    printf("%-6s %-15s %-10s %-15s\n", "-----", "-----------", "---------", "--------");
    printf("Accounts Found: %d | Total Balance in Search: $%.2f\n", found, totalBalance);
    printf("Total Active Accounts: %d\n", totalAccounts);
} // end function viewAccountStats

// View all active accounts with details
void viewAllAccounts(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    int count = 0;
    double totalBalance = 0.0;
    double highestBalance = 0.0, lowestBalance = 999999.99;
    unsigned int highestAcct = 0, lowestAcct = 0;
    
    printf("\n--- ALL ACTIVE ACCOUNTS ---\n");
    
    rewind(fPtr);
    
    printf("\n%-6s %-15s %-10s %-15s\n", "Acct#", "Last Name", "First Name", "Balance");
    printf("%-6s %-15s %-10s %-15s\n", "-----", "-----------", "---------", "--------");
    
    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            printf("%-6u %-15s %-10s $%14.2f\n", 
                   client.acctNum, client.lastName, client.firstName, client.balance);
            
            totalBalance += client.balance;
            count++;
            
            if (client.balance > highestBalance)
            {
                highestBalance = client.balance;
                highestAcct = client.acctNum;
            }
            
            if (client.balance < lowestBalance)
            {
                lowestBalance = client.balance;
                lowestAcct = client.acctNum;
            }
        }
    }
    
    printf("%-6s %-15s %-10s %-15s\n", "-----", "-----------", "---------", "--------");
    printf("\nSummary:\n");
    printf("Total Active Accounts: %d\n", count);
    printf("Total Balance: $%.2f\n", totalBalance);
    printf("Average Balance: $%.2f\n", count > 0 ? totalBalance / count : 0.0);
    printf("Highest Balance: $%.2f (Account #%u)\n", highestBalance, highestAcct);
    printf("Lowest Balance: $%.2f (Account #%u)\n", lowestBalance, lowestAcct);
} // end function viewAllAccounts

// Apply monthly interest to all accounts
void applyInterest(FILE *fPtr)
{
    double interestRate;
    int scanResult;
    struct clientData client = {0, "", "", 0.0};
    int count = 0;
    double totalInterest = 0.0;
    
    printf("\n--- APPLY MONTHLY INTEREST ---\n");
    printf("Enter monthly interest rate (%%): ");
    scanResult = scanf("%lf", &interestRate);
    clearInputBuffer();
    
    if (scanResult != 1 || interestRate < 0 || interestRate > 100)
    {
        printf("Invalid interest rate. Must be between 0 and 100.\n");
        return;
    }
    
    rewind(fPtr);
    
    printf("\nApplying %.2f%% interest to all accounts...\n\n", interestRate);
    printf("%-6s %-15s %-10s %-15s %-15s\n", "Acct#", "Last Name", "First Name", "Old Balance", "Interest");
    printf("%-6s %-15s %-10s %-15s %-15s\n", "-----", "-----------", "---------", "-----------", "---------");
    
    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            double interest = client.balance * (interestRate / 100.0);
            client.balance += interest;
            totalInterest += interest;
            count++;
            
            printf("%-6u %-15s %-10s $%14.2f $%14.2f\n", 
                   client.acctNum, client.lastName, client.firstName, 
                   client.balance - interest, interest);
            
            // Write updated balance back
            fseek(fPtr, -sizeof(struct clientData), SEEK_CUR);
            fwrite(&client, sizeof(struct clientData), 1, fPtr);
        }
    }
    
    printf("%-6s %-15s %-10s %-15s %-15s\n", "-----", "-----------", "---------", "-----------", "---------");
    printf("\nInterest Applied:\n");
    printf("Total Accounts: %d\n", count);
    printf("Total Interest Added: $%.2f\n", totalInterest);
    printf("✓ Interest application complete!\n");
} // end function applyInterest

// Search account by name (helper - can be called separately)
void searchAccountByName(FILE *fPtr)
{
    char lastName[LAST_NAME_SIZE];
    struct clientData client = {0, "", "", 0.0};
    int found = 0;
    
    printf("Enter last name to search: ");
    scanf("%14s", lastName);
    clearInputBuffer();
    
    rewind(fPtr);
    
    printf("\nSearch Results for '%s':\n", lastName);
    printf("%-6s %-15s %-10s %-15s\n", "Acct#", "Last Name", "First Name", "Balance");
    printf("%-6s %-15s %-10s %-15s\n", "-----", "-----------", "---------", "--------");
    
    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0 && strcmp(client.lastName, lastName) == 0)
        {
            printf("%-6u %-15s %-10s $%14.2f\n", 
                   client.acctNum, client.lastName, client.firstName, client.balance);
            found++;
        }
    }
    
    if (found == 0)
        printf("No accounts found with last name '%s'.\n", lastName);
    else
        printf("Total matches: %d\n", found);
} // end function searchAccountByName