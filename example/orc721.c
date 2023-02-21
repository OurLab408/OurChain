#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ourcontract.h>

#define CONTRACT_INIT_FUNC "init"
#define INIT_TOKEN_ARRAY_SIZE 20
#define INIT_ACCOUNT_ARRAY_SIZE 20
#define INIT_ALLOWANCE_ARRAY_SIZE 10
#define INIT_ALLOWANCE_RECORD_ARRAY_SIZE 5
#define INIT_ALLOWANCE_TOKEN_RECORD_ARRAY_SIZE 1

/* Non-fungible token */
typedef struct token {
    char name[20];
    char owner[40]; //account address
} Token

typedef struct account {
    char address[40];
    int balance;
} Account;

typedef struct allowance_record {
    char address[40];
} AllowanceRecord;

typedef struct _allowance {
    char allownace_token_name[20];
    int record_count;
    int allocated_array_size;

    // using pointer to have dynamic size array
    AllowanceRecord *records;
} Allowance;

typedef struct state {
    unsigned int size_contract;
    unsigned int num_token;
    unsigned int allocated_token_array_size;
    unsigned int num_account;
    unsigned int allocated_account_array_size;
    unsigned int num_allowance;
    unsigned int allocated_allowance_array_size;
    char contractOwnerAddress[40];
} ContractState;

/* private function */
static int isOwner(char*, Token*);
static int isAllowance(Account*, Token*);
static void transfer(Acount*, Token*);

/* required APIs */ //
int balanceOf(char*);
char* ownerOf(char*); 
int transferFrom(char*, char*, char*);
int approve(char*, char*);
int mint(char*, char*);

/* function for token table */
static void initTokenArray();
static Token* findToken(char*);
static Token createToken(char*, char*);
static void appendToTokenArray(Token);

/* function for account table */
static void initAccountArray();
static Account* findAccount(char*);
static Account createAccount(char*);
static void appendToAccountArray(Account);

/* function for allowance table */
static void initAllowanceArray();
static Allowance* findAllowance(Account*);
static Allowance createAllowance(Account*);
static void appendToAllowanceArray(Allowance);

/* function for allowance attribute */
static AllowanceRecord* findAllowanceRecord(Allowance*, Account*);
static AllowanceRecord createAllowanceRecord(Account*, int);
static void appendToAllowanceRecordArray(Allowance*, AllowanceRecord);


/* function for read state */
static unsigned int readState();
static unsigned int readContractState(unsigned char*, unsigned int);
static unsigned int readTokenArray(unsigned char*, unsigned int);
static unsigned int readAccountArray(unsigned char*, unsigned int);
static unsigned int readAllowanceArray(unsigned char*, unsigned int);

/* function for write state */
static unsigned int writeState();
static unsigned int writeContractStateToState(unsigned char*, unsigned int);
static unsigned int writeTokenArrayToState(unsigned char*, unsigned int);
static unsigned int writeAccountArrayToState(unsigned char*, unsigned int);
static unsigned int writeAllowanceArrayToState(unsigned char*, unsigned int);

static unsigned int compute_contract_size();

/*
  Debug functions
  err_printf() will print to regtest/contracts/err
  out_printf() will print to regtest/contracts/<contract_id>/out

  Warning: DO NOT use printf(). Usually it will block your program
*/
void print_contract_state();
void print_global_token_array();
void print_global_account_array();
void print_global_allowance_array();
void print_sys_args(int argc, char** argv);
static int too_few_args();

/* global variable (database table) */
Account *globalAccountArray;
Allowance *globalAllowanceArray;
Token *globalTokenArray;
ContractState theContractState;

