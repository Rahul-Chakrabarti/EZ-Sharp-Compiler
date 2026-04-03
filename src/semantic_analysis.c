/*CP471 Semantic Analysis and Intermediate Code Generation (Phase 3 + 4)
This code is the same as my syntax code, but changed to look for semantic errors instead of syntax errors (assuming syntax is correct)
Also, as I progress through the token file, I update intermediate.txt with the 3TAC code of the original code given.
By: Ethan Sadler
#210557460
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stddef.h>

FILE* ll1;
FILE* tokens; //token lexeme pairs from phase 1
FILE* error; //error log file
FILE* inter; //file to print the intermediate code

char *tokenpair[500][3];
char *separator = "|"; //global variable for separator terminal
char *space = " ";
int x = 0;
char* term = " ";
int lineCount = 0;

char* type_check[1][7];
int type_flag = 0;
int func_flag = 0;
int func_count = 4;
int method_count = 0;
int local_flag = 0;
int switcher = 0;
int fi_dup = 0;
int open = 0;
int l1y = 0;
int l1x = 0;
int err = 0;
char* curr_type;
char* curr_id;
int symbol_count = 0;
char *temp_s[5][6];
char *temp_int[1][20];
char *temp_func[1][20];
int tfunc_count = 0;
int inter_count = 0;
int inter_line = -1;
int inter_flag = 0;
int term_counter = 1;
int ret_flag = 0;
int pri_flag = 0;

int arg_count = 8;
int temp_arg_count = 0;
int comp_flag = 0;
int label_counter = 1;
int el_flag = 0;
int method_store[5][5];
int index_store[5][5];
int store_count = 0;

char *global[10][7];
char *local[10][7];
char test[100][10];

int currTestVal = 0;
int definition = 0;
int def_count = 4;
int num = 0;
int declaration = 0;
char ***currScope = (char***) global;
char ***otherScope = (char***) local;
int x_sem = 0;
int x_temp = 0;

int ltable[37][32];

char* scope[100][100] = {"Global", "Procedure Parameter", "Inner"};
int scope_index = 0;
//updated grammar
char* LHS[100][100] = {"Program", "FDECLS", "FDECLS", "FDECLS2", "FDCLS2", "FDEC", "PARAMS", "PARAMS", "PARAMS2", "PARAMS2", "FNAME", "DECLARATIONS", "DECLARATIONS", "DECLARATIONS2", "DECLARATIONS2", "DECL", "TYPE", "TYPE", "VARLIST", "VARLIST2", "VARLIST2", "STATEMENT_SEQ", "STATEMENT_SEQ2", "STATEMENT_SEQ2", "STATEMENT", "STATEMENT", "STATEMENT2", "STATEMENT2", "STATEMENT", "STATEMENT", "STATEMENT", "STATEMENT", "EXPR", "EXPR2", "EXPR2", "EXPR2", "TERM", "TERM2", "TERM2", "TERM2", "TERM2", "FACTOR", "FACTOR", "FACTOR", "FACTOR2","FACTOR2", "EXPRSEQ", "EXPRSEQ", "EXPRSEQ2", "EXPRSEQ2", "BEXPR", "BEXPR2", "BEXPR2", "BTERM", "BTERM2", "BTERM2", "BFACTOR", "BFACTOR", "VAR", "VAR2", "VAR2", "M1", "M2", "M3", "M4", "M5"};
char* RHS[100][100] = {"FDECLS DECLARATIONS STATEMENT_SEQ .", "FDEC ; FDECLS2", "#", "FDEC ; FDECLS2", "#", "def TYPE FNAME ( PARAMS ) DECLARATIONS STATEMENT_SEQ fed", "TYPE VAR PARAMS2", "#", ", PARAMS", "#", "ID", "DECL ; DECLARATIONS2", "#", "DECL ; DECLARATIONS2", "#", "TYPE VARLIST M3", "int", "double", "VAR VARLIST2", ", VARLIST", "#", "STATEMENT STATEMENT_SEQ2", "; STATEMENT_SEQ", "#", "VAR M1 = EXPR M2", "if BEXPR then STATEMENT_SEQ STATEMENT2", "fi", "else STATEMENT_SEQ fi", "while BEXPR do STATEMENT_SEQ od", "print EXPR", "return EXPR", "#", "TERM EXPR2", "+ TERM EXPR2", "- TERM EXPR2", "#", "FACTOR TERM2", "* FACTOR TERM2", "/ FACTOR TERM2", "% FACTOR TERM2", "#", "ID M4 FACTOR2 M4 M5", "NUMBER", "( EXPR )", "VAR2", "( EXPRSEQ )", "EXPR EXPRSEQ2", "#", ", EXPRSEQ", "#", "BTERM BEXPR2", "or BTERM BEXPR2", "#", "BFACTOR BTERM2", "and BFACTOR BTERM2", "#", "not BFACTOR", "( EXPR COMP EXPR )", "ID VAR2", "[ EXPR ]", "#", "#", "#", "#", "#", "#"};

//all terminals in the grammar
char* terminals[34][100] = {"$", ".", ";", "def", "(",  ")", "fed", ",", "ID", "int", "double", "=", "if", "then", "fi", "else", "while",
    "do", "od", "print", "return", "+", "-", "*", "/", "%", "NUMBER", "or", "and", "not", "COMP", "[", "]", "#"};

char* operators[7][100] = {"<", ">", "==", ">=", "<=", "<>"};
char* number[3][100] = {"INT", "DOUBLE"};
char* headers[4][100] = {"ID", "NUMBER", "COMP"};
//all nonterminals in the grammar
char* nonterminals[37][100] = {"Program", "FDECLS", "FDECLS2", "FDEC", "PARAMS", "PARAMS2", "FNAME", "DECLARATIONS", "DECLARATIONS2",
    "DECL", "TYPE", "VARLIST", "VARLIST2", "STATEMENT_SEQ", "STATEMENT_SEQ2", "STATEMENT", "STATEMENT2", "EXPR", "EXPR2", "TERM",
    "TERM2", "FACTOR", "FACTOR2", "EXPRSEQ", "EXPRSEQ2", "BEXPR", "BEXPR2", "BTERM", "BTERM2", "BFACTOR", "VAR", "VAR2", "M1", "M2", "M3", "M4", "M5"};


//data structure for an AST Node
typedef struct ASTNode {
    char *name;
    int type; //0 if it is a terminal, 1 if it is a non-terminal
    int searched; //0 if it hasn't been searched yet, 1 if it has
    struct ASTNode* children[15];
    int total_children;
} ASTNode;

typedef struct SymbolNode {
    char *data[100][3];
    struct SymbolNode* child;
    struct SymbolNode* parent;
} SymbolNode;

SymbolNode* symbols = NULL;
int symbols_size = 0;

// CUSTOM STRING FUNCTIONS FROM SCRATCH

char* my_strcpy(char* dest, const char* src) {
    char* original = dest;
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0'; // Null-terminate the destination string
    return original;
}

int my_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)(*s1) - (unsigned char)(*s2);
}

int my_atoi(const char* str) {
    int result = 0;
    int sign = 1;
    // Skip leading whitespace
    while (*str == ' ' || (*str >= '\t' && *str <= '\r')) str++;
    // Optional sign
    if (*str == '-' || *str == '+') {
        if (*str == '-') sign = -1;
        str++;
    }
    // Parse digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return sign * result;
}

// Helper: checks if character is in the delimiter string 
int is_delim(char ch, const char* delim) {
    // Used for my_atok
    while (*delim) {
        if (ch == *delim) return 1;
        delim++;
    }
    return 0;
}

// Custom strtok: tokenizes and splits string
char* my_atok(char* str, const char* delim) {
    static char* next_token = 0;
    if (str != 0) {
        next_token = str;
    }
    if (next_token == 0 || *next_token == '\0') {
        return 0;
    }
    // Skip leading delimiters
    while (*next_token && is_delim(*next_token, delim)) { // Use of helper function 
        next_token++;
    }
    if (*next_token == '\0') return 0;
    char* token_start = next_token;
    // Move to next delimiter or end
    while (*next_token && !is_delim(*next_token, delim)) {
        next_token++;
    }
    // Null-terminate current token
    if (*next_token) {
        *next_token = '\0';
        next_token++;
    } else {
        next_token = 0;  // No more tokens
    }
    return token_start;
} 
 
// Custom itoa: Only supports base 10; keeps base param for compatibility (dont wnat change the signuature)
char* my_itoa(int value, char* str, int base) {
    if (base != 10) {
        str[0] = '\0';  // if not base 10 easy base case
        return str;
    }

    char* p = str;
    int is_negative = 0;

    // Handle zero
    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return str;
    }

    // Handle negatives
    unsigned int uvalue;
    if (value < 0) {
        is_negative = 1;
        uvalue = (unsigned int)(-value);
    } else {
        uvalue = (unsigned int)value;
    }

    // Extract digits in reverse order (no reverse helper function like geeksforgeeks because didnt know if its allowed)
    while (uvalue > 0) {
        *p++ = (uvalue % 10) + '0';
        uvalue /= 10;
    }
    // Add negative sign if needed
    if (is_negative) {
        *p++ = '-';
    }
    *p = '\0';

    // Reverse the string in-place
    char* start = str;
    char* end = p - 1;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    return str;
} 


// custom strlen function
size_t my_strlen(const char* str) {
    size_t length = 0;
    while (*str++) {
        length++;
    }
    return length;
}

SymbolNode* createSymbol(char *data[1][3]){
    SymbolNode* newNode = (SymbolNode*)malloc(sizeof(SymbolNode));
    newNode->child = NULL;
    newNode->parent = NULL;
    newNode->data[symbol_count][0] = data[0][0];
    //printf("%s\n", newNode->data[symbol_count][0]);
    newNode->data[symbol_count][1] = data[0][1];
    //printf("%s\n", newNode->data[symbol_count][1]);
    newNode->data[symbol_count][2] = data[0][2];
    //printf("%s\n", newNode->data[symbol_count][2]);
    symbol_count++;
    
    return newNode;

}



//method to search the symbol table(s) to see if the identifier exists in the table
int semantic_search(){ 
    if (currScope == global){ //if the current pointer is the global symbol table
        printf("GLOBAL\n");
        printf("%d", x_sem);
        for (int i = 0;i<x_sem;i++){
            if (my_strcmp(currScope[i][0], currScope[x_sem][0]) == 0){
                
                return i;
            }
        }
        return -1;
    } else { //else if it is the local symbol table
        printf("LOCAL\n");
        for (int i = 0;i<x_sem;i++){
            if (my_strcmp(currScope[i][0], currScope[x_sem][0]) == 0){
                if (local_flag == 1){
                    local_flag = 0;
                    switcher = 1;

                }
                return i;
            }
        }
        //if it doesn't exist in local symbol table, check global symbol table
        for (int i = 0;i<x_temp;i++){
            if (my_strcmp(otherScope[i][0], currScope[x_sem][0]) == 0){
                local_flag = 1;
                
                return i;
            }
        }
        return -1;
    }
}


//symbol table creation!!!
int symboltable(char *temp_s[1][6]){
    currScope[x_sem][0] = temp_s[0][0];
    int val = semantic_search();
    printf("SEARCH FOR %s = %d\n", currScope[x_sem][0], val);
    printf("Definition = %d, declaration = %d\n", definition, declaration);

    if (definition == 1){ //if this identifier is a definition
        if (val >= 0){ //ERROR: REDECLARATION OF FUNCTION
            printf("ERROR: def = 1");
            fprintf(error, "scoping error on line %s\n", tokenpair[x][2]);
            return 0;
        }
        fprintf(inter, "jump main\n");
        fprintf(inter, "%s:\n", tokenpair[x][1]);
        fprintf(inter, "push LR\n");
        fprintf(inter, "push FP\n");
        fprintf(inter, "FP = SP\n");
        temp_arg_count = arg_count;
        //flips the current scope to the local scope
        for (int i = 1;i<6;i++){
            currScope[x_sem][i] = temp_s[0][i];
        }
        
        char ***a = otherScope;
        otherScope = currScope;
        currScope = a;
        //flips the current x value to the local x value
        printf("FLIP\n");
        int b = x_sem;
        x_sem = x_temp;
        x_temp = b;

        definition = 2;
        return 0;
        

    } else if (definition == 2){
        if (val >= 0){ //ERROR: REDECLARATION
            printf("ERROR: def = 2");
            fprintf(error, "scoping error on line %s\n", tokenpair[x][2]);
            return 0;
        }

        if (def_count == 4){
            currScope[x_sem][4] = "-1";
            currScope[x_sem][5] = "-1";
            currScope[x_sem][6] = "-1";
        }
        for (int i = 1;i<6;i++){
            currScope[x_sem][i] = temp_s[0][i];
        }
        for (int i = 0;i<x_sem+1;i++){
            printf("%s, %s, %s, %s, %s\n", currScope[i][0], currScope[i][1], currScope[i][2], currScope[i][3], currScope[i][4]);
        }
        
        if (my_strcmp(currScope[x_sem][1], "int") == 0){
            otherScope[x_temp][def_count] = "int";
        } else {
            otherScope[x_temp][def_count] = "double";
        }
        fprintf(inter, "%s = fp + %d\n", tokenpair[x][1], arg_count);
        arg_count+=4;
        def_count++;
        x_sem++;
        num++;
        
    } else if (declaration == 1){ //if this identifier is a new declaration
        if (val >= 0){ //THROW ERROR: REDECLARATION OF A VARIABLE
            printf("ERROR: decl = 1");
            fprintf(error, "scoping error on line %s\n", tokenpair[x][2]);
            return 0;
        }
        for (int i = 1;i<6;i++){
            currScope[x_sem][i] = temp_s[0][i];
        }
        fprintf(inter, "%s = fp + %d\n", tokenpair[x][1], arg_count);
        arg_count+=4;
        x_sem++;
        
    } else { //if this identifier is not a declaration or a definition
        
        if (val == -1){ //ERROR: VARAIBLE IS NOT DECLARED
            printf("ERROR: decl = 0");
            fprintf(error, "scoping error on line %s\n", tokenpair[x][2]);
            return 0;
        }
        if (inter_line == -1){
            inter_line = my_atoi(tokenpair[x][2]);
            inter_flag = 1;
        } 
        if (inter_line != my_atoi(tokenpair[x][2])){
            printf("INTER LINE = %d\nOTHER = %d\n", inter_line, my_atoi(tokenpair[x][2]));
            for (int i = 1;i<8;i++){
                temp_int[0][i] = "-1";
            }
            inter_line = my_atoi(tokenpair[x][2]);
            inter_count = 1;
            temp_int[0][0] = tokenpair[x][1];
        } else {
            temp_int[0][inter_count] = tokenpair[x][1];
            inter_count++;
            inter_flag = 1;
            printf("NEW COUNTER 1 = %d\n", inter_count);
        }
        //check to see if it needs to be type checked
        if (type_flag == 0 && func_flag == 0){ //if the flag is off (first identifier that needs to be type checked)
            if (my_strcmp(currScope[val][1], "int") == 0){ //if it is just a integer
                type_check[0][0] = "int";

            } else { //else, if it is a double
                type_check[0][0] = "double";
            }
            type_flag = 1;
        } else if (type_flag == 1 && func_flag == 0){ //if the flag is on (second identifier that needs to be type checked)
            if (local_flag == 1){
                if (my_strcmp(otherScope[val][1], type_check[0][0]) == 0){
                    printf("TYPES ARE THE SAME!!\n");
                } else {
                    printf("SEMANTIC ERROR\n");
                    fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                }
            } else {
                if (my_strcmp(currScope[val][1], type_check[0][0]) == 0){ //type check is correct
                    printf("TYPES ARE THE SAME!\n");
                } else { //type check is incorrect
                    printf("SEMANTIC ERROR\n");
                    fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                }
            }
            type_flag = 0;
            
        } else if (func_flag > 0){
            if (local_flag == 1){
                printf("REQ = %s\n", otherScope[val][2]);
                printf("LOCAL FLAG = %d\n", local_flag);
                if (my_strcmp(otherScope[val][2], "1") == 0){
                    func_flag++;
                    method_store[0][store_count] = method_count;
                    index_store[0][store_count] = func_count;
                    method_count = my_atoi(otherScope[0][3]);
                    store_count++;
                    func_count = 4;
                    if (my_strcmp(otherScope[val][1], type_check[0][0]) == 0){
                        printf("TYPES ARE THE SAME!!\n");
                    } else {
                        printf("SEMANTIC ERROR\n");
                        fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                    }
                    type_flag = 0;
                } else {
                    if (method_count > 0){
                        temp_func[0][tfunc_count] = currScope[val][0];
                        tfunc_count++;
                        if (my_strcmp(otherScope[0][func_count], currScope[val][1]) == 0){
                            printf("FUNCTION TYPES MATCH WITH INDEX %d\n", func_count);
                        } else {
                            printf("SEMANTIC ERROR\n");
                            fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                        }
                        
                    }
                }
            } else {
                if (my_strcmp(currScope[val][2], "1") == 0){ //if it is a function
                    func_flag++;
                    method_store[0][store_count] = method_count;
                    index_store[0][store_count] = func_count;
                    method_count = my_atoi(currScope[0][3]);
                    store_count++;
                    func_count = 4;
                    if (my_strcmp(currScope[val][1], type_check[0][0]) == 0){ //type check is correct
                        printf("TYPES ARE THE SAME!\n");
                    } else { //type check is incorrect
                        printf("SEMANTIC ERROR\n");
                        fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                    }
                    type_flag = 0;
                } else {
                    if (method_count > 0){
                        temp_func[0][tfunc_count] = currScope[val][0];
                        tfunc_count++;
                        if (my_strcmp(currScope[0][func_count], currScope[val][1]) == 0){
                            printf("FUNCTION TYPES MATCH WITH INDEX %d\n", func_count);
                        } else {
                            printf("SEMANTIC ERROR\n");
                            fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                        }
                        
                    }
                }
            }
            
        }

        if (my_strcmp(currScope[val][2], "1") == 0){ //if it is a function
            func_flag++;
            method_count = my_atoi(currScope[0][3]);
        }
        if (local_flag == 1){
            printf("REQ = %s\n", otherScope[val][2]);
            printf("LOCAL FLAG = %d\n", local_flag);
            if (my_strcmp(otherScope[val][2], "1") == 0){
                func_flag++;
                method_count = my_atoi(otherScope[0][3]);
            }
        }

    }

    return 0;
}

int search(char* term, int s){
   
    int l1 = 0;
    if (s == 1){
        int size = sizeof(terminals) / sizeof(terminals[0]);
        l1 = 8; //for identifier
        for (int i = 0;i<size;i++){
            if (my_strcmp(terminals[0][i], term) == 0){
                l1 = i;
                break;
            }
            if (i < 2){
                if (my_strcmp(term, number[0][i]) == 0){
                    l1 = 26;
                    break;
                } else if (my_strcmp(term, operators[0][i]) == 0){
                    l1 = 30;
                    break;
                }
            } else if (i < 6) {
                if (my_strcmp(term,operators[0][i]) == 0){
                    l1 = 30;
                    break;
                }
            }
        }
        
    } else {
        int size = sizeof(nonterminals) / sizeof(nonterminals[0]);
        for (int i = 0;i<size;i++){
            if (my_strcmp(nonterminals[0][i], term) == 0){
                l1 = i;
                break;
            }
        }


    }
    return l1;
}

ASTNode* create(ASTNode* node, int x, int y){

    int count = 0;
    //creates the initial node
    node->name = LHS[x][y];
    node->type = 1;
    node->searched = 1;
    char str_copy[100];
    my_strcpy(str_copy, RHS[x][y]);
    //printf("%s\n", str_copy);

    //splits it up by spaces, and creates new nodes for all of its children
    char* str = my_atok(str_copy, " ");
    while (str != NULL){
        ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
        n->name = (char*)malloc(my_strlen(str)+1);
        my_strcpy(n->name, str);
        
        n->searched = 0;
        //printf("%s\n", n->name);
        int i = search(n->name, 1);

        if ((i == 8 && my_strcmp(n->name,headers[0][0]) == 0) || (i == 26 && my_strcmp(n->name, headers[0][1]) == 0) ||
         (i == 30 && my_strcmp(n->name, headers[0][2]) == 0) || my_strcmp(n->name,"#") == 0){ //terminal found
            //printf("True, %s", headers[0][0]);
            n->type = 0;
        } else if (i > 0 && i != 8 && i != 26 && i != 30){
            n->type = 0;
        } else {
            n->type = 1;
        }
        node->children[count] = n;
        count++;
        
        str = my_atok(NULL, " ");
    }
    node->total_children = count;
    

    return node;

}
int recursionyay(ASTNode* root, int curr, int temp){
    int errrrr = 0;
    /*if M1 then check from the table */

    if (x >= lineCount){
        return 1;
    }
    //printf("\nCURRENT ROOT: %s, %d\n", root->name, curr);
    if (root->children[curr]->searched == 0 && root->children[curr]->type == 1){ //if the leftmost child node hasn't been searched and is a non-terminal
        if (temp == 1){
            l1x = search(root->name, 0);
        } else {
        //continue parsing via DFS
            l1x = search(root->children[curr]->name, 0);
        }
        //printf("NON_TERMINAL\n");
        //printf("%s, %d, %d\n", root->name, l1x, l1y);
        
        if (ltable[l1x][l1y] == 0){ //ERROR (will get cauught later): PANIC MODE: POP OUT ELEMENT AND MOVE TO NEXT ITEM IN EQUATION
            //printf("\nCURRENT ROOT: %s, %d\n", root->name, curr);
            return 1;
        }
       recursionyay(create(root->children[curr], 0, ltable[l1x][l1y]-1), 0, temp);

    } else if (root->children[curr]->type == 0) { //if it is a terminal (or epsilon)

        //printf("%s, %s, %d\n", root->children[curr]->name, term, l1y);
        if (my_strcmp(root->children[curr]->name, term) == 0){
            printf("\nMATCH!, %s\n", term);

            if (my_strcmp(term, "def") == 0){ //if the token-lexeme pair is a new definition
                temp_s[0][2] = "1";
                temp_s[0][3] = "0";
                
                definition = 1;
                


            }

            else if (my_strcmp(term, "int") == 0){ //if the token-lexeme pair is an integer
                
                temp_s[0][1] = "int";
                declaration = 1;
            }

            else if (my_strcmp(term, "double") == 0){ //if the token-lexeme pair is a double
                temp_s[0][1] = "double";
                declaration = 1;
            }

            else if (my_strcmp(term, "ID") == 0){ //if the token-lexeme pair is an identifier
                temp_s[0][0] = tokenpair[x][1];
                
                if (!isdigit(temp_s[0][0][0])){ //if the first digit of the identifier is not a digit (painful, but needed)
                    if (my_strcmp(temp_s[0][2], "-1") == 0){
                        temp_s[0][2] = "0";
                    }
                    for (int i = 0;i<6;i++){
                        printf("%s, ", temp_s[0][i]);
                    }
                    printf("\n");
                    int val = symboltable(temp_s);
                    temp_s[0][0] = "-1";
                    for (int i = 2;i<6;i++){
                        temp_s[0][i] = "-1";
                    }
                } else {
                    int i = 0;
                    while (tokenpair[x][0][i]){
                        tokenpair[x][0][i] = tolower(tokenpair[x][0][i]);
                        i++;
                    }
                                
                    //check to see if it needs to be type checked
                    if (type_flag == 0 && func_flag == 0){ //if the flag is off (first identifier that needs to be type checked)
                        if (my_strcmp(tokenpair[x][0], "int") == 0){ //if it is just a integer
                            type_check[0][0] = "int";

                        } else { //else, if it is a double
                            type_check[0][0] = "double";
                        }
                        type_flag = 1;
                    } else if (type_flag == 1 && func_flag == 0){ //if the flag is on (second identifier that needs to be type checked)
                        if (my_strcmp(tokenpair[x][0], type_check[0][0]) == 0){ //type check is correct
                            printf("TYPES ARE THE SAME!\n");
                        } else { //type check is incorrect
                            printf("SEMANTIC ERROR\n");
                            fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                        }
                        type_flag = 0;
                        
                    } else if (func_flag > 0){
                        if (method_count > 0){
                            temp_func[0][tfunc_count] = tokenpair[x][1];
                            tfunc_count++;
                            if (my_strcmp(currScope[0][func_count], tokenpair[x][0]) == 0){
                                printf("FUNCTION TYPES MATCH WITH INDEX %d\n", func_count);
                            } else {
                                printf("SEMANTIC ERROR");
                                fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                            }
                            
                        }
                        
                    }
                    if (inter_flag == 1){
                        temp_int[0][inter_count] = tokenpair[x][1];
                        inter_count++;
                        printf("NEW COUNTER 2 = %d\n", inter_count);
                    }
                    
                }
                
            }

            else if ((my_strcmp(term, "+") == 0) || (my_strcmp(term, "-") == 0)  
            || (my_strcmp(term, "*") == 0) || (my_strcmp(term, "/") == 0) 
            || (my_strcmp(term, "%") == 0) ) { //if there is a comparison value, override so it gets type checked with intial variable (hardcoded cause I'm lazy)
                type_flag = 1;
                temp_int[0][inter_count] = term;
                inter_count++;
                printf("NEW COUNTER 3 = %d\n", inter_count);
            }
            else if (my_strcmp(term, ")") == 0){
                if (definition == 2){
                    definition = 0;
                    
                    my_itoa(num, test[currTestVal], 10);
                    printf("%d", num);
                    otherScope[x_temp][3] = test[currTestVal];
                    currTestVal++;
                    //my_itoa(num, otherScope[x_temp][3], 10);
                    x_temp++;
                    num = 0;
                    
                    for (int i = 0;i<7;i++){
                        otherScope[x_temp][i] = "-1";
                    }

                }
                declaration = 0;
                if (method_count == 1 && open == 0){
                    if (store_count > 0){
                        store_count--;
                        method_count = method_store[0][store_count];
                        func_count = index_store[0][store_count];
                        
                    } else {
                        method_count--;
                        func_count = 4;
                        func_flag--;
                        
                    }
                } else if (open > 0){
                    open--;
                }
                local_flag = 0;
            }

            else if (my_strcmp(term, ";") == 0){
                declaration = 0;
            }

            else if (my_strcmp(term, ",") == 0){
                if (method_count > 0){
                    method_count--;
                    func_count++;
                }
                open = 0;
            }

            else if (my_strcmp(term, "=") == 0){
                if (inter_flag == 1){
                    temp_int[0][inter_count] = tokenpair[x][1];
                    inter_count++;
                    printf("NEW COUNTER 4 = %d\n", inter_count);
                }
            }
            else if (my_strcmp(term, "(") == 0){
                open++;
            }

            else if (my_strcmp(term, "fed") == 0){ //if this is the end of the method (erase local scope)
                printf("LOCAL SCOPE RESULTS:\n");
                for (int i = 0;i<x_sem;i++){
                    printf("%s, %s\n", currScope[i][0], currScope[i][1]);
                }
                //erase the local scope
                for (int i = 0;i<10;i++){
                    for (int j = 0;j<7;j++){
                        currScope[i][j] = "-1";
                    }
                } 
                
                fprintf(inter, "pop FP\n");
                fprintf(inter, "pop PC\n");
                fprintf(inter, "main:\n");
                arg_count = temp_arg_count;
                //flip the current scope back to the global scope
                char ***a = otherScope;
                otherScope = currScope;
                currScope = a;
                x_sem = 0;
                //flips the current x value to the global x value
                printf("FLIP\n");
                int b = x_sem;
                x_sem = x_temp;
                x_temp = b;
            }

            else if (my_strcmp(term, "print") == 0){
                pri_flag = 1;
                inter_flag = 1;
            }
            else if (my_strcmp(term, "return") == 0){
                //check to see if it needs to be type checked
                ret_flag = 1;
                if (type_flag == 0){ //if the flag is off (first identifier that needs to be type checked)
                    if (my_strcmp(otherScope[x_temp-1][1], "int") == 0){ //if it is just a integer
                        type_check[0][0] = "int";
                    } else { //else, if it is a double
                        type_check[0][0] = "double";
                        
                    }
                    type_flag = 1;
                } else { //if the flag is on (second identifier that needs to be type checked)
                    if (my_strcmp(currScope[x_temp-1][1], type_check[0][0]) == 0){ //type check is correct
                        printf("TYPES ARE THE SAME!\n");
                    } else { //type check is incorrect
                        printf("SEMANTIC ERROR\n");
                        fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                    }
                    type_flag = 0;
                    
                    
                }
                inter_flag = 1;
            }
            else if (my_strcmp(term, "if") == 0){
                comp_flag = 1; //represents if statement
                
                fi_dup++;
            }

            else if (my_strcmp(term, "then") == 0){
                
                //printf("CAYY = %d", inter_count);
                if (inter_count == 3){
                    fprintf(inter, "comp (%s %s %s)\n", tokenpair[x-4][1], tokenpair[x-3][1], tokenpair[x-2][1]);
                }
                
                fprintf(inter, "beq label%d\n", label_counter);
                label_counter++;
                fprintf(inter, "b label%d\n", label_counter);
                label_counter++;
                fprintf(inter, "label%d:\n", label_counter-2);
            }

            else if (my_strcmp(term, "else") == 0){
                el_flag = 2;
                if (my_strcmp(tokenpair[x+1][1], "if") == 0){
                    fprintf(inter, "b label%d\n", label_counter+2);
                } else {
                    fprintf(inter, "b label%d\n", label_counter);
                    el_flag = 1; //flag for whether it is an if-else statement
                    
                }
                fprintf(inter, "label%d:\n", label_counter-1);
               
                
                
            }

            else if (my_strcmp(term, "fi") == 0){
                
                if (el_flag == 1){
                    fprintf(inter, "label%d:\n", label_counter);
                    label_counter++;
                } else {
                    if (fi_dup > 1 || el_flag == 0){
                        fprintf(inter, "label%d:\n", label_counter-1);
                    } else if (el_flag == 2) {
                        fprintf(inter, "label%d:\n", label_counter);
                        label_counter++;
                    }
                }
                comp_flag = 0;
                fi_dup--;
                if (fi_dup == 0){
                    el_flag = 0;
                }
                //printf("YAGG = %d", inter_count);
            }

            else if (my_strcmp(term, "while") == 0){
                
                comp_flag = 2;
            }

            else if (my_strcmp(term, "do") == 0){
                fprintf(inter, "label%d:\n", label_counter);
                label_counter++;
                
                //printf("CAYY = %d", inter_count);
                if (inter_count == 3){
                    fprintf(inter, "comp (%s %s %s)\n", tokenpair[x-4][1], tokenpair[x-3][1], tokenpair[x-2][1]);
                }
                
                
                fprintf(inter, "bne label%d\n", label_counter);
                label_counter++;
            }

            else if (my_strcmp(term, "COMP") == 0){
                temp_int[0][inter_count] = term;
                inter_count++;
                printf("NEW COUNTER 5 = %d\n", inter_count);
            }


            else if (my_strcmp(term, "od") == 0){
                fprintf(inter, "b label%d\n", label_counter-2);
                fprintf(inter, "label%d:\n", label_counter-1);
            }

            x++;
            term = tokenpair[x][1];
            curr_id = term;

            if (inter_flag == 1 && !(my_strcmp(tokenpair[x-1][1], "then") == 0) && !(my_strcmp(tokenpair[x-1][1], "do") == 0) && !(my_strcmp(tokenpair[x-1][1], "else") == 0)){
                //printf("%d, %d type shit\n", inter_line, my_atoi(tokenpair[x][2]));
                if (inter_line != my_atoi(tokenpair[x][2])){ //this is where we have to print the expression into the intermediate code
                    int mid = (inter_count) / 2;
                    //printf("MIDPOINT MOMENT = %d\n", mid);
                    //printf("INTER COUNT = %d\n", inter_count);
                    for (int i = 0;i<10;i++){
                        printf("%s, ", temp_int[0][i]);
                    }
                    inter_flag = 0;
                    
                    if (tfunc_count > 0){
                        int d = 1;
                        while (tfunc_count > 0){

                            fprintf(inter, "push %s\n", temp_func[0][tfunc_count-d]);
                            if ((tfunc_count - d) <= 0){
                                break;
                            }
                            d++;
                        }
                        if (local_flag == 1 || switcher == 1){
                            fprintf(inter, "bal %s\n", otherScope[0][0]);
                        } else {
                            fprintf(inter, "bal %s\n", currScope[0][0]);
                        }
                        for (int i = 0;i<tfunc_count;i++){
                            fprintf(inter, "pop %s\n", temp_func[0][i]);
                        }
                        if (local_flag == 1 || switcher == 1){
                            fprintf(inter, "%s = %s\n", temp_int[0][0], otherScope[0][0]);
                            switcher = 0;
                        } else {
                            fprintf(inter, "%s = %s\n", temp_int[0][0], currScope[0][0]);
                        }
                        tfunc_count = 0;
                    } else
                    if (my_strcmp(temp_int[0][mid], "=") == 0){
                        fprintf(inter, "%s = %s\n", temp_int[0][mid-1], temp_int[0][mid+1]);
                    }
                    else if (!((my_strcmp(temp_int[0][mid], "+") == 0) && (my_strcmp(temp_int[0][mid], "-") == 0)  
                    && (my_strcmp(temp_int[0][mid], "*") == 0) && (my_strcmp(temp_int[0][mid], "/") == 0) 
                    && (my_strcmp(temp_int[0][mid], "%") == 0)) && inter_count == 5 ) {
                        fprintf(inter, "%s = %s %s %s\n", temp_int[0][mid-2], temp_int[0][mid], temp_int[0][mid+1], temp_int[0][mid+2]);
                    }
                    else if ((my_strcmp(temp_int[0][mid], "+") == 0) || (my_strcmp(temp_int[0][mid], "-") == 0)  
                    || (my_strcmp(temp_int[0][mid], "*") == 0) || (my_strcmp(temp_int[0][mid], "/") == 0) 
                    || (my_strcmp(temp_int[0][mid], "%") == 0) ) {
                        mid++;
                        int diff = 0;
                        while (inter_flag == 0){
                            if (diff == 0){
                                if (my_strcmp(temp_int[0][mid], "=") == 0){
                                    break;
                                }
                                fprintf(inter, "t%d = %s\n", term_counter, temp_int[0][mid]);
                                term_counter++;
                                diff++;
                            } else {
                                printf("INTER COUNT = %d\n", inter_count);
                                if ((mid + diff) >= (inter_count - 1)){
                                    break;
                                }
                                fprintf(inter, "t%d = t%d %s %s\n", term_counter, term_counter-1, temp_int[0][mid+diff], temp_int[0][mid+(diff+1)]);
                                term_counter++;

                                if (my_strcmp(temp_int[0][mid-diff], "=") == 0){
                                    break;
                                }

                                fprintf(inter, "t%d = %s %s t%d\n", term_counter, temp_int[0][mid-(diff+1)], temp_int[0][mid-diff], term_counter-1);
                                term_counter++;
                                diff+=2;
                            }
                        }
                        fprintf(inter, "%s = t%d\n", temp_int[0][0], term_counter-1);
                    } else {
                        if (ret_flag == 1){
                            ret_flag = 2;
                            if (my_strcmp(term, "fed") == 0){
                                //printf("COUNTTTT = %d\n", inter_count);
                                if (inter_count == 1){
                                    fprintf(inter, "return %s\n", temp_int[0][0]);
                                } else if (inter_count == 3){
                                    fprintf(inter, "return %s %s %s\n", temp_int[0][0], temp_int[0][1], temp_int[0][2]);
                                }
                                ret_flag = 0;
                            }  
                        } else
                        if (ret_flag == 2){
                            if (inter_count == 1){
                                fprintf(inter, "return %s\n", temp_int[0][0]);
                            } else if (inter_count == 3){
                                fprintf(inter, "return %s %s %s\n", temp_int[0][0], temp_int[0][1], temp_int[0][2]);
                            }
                            ret_flag = 0;
                        }else
                            if (pri_flag == 1){
                                pri_flag = 2;
                                if (my_strcmp(term, "fed") == 0){
                                    //printf("COUNTTTT = %d\n", inter_count);
                                    if (inter_count == 1){
                                        fprintf(inter, "print %s\n", temp_int[0][0]);
                                    } else if (inter_count == 3){
                                        fprintf(inter, "print %s %s %s\n", temp_int[0][0], temp_int[0][1], temp_int[0][2]);
                                    }
                                    pri_flag = 0;
                                }  
                            } else
                            if (pri_flag == 2){
                                if (inter_count == 1){
                                    fprintf(inter, "print %s\n", temp_int[0][0]);
                                } else if (inter_count == 3){
                                    fprintf(inter, "print %s %s %s\n", temp_int[0][0], temp_int[0][1], temp_int[0][2]);
                                }
                                pri_flag = 0;   
                        } else {
                            int diff = 0;
                            while (inter_flag == 0){
                                if (diff == 0){
                                    if (my_strcmp(temp_int[0][mid], "=") == 0){
                                        break;
                                    }
                                    fprintf(inter, "t%d = %s\n", term_counter, temp_int[0][mid]);
                                    term_counter++;
                                    diff++;
                                } else {
                                    printf("INTER COUNT = %d\n", inter_count);
                                    if ((mid + diff) >= (inter_count - 1)){
                                        break;
                                    }
                                    fprintf(inter, "t%d = t%d %s %s\n", term_counter, term_counter-1, temp_int[0][mid+diff], temp_int[0][mid+(diff+1)]);
                                    term_counter++;

                                    if (my_strcmp(temp_int[0][mid-diff], "=") == 0){
                                        break;
                                    }

                                    fprintf(inter, "t%d = %s %s t%d\n", term_counter, temp_int[0][mid-(diff+1)], temp_int[0][mid-diff], term_counter-1);
                                    term_counter++;
                                    diff+=2;
                                }
                            }
                            
                            fprintf(inter, "%s = t%d\n", temp_int[0][0], term_counter-1);
                            
                        }
                    }
                    inter_count = 0;
                    inter_line = -1;
                    for (int i = 0;i<20;i++){
                        temp_int[0][i] = "-1";
                    }
                   
                }
            }
            if (term == NULL){
                //printf("\nCURRENT ROOT: %s, %d\n", root->name, curr);
                return 1;
            }
            l1y = search(term, 1);
            if (l1y == 8){
                term = "ID";
            } else if (l1y == 26){
                term = "NUMBER";
            } else if (l1y == 30){
                term = "COMP";
            }
            //printf("%s\n", term);
            //printf("%d\n", l1y);
            //printf("Line number: %d\n", x);
            
            
        } else if (my_strcmp(root->children[curr]->name, "#") == 0){ //if the comparison is epsilon
            return 0; //immediately return

        } else if (my_strcmp(root->children[curr]->name, "#") != 0) { //If comparison doesn't match (error)
           //print that there is an error in the program, move on to next item of the equation
            temp = 1;
            //fprintf(error, "error on line %s\n", tokenpair[x][2]);
            //printf("ERROR\n");
            
            err++;
            // x++;
            // term = tokenpair[x][1];
            // curr_id = term;
            // if (term == NULL){
            //     printf("\nCURRENT ROOT: %s, %d\n", root->name, curr);
            //     return 0;
            // }
            // l1y = search(term, 1);
            
            // if (l1y == 8){
            //     term = "ID";
            // } else if (l1y == 26){
            //     term = "NUMBER";
            // } else if (l1y == 30){
            //     term = "COMP";
            // }
            
            // if (errrrr == 0)
            // {
            //     errrrr = 1;
            //     goto verybadprogramming;
            // }

            //printf("ltable = %d, %d\n", l1x, l1y);
            if (ltable[l1x][l1y] == 0){
                //printf("\nCURRENT ROOT: %s, %d\n", root->name, curr);
                return 1;
            }
            
        }

    }
    //printf("Current node: %s\n", root->name);
    //printf("total children = %d, current = %d\n", root->total_children, curr);
    if (root->total_children <= curr+1){
        //printf("indeed, it does\n");
        //printf("\nCURRENT ROOT: %s, %d\n", root->name, curr);
        return 1;
    }
    
    curr++;
    recursionyay(root, curr, temp);
    //printf("\nCURRENT ROOT: %s, %d\n", root->name, curr);

}