int contract_main(int argc, char** argv)
{
    if (argc < 2) {
        return too_few_args();
    }

    if (strcmp(argv[1], CONTRACT_INIT_FUNC) == 0) {
        err_printf("init contract\n");

        // contract-related data
        strcpy(ourToken.contractOwnerAddress, "0xContractOwnerAddress");
        strcpy(ourToken.name, "OurToken");
        strcpy(ourToken.symbol, "OTK");
        ourToken.decimal = 0;
        ourToken.totalSupply = 1e8;

        // contract-state data
        initTokenArray();
        initAccountArray();
        initAllowanceArray();
        theContractState.size_contract = compute_contract_size();

        writeState();
    } else {
        readState();

int transferFrom(char*, char*, char*);
int approve(char*, char*);
int mint(char*, char*);

        if (strcmp(argv[1], "balanceOf") == 0) {
            if (argc < 3) {
                err_printf("balanceOf: ");
                return too_few_args();
            }
            err_printf("Account: %s, balanceOf: %d\n", argv[2], balanceOf(argv[2]));

        } else if (strcmp(argv[1], "ownerOf") == 0) {
            if (argc < 3) {
                err_printf("ownerOf: ");
                return too_few_args();
            }
            err_printf("Token: %s, owner: %s\n", argv[2], ownerOf(argv[2]));

        } else if (strcmp(argv[1], "transferFrom") == 0) {
            if (argc < 5) {
                err_printf("transferFrom: ");
                return too_few_args();
            }
            if (transferFrom(argv[2], argv[3], argv[4])) {
                err_printf("Token %s Transfer From %s to %s success!\n", argv[4], argv[2], argv[3]);
            } else {
                err_printf("Token %s Transfer From %s to %s FAILED!\n", argv[4], argv[2], argv[3]);
            }

        } else if (strcmp(argv[1], "approve") == 0) {
            if (argc < 5) {
                err_printf("approve: ");
                return too_few_args();
            }
            if (approve(argv[2], argv[3], argv[4])) {
                err_printf("Token %s authenticate to %s success!\n", argv[4], argv[3]);
            } else {
                err_printf("Token %s authenticate to %s FAILED\n", argv[4], argv[3]);
            }

        } else if (strcmp(argv[1], "mint") == 0) {
            if (argc < 4) {
                err_printf("mint: ");
                return too_few_args();
            }
            if (mint(argv[2], argv[3])) {
                err_printf("Account %s mint Token %s success!\n", argv[2], argv[3]);
            } else {
                err_printf("Account %s mint Token %s FAILED\n", argv[2], argv[3]);
            }

        } else {
            err_printf("Error: command not found\n");
            return 0;
        }
    }

    return 0;
}

static int isOwner(Account *account, Token *token)
{
    if (strcmp(account->address, token->owner)) {
        return 0;
    }
    
    return 1;
}

static int isAllowance(Account *account, Token *token)
{
    Allowance* token_allowance = findAllowance(token);
    if (token_allowance == NULL) {
        return 0;
    }

    AllowanceRecord* record = findAllowanceRecord(token_allowance, spender_account);
    if (record == NULL) {
        return 0;
    }

    return 1;
}

static void transfer(Account* to_account, Token *token) {
    Account* owner_account = findAccount(token->owner);
    
    strcpy(token->owner, to_account->address);
    to_account->balance++;
    owner_account->balance--;

    int ind;
    for (ind = 0; ind < theContractState.num_allowance; ind++) {
        if (strcmp(globalAllowanceArray[ind].allownace_token_name, token->name) == 0) {
            break;
        }
    }

    for(int i = ind; i < theContractState.num_allowance - 1; i++) {
        if (globalAllowanceArray[ind + 1] != NULL) {
            globalAllowanceArray[ind] = globalAllowanceArray[ind + 1];
        } else {
            break;
        }
    }
    return;
}

int balanceOf(char* requester_address)
{
    Account *requester_account = findAccount(requester_address);

    if (requester_account == NULL) {
        err_printf("%s account not found\n", requester_address);
        return 0;
    }

    return requester_account->balance;
}

char* ownerOf(char* request_name)
{
    Token *requester_token = findToken(request_name);

    if (requester_token == NULL) {
        err_printf("%s token not found\n", requester_token);
        return 0;
    }

    return requester_token->owner;
}


int approve(char* owner_address, char* spender_address, char *token_name)
{
    Token* token = findToken(token);
    if (token == NULL) {
        err_printf("Token is not found\n");
        return -1;
    }

    if (strcmp(token->owner, owner_address) != 0) {
        err_printf("Token %s is not %s\n", token->name, owner_address);
        return -1;
    }

    Account* spender_account = findAccount(spender_address);
    if (spender_account == NULL) {
        appendToAccountArray(createAccount(spender_address));
        spender_account = findAccount(spender_address);
    }

    Allowance* token_allowance = findAllowance(token);
    if (token_allowance == NULL) {
        appendToAllowanceArray(createAllowance(&token));
        token_allowance = findAllowance(&token);
    }


    AllowanceRecord* record = findAllowanceRecord(token_allowance, spender_account);
    if (record == NULL) {
        appendToAllowanceRecordArray(token_allowance, createAllowanceRecord(spender_account));
        record = findAllowanceRecord(token_allowance, spender_account);
    } else {
        err_printf("Spender account has been authorization\n");
        return -1;
    }

    return 1;
}

int transferFrom(char* from_address, char* to_address, char* token_name)
{
    Account* from_account = findAccount(from_address);
    if (from_account == NULL) {
        err_printf("From: %s account not found\n", from_address);
        return -1;
    }

    Account* to_account = findAccount(to_address);
    if (to_account == NULL) {
        appendToAccountArray(createAccount(to_address));
        to_account = findAccount(to_address);
    }

    Token* token = findToken(token_name);
    if (token == NULL) {
        err_printf("Token: %s not found\n", token_name);
        return -1;
    }

    if (!isOwner(from_account, token)) {
        if (!isAllowance(from_account, token)) {
            err_printf("Token %s cannot be used by %s\n", token->name, from_account->address);
            return -1;
        }
    }

    transfer(to_account, token);
    return 1;
}

int mint(char *requester_address, char *token_name)
{
    Account* owner_account = findAccount(requester_address);
    if(owner_account == NULL){
        appendToAccountArray(createAccount(requester_address));
        owner_account = findAccount(requester_address);
    }
    
    if (findToken(token_name) != NULL) {
        err_printf("The token has been minted\n");
        return -1;
    }

    Token token = createToken(token_name, requester_address);
    appendToTokenArray(token);
    owner_account->balance++;

    return 1;
}

static void initTokenArray()
{
    globalTokenArray = malloc(sizeof(Token) * INIT_TOKEN_ARRAY_SIZE);

    theContractState.allocated_token_array_size = INIT_TOKEN_ARRAY_SIZE;
    theContractState.num_token = 0;
    return;
}

static Token* findToken(char* name)
{
    for (int i = 0; i < theContractState.num_token; i++) {
        if (strcmp(globalTokenArray[i].name, name) == 0) {
            return &globalTokenArray[i];
        }
    }
    return NULL;
}

static Token createToken(char* name, char* address)
{
    Token token;

    strcpy(token.name, name);
    strcpy(token.owner, address);
    return token;
}

static void appendToTokenArray(Token token)
{
    if (theContractState.num_token < theContractState.allocated_token_array_size) {
        globalAccountArray[theContractState.num_token] = token;
        theContractState.num_token++;
    } else {
        // re-allocate a bigger array
        int new_allocated_token_array_size = theContractState.allocated_token_array_size * 2;
        Token* newTokenArray = malloc(sizeof(Token) * new_allocated_token_array_size);

        for (int i = 0; i < theContractState.allocated_token_array_size; i++) {
            newTokenArray[i] = globalTokenArray[i];
        }

        globalTokenArray = newTokenArray;

        globalTokenArray[theContractState.num_token] = token;
        theContractState.num_token++;
        theContractState.allocated_token_array_size = new_allocated_token_array_size;
    }
    return;
}

static void initAccountArray()
{
    globalAccountArray = malloc(sizeof(Account) * INIT_ACCOUNT_ARRAY_SIZE);

    theContractState.allocated_account_array_size = INIT_ACCOUNT_ARRAY_SIZE;
    theContractState.num_account = 0;
    return;
}

static Account* findAccount(char* address)
{
    for (int i = 0; i < theContractState.num_account; i++) {
        if (strcmp(globalAccountArray[i].address, address) == 0) {
            return &globalAccountArray[i];
        }
    }
    return NULL;
}

static Account createAccount(char* address)
{
    Account account;

    strcpy(account.address, address);
    account.balance = 0;
    return account;
}