int semantic() {

    
    error = fopen("error.txt", "w");
    tokens = fopen("token_file.txt", "r");
    ll1 = fopen("ll1.txt", "r");
    inter = fopen("intermediate.txt", "w");

    
    char line[256];
    char *temp[50];

    type_check[0][0] = "-1";
    local_flag = 0;
    
    curr_type = (char*)malloc(sizeof(char));

    while (fgets(line, sizeof(line), tokens)){
        char* string = my_atok(line, separator);
        int tokenCount = 0;
        while (string != NULL){
            tokenpair[lineCount][tokenCount] = (char*)malloc(my_strlen(string) + 1);
            my_strcpy(tokenpair[lineCount][tokenCount], string);
            tokenCount++;
            string = my_atok(NULL, separator);
            //printf("%s\n", tokenpair[lineCount][tokenCount-1]);
            
        }

        lineCount++;
        
    }
    

    for (int i=0;i<37;i++){
        for (int j = 0;j<33;j++){
            fscanf(ll1, "\t%d", &ltable[i][j]);
            //printf("%d, ", ltable[i][j]);
        }
        //printf("\n");
    }
    if (NULL == ltable){
        printf("File can't be opened \n");
        return -1;
    }
    
    //gets the first token found in token-lexeme pairs
    term = tokenpair[x][1];
    curr_id = term;
    //printf("%s", term);
    //searches all terminals to see where it is in LL1 table
    l1y = search(term, 1);
    //printf("%d", l1y);
    //starts to create whole AST
    
    //create the root node of the AST (Program)
    ASTNode *root = (ASTNode*)malloc(sizeof(ASTNode));
    root = create(root, 0, 0);

    
    for (int i = 0;i<10;i++){
        currScope[i] = (char**)malloc(7 * sizeof(char*));
        otherScope[i] = (char**)malloc(7 * sizeof(char*));
    }
    temp_s[0][0] = "-1";
    for (int i = 1;i<6;i++){
        temp_s[0][i] = "-1";
    }
    
    recursionyay(root, 0, 0);

    printf("GLOBAL SCOPE RESULTS:\n");
    for (int i = 0;i<x_sem;i++){
        printf("%s, %s, %s, %s, %s, %s\n", currScope[i][0],currScope[i][1],currScope[i][2],currScope[i][3],currScope[i][4],currScope[i][5]);
    }
    if (err == 0){
       //printf("Congratualations!  There were no errors in this code!\n");

    } else {
        //printf("Sorry, there were %d error(s) in this code.\n", err);
    }
    
    //printf("SYMBOLS\n");
    while (symbols != NULL){
        //printf("%s\n", symbols->data);
        symbols = symbols->child;
    }
    free(curr_type);

    return 0;

}

int main(){
    semantic();
    printf("\nDone!");
    return 0;
}