static void appendToAccountArray(Account account)
{
    if (theContractState.num_account < theContractState.allocated_account_array_size) {
        globalAccountArray[theContractState.num_account] = account;
        theContractState.num_account++;
    } else {
        // re-allocate a bigger array
        int new_allocated_account_array_size = theContractState.allocated_account_array_size * 2;
        Account* newAccountArray = malloc(sizeof(Account) * new_allocated_account_array_size);

        for (int i = 0; i < theContractState.allocated_account_array_size; i++) {
            newAccountArray[i] = globalAccountArray[i];
        }

        globalAccountArray = newAccountArray;

        globalAccountArray[theContractState.num_account] = account;
        theContractState.num_account++;
        theContractState.allocated_account_array_size = new_allocated_account_array_size;
    }
    return;
}

static void initAllowanceArray()
{
    globalAllowanceArray = malloc(sizeof(Allowance) * INIT_ALLOWANCE_ARRAY_SIZE);

    theContractState.allocated_allowance_array_size = INIT_ALLOWANCE_ARRAY_SIZE;
    theContractState.num_allowance = 0;
    return;
}

static Allowance* findAllowance(Token* token)
{
    for (int i = 0; i < theContractState.num_allowance; i++) {
        if (strcmp(globalAllowanceArray[i].allownace_token_name, token->name) == 0) {
            return &globalAllowanceArray[i];
        }
    }

    return NULL;
}

static Allowance createAllowance(Token* token)
{
    Allowance allowance;
    
    strcpy(allowance.allownace_token_name, token_name);
    allowance.record_count = 0;
    allowance.records = malloc(sizeof(AllowanceRecord) * INIT_ALLOWANCE_RECORD_ARRAY_SIZE);
    allowance.allocated_array_size = INIT_ALLOWANCE_RECORD_ARRAY_SIZE;

    return allowance;
}

static void appendToAllowanceArray(Allowance target_allowance)
{
    if (theContractState.num_allowance < theContractState.allocated_allowance_array_size) {
        globalAllowanceArray[theContractState.num_allowance] = target_allowance;
        theContractState.num_allowance++;
    } else {
        // re-allocate a bigger array
        int new_allocated_allowance_array_size = theContractState.allocated_allowance_array_size * 2;
        Allowance *newAllowanceArray = malloc(sizeof(Allowance) * new_allocated_allowance_array_size);

        for (int i = 0; i < theContractState.allocated_allowance_array_size; i++) {
            newAllowanceArray[i] = globalAllowanceArray[i];
        }

        globalAllowanceArray = newAllowanceArray;

        globalAllowanceArray[theContractState.num_allowance] = target_allowance;
        theContractState.num_allowance++;
        theContractState.allocated_allowance_array_size = new_allocated_allowance_array_size;
    }

    return;
}

static AllowanceRecord* findAllowanceRecord(Allowance *target_allowance, Account *spender_account)
{
    for (int i = 0; i < target_allowance->record_count; i++) {
        /* check token name */
        if (strcmp(target_allowance->records[i].address, spender_account->account) == 0) {
            return &target_allowance->records[i];
        }
    }

    return NULL;
}

static AllowanceRecord createAllowanceRecord(Account* account)
{
    AllowanceRecord record;

    strcpy(record.spender_address, account->address);

    return record;
}

static void appendToAllowanceRecordArray(Allowance *target_allowance, AllowanceRecord record)
{
    if (target_allowance->record_count < target_allowance->allocated_array_size) {
        target_allowance->records[target_allowance->record_count] = record;
        target_allowance->record_count++;
    } else {
        // re-allocate to bigger array
        int new_allocated_array_size = target_allowance->allocated_array_size * 2;
        AllowanceRecord *new_records = malloc(sizeof(AllowanceRecord) * new_allocated_array_size);

        for (int i = 0; i < target_allowance->allocated_array_size; i++) {
            new_records[i] = target_allowance->records[i];
        }

        target_allowance->records = new_records;

        target_allowance->records[target_allowance->record_count] = record;
        target_allowance->record_count++;
        target_allowance->allocated_array_size = new_allocated_array_size;
    }

    return;
}

static unsigned int readState()
{
    /*
        Use state_read() to read your program data
        The data are stored in memory, tight together with UTXO so it will revert automatically

        state_read(buff, size) is straightforward: read `size` bytes to `buff`
        The point is how you define your structure and serialize it

        The following code is just one of the way to read state
            * In write stage:
            * you first write how many byte you stored
            * then write all your data
            * In read stage:
            * first get the size of data
            * then get all the data
            * unserialize the data
    */

    unsigned int count;
    state_read(&count, sizeof(int));

    unsigned char* buff = malloc(sizeof(char) * count);
    unsigned int offset = 0;
    state_read(buff, count);

    offset += readContractState(buff, offset);
    offset += readTokenArray(buff, offset);
    offset += readAccountArray(buff, offset);
    offset += readAllowanceArray(buff, offset);

    assert(offset == count);
    return offset;
}

static unsigned int readContractState(unsigned char* buffer, unsigned int offset)
{
    memcpy(&theContractState, buffer + offset, sizeof(ContractState));
    return sizeof(ContractState);
}

static unsigned int readTokenArray(unsigned char* buffer, unsigned int offset)
{
    globalTokenArray = malloc(sizeof(Token) * theContractState.allocated_token_array_size);
    memcpy(globalTokenArray, buffer + offset, sizeof(Token) * theContractState.allocated_token_array_size);
    return sizeof(Token) * theContractState.allocated_token_array_size;
}

static unsigned int readAccountArray(unsigned char* buffer, unsigned int offset)
{
    globalAccountArray = malloc(sizeof(Account) * theContractState.allocated_account_array_size);
    memcpy(globalAccountArray, buffer + offset, sizeof(Account) * theContractState.allocated_account_array_size);
    return sizeof(Account) * theContractState.allocated_account_array_size;
}

static unsigned int readAllowanceArray(unsigned char* buffer, unsigned int offset)
{
    unsigned int written_bytes = 0;
    globalAllowanceArray = malloc(sizeof(Allowance) * theContractState.allocated_allowance_array_size);

    for (int i = 0; i < theContractState.allocated_allowance_array_size; i++) {
        memcpy(&globalAllowanceArray[i], buffer + offset, sizeof(Allowance));
        written_bytes += sizeof(Allowance);
        offset += sizeof(Allowance);

        if (i <= theContractState.num_allowance) {
            globalAllowanceArray[i].records = malloc(sizeof(AllowanceRecord) * globalAllowanceArray[i].allocated_array_size);
            memcpy(globalAllowanceArray[i].records, buffer + offset, sizeof(AllowanceRecord) * globalAllowanceArray[i].allocated_array_size);
            written_bytes += sizeof(AllowanceRecord) * globalAllowanceArray[i].allocated_array_size;
            offset += sizeof(AllowanceRecord) * globalAllowanceArray[i].allocated_array_size;
        }
    }

static unsigned int writeState()
{
    /*
        Use state_write() to write your program data
        The data are stored in memory, tight together with UTXO so it will revert automatically

        state_read(buff, size) is straightforward: write `size` bytes from `buff`

        Warning: You need to write all your data at once.
        The state is implement as a vector, and will resize every time you use state_write
        So if you write multiple times, it will be the size of last write

        One way to solve this is you memcpy() all your serialized data to a big array
        and then call only one time state_write()
    */

    unsigned char *buff = malloc(sizeof(int) + sizeof(char) * theContractState.size_contract);
    unsigned int offset = 0;

    memcpy(buff, &theContractState.size_contract, sizeof(int));
    offset += sizeof(int);

    offset += writeContractStateToState(buff, offset);
    offset += writeTokenArrayToState(buff, offset);
    offset += writeAccountArrayToState(buff, offset);
    offset += writeAllowanceArrayToState(buff, offset);

    assert(offset == sizeof(int) + sizeof(char)* theContractState.size_contract);
    state_write(buff, offset);
    return offset;
}

static unsigned int writeContractStateToState(unsigned char* buffer, unsigned int offset)
{
    memcpy(buffer + offset, &theContractState, sizeof(ContractState));
    return sizeof(ContractState);
}

static unsigned int writeTokenArrayToState(unsigned char* buffer, unsigned int offset)
{
    memcpy(buffer + offset, globalTokenArray, sizeof(Token) * theContractState.allocated_token_array_size);
    return sizeof(Token) * theContractState.allocated_token_array_size;
}

static unsigned int writeAccountArrayToState(unsigned char* buffer, unsigned int offset)
{
    memcpy(buffer + offset, globalAccountArray, sizeof(Account) * theContractState.allocated_account_array_size);
    return sizeof(Account) * theContractState.allocated_account_array_size;
}

static unsigned int writeAllowanceArrayToState(unsigned char* buffer, unsigned int offset)
{
    unsigned int written_bytes = 0;
    for (int i = 0; i < theContractState.allocated_allowance_array_size; i++) {
        memcpy(buffer + offset + written_bytes, &globalAllowanceArray[i], sizeof(Allowance));
        written_bytes += sizeof(Allowance);
        if (i <= theContractState.num_allowance) {
            memcpy(buffer + offset + written_bytes, globalAllowanceArray[i].records, sizeof(AllowanceRecord) * globalAllowanceArray[i].allocated_array_size);
            written_bytes += sizeof(AllowanceRecord) * globalAllowanceArray[i].allocated_array_size;
        }
    }

    return written_bytes;
}

static unsigned int compute_contract_size()
{
    unsigned int size_sum = 0;

    unsigned int sz_contract_state = sizeof(ContractState);
    unsigned int sz_token_array = sizeof(Token) * theContractState.allocated_token_array_size;
    unsigned int sz_account_array = sizeof(Account) * theContractState.allocated_account_array_size;
    unsigned int sz_allowance_array = sizeof(Allowance) * theContractState.allocated_allowance_array_size;
    unsigned int sz_allowance_records = 0;
    for (int i = 0; i < theContractState.num_allowance; i++) {
        sz_allowance_records += globalAllowanceArray[i].allocated_array_size * sizeof(AllowanceRecord);
    }

    size_sum =  sz_contract_state + sz_token_array + sz_account_array + sz_allowance_array + sz_allowance_records;
    return size_sum;
}

void print_contract_state()
{
    err_printf("{\n\tcontract_size: %u,\n\ttoken_num: %u,\n\ttoken_array_size: %u,\n\taccount_num: %u,\n\taccount_array_size: %u\n\tallowance_num: %u,\n\tallowance_array_size: %u\n}",
               theContractState.size_contract,
               theContractState.num_token,
               theContractState.allocated_token_array_size,
               theContractState.num_account,
               theContractState.allocated_account_array_size,
               theContractState.num_allowance,
               theContractState.allocated_allowance_array_size);
    return;
}

void print_global_token_array()
{
    err_printf("Tokens: \n");
    for (int i = 0; i < theContractState.num_token; i++) {
        err_printf("{ Token_name: %s, owner: %d }\n", globalTokenArray[i].name, globalTokenArray[i].owner);
    }
    return;
}

void print_global_account_array()
{
    err_printf("Accounts: \n");
    for (int i = 0; i < theContractState.num_account; i++) {
        err_printf("{ Accounts: %s, own_tokens: %d\n }", globalAccountArray[i].address, globalAccountArray[i].balance);
    }
    return;
}

typedef struct allowance_record {
    char address[40];
} AllowanceRecord;

typedef struct _allowance {
    char allownace_token_name[20];
    int record_count;
    int allocated_array_size;

    // using pointer to have dynamic size array
    AllowanceRecord *records;
} Allowance;

void print_global_allowance_array()
{
    err_printf("Token Allowance: \n");
    for (int i = 0; i < theContractState.num_allowance; i++) {
        err_printf("Token: %s, records: %d\n{\n", globalAllowanceArray[i].allownace_token_name, globalAllowanceArray[i].record_count);
        for (int j = 0; j < globalAllowanceArray[i].record_count; j++) {
            err_printf("\t%s\n", globalAllowanceArray[i].records[j].spender_address);
        }
        err_printf("}\n");
    }
    return;
}

void print_sys_args(int argc, char** argv)
{
    for (int i = 0; i < argc; i++) err_printf("%s,", argv[i]);
    err_printf("\n");
    return;
}

static int too_few_args()
{
    err_printf("too few arguments\n");
    return -1;
}

