// Written by Rahul Chakrabarti
/* This code ensures scope and type legality adherence for EZ-Sharp.
    - A symbol table is implemented for smooth and efficient functionality.
    - Symbol data structures via nodes are initialized to create an instance of a symbol table.
*/
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cctype>

FILE* ll1;
FILE* tokens; //token lexeme pairs from phase 1
FILE* error; //error log file

char *tokenpair[500][3];
char *separator = "|"; //global variable for separator terminal
char *space = " ";
int x = 0;
char* term = " ";
int lineCount = 0;

// added a ton of new stuff here sepcifically for phase 3
char* type_check[1][7];
int type_flag = 0;
int func_flag = 0;
int func_count = 4;
int method_count = 0;
int local_flag = 0;
int open = 0;
int l1y = 0;
int l1x = 0;
int err = 0;
char* curr_type;
char* curr_id;
int symbol_count = 0;
char *temp_s[5][6];

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
char ***currScope = (char***) global; // triple pointer done bc it points to a pointer of a 2d array
char ***otherScope = (char***) local; // ''
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

// data structure for node in AST
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

SymbolNode* createSymbol(char *data[1][3]){
    SymbolNode* newNode = (SymbolNode*)malloc(sizeof(SymbolNode)); // change malloc to new because of memory
    newNode->child = NULL; 
    newNode->parent = NULL;
    newNode->data[symbol_count][0] = data[0][0];
    //std::cout << "" << newNode->data[symbol_count][0] << "" << std::endl;
    newNode->data[symbol_count][1] = data[0][1];
    newNode->data[symbol_count][2] = data[0][2];
    symbol_count++;
    return newNode;
}

// method to search the symbol table(s) to see if the identifier exists in the table
int semantic_search(){ 
    if ((void*)currScope == (void*)global){ //if the current pointer is the global symbol table //fix later idk what im doign 
        std::cout << "GLOBAL" << std::endl;
        printf("%d", x_sem);
        for (int i = 0;i<x_sem;i++){
            if (strcmp(currScope[i][0], currScope[x_sem][0]) == 0){
                return i; // updated for readbility
            }
        }
        return -1;
    } else { //else if it is the local symbol table
        std::cout << "LOCAL" << std::endl;
        for (int i = 0;i<x_sem;i++){
            if (strcmp(currScope[i][0], currScope[x_sem][0]) == 0){
                return i;
            }
        }
        // if it doesn't exist in local symbol table, check global symbol table
        for (int i = 0;i<x_temp;i++){
            if (strcmp(otherScope[i][0], currScope[x_sem][0]) == 0){
                local_flag = 1;
                return i;
            }
        } // ALL OFTHIS ABOVE IS BASED ON PSUEDOCODE ESQUE DESCRIPTION 
        return -1;
    }
}
//symbol table creation
int symboltable(char *temp_s[1][6]){
    currScope[x_sem][0] = temp_s[0][0];
    int val = semantic_search(); // use search above for identifer to be found
    printf("SEARCH FOR %s = %d\n", currScope[x_sem][0], val);
    printf("Definition = %d, declaration = %d\n", definition, declaration);

    if (definition == 1){ //if this identifier is a definition
        if (val >= 0){ // Function declared twice or morez
            std::cout << "ERROR: def = 1";
            fprintf(error, "scoping error on line %s\n", tokenpair[x][2]);
            return 0;
        }
        // Fwips the current scope to local scope
        for (int i = 1;i<6;i++){
            currScope[x_sem][i] = temp_s[0][i];
        }
        
        char ***a = otherScope;
        otherScope = currScope;
        currScope = a;
        // fwips the current x value to the local x value
        // std::cout << "FLIP" << std::endl;
        int b = x_sem;
        x_sem = x_temp;
        x_temp = b;

        definition = 2;
        return 0;
    } else if (definition == 2){
        if (val >= 0){ //ERROR: REDECLARATION
            std::cout << "ERROR: def done = 2";
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
        
        if (strcmp(currScope[x_sem][1], "int") == 0){
            otherScope[x_temp][def_count] = "int";
        } else {
            otherScope[x_temp][def_count] = "double";
        }
        def_count++;
        x_sem++;
        num++;
    } else if (declaration == 1){ // if this identifier is a new declaration
        if (val >= 0){ // THROW ERROR: REDECLARATION OF A VARIABLE
            std::cout << "ERROR: decl = 1";
            fprintf(error, "scoping error on line %s\n", tokenpair[x][2]);
            return 0;
        }
        for (int i = 1;i<6;i++){
            currScope[x_sem][i] = temp_s[0][i];
        }
        x_sem++;
    } else { //if this identifier is not a declaration or a definition
        if (val == -1){ //ERROR: VARIABLE IS NOT DECLARED
            std::cout << "ERROR: decl = 0";
            fprintf(error, "scoping error on line %s\n", tokenpair[x][2]);
            return 0;
        }
        //check to see if it needs to be type checked
        if (type_flag == 0 && func_flag == 0){ //if the flag is off (first identifier that needs to be type checked)
            if (strcmp(currScope[val][1], "int") == 0){ //if it is just a integer
                type_check[0][0] = "int";

            } else { //else, if it is a double
                type_check[0][0] = "double";
            }
            type_flag = 1;
        } else if (type_flag == 1 && func_flag == 0){ //if the flag is on (second identifier that needs to be type checked)
            if (local_flag == 1){
                if (strcmp(otherScope[val][1], type_check[0][0]) == 0){
                    std::cout << "TYPES ARE THE SAME!!" << std::endl;
                } else {
                    std::cout << "SEMANTIC ERROR" << std::endl;
                    fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                }
            } else {
                if (strcmp(currScope[val][1], type_check[0][0]) == 0){ //type check is correct
                    std::cout << "TYPES ARE THE SAME!" << std::endl;
                } else { //type check is incorrect
                    std::cout << "SEMANTIC ERROR" << std::endl;
                    fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                }
            }
            type_flag = 0;
        } else if (func_flag > 0){
            if (local_flag == 1){
                std::cout << "REQ = " << otherScope[val][2] << "" << std::endl;
                std::cout << "LOCAL FLAG = " << local_flag << "" << std::endl;
                if (strcmp(otherScope[val][2], "1") == 0){
                    func_flag++;
                    method_store[0][store_count] = method_count;
                    index_store[0][store_count] = func_count;
                    method_count = atoi(otherScope[0][3]);
                    store_count++;
                    func_count = 4;
                    if (strcmp(otherScope[val][1], type_check[0][0]) == 0){
                        std::cout << "TYPES ARE THE SAME!!" << std::endl;
                    } else {
                        std::cout << "SEMANTIC ERROR" << std::endl;
                        fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                    }
                    type_flag = 0;
                } else {
                    if (method_count > 0){
                        if (strcmp(otherScope[0][func_count], currScope[val][1]) == 0){
                            std::cout << "FUNCTION TYPES MATCH WITH INDEX " << func_count << "" << std::endl;
                        } else {
                            std::cout << "SEMANTIC ERROR DETECTED" << std::endl;
                            fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                        }
                    }
                }
            } else {
                if (strcmp(currScope[val][2], "1") == 0){ //if it is a function
                    func_flag++;
                    method_store[0][store_count] = method_count;
                    index_store[0][store_count] = func_count;
                    method_count = atoi(currScope[0][3]);
                    store_count++;
                    func_count = 4;
                    if (strcmp(currScope[val][1], type_check[0][0]) == 0){ //type check is correct
                        std::cout << "TYPES ARE THE SAME!" << std::endl;
                    } else { //type check is incorrect
                        std::cout << "SEMANTIC ERROR DETECTED" << std::endl;
                        fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                    }
                    type_flag = 0;
                } else {
                    if (method_count > 0){
                        if (strcmp(currScope[0][func_count], currScope[val][1]) == 0){
                            std::cout << "FUNCTION TYPES MATCH WITH INDEX " << func_count << "" << std::endl;
                        } else {
                            std::cout << "SEMANTIC ERROR" << std::endl;
                            fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                        }   
                    }
                }
            }   
        }
        if (strcmp(currScope[val][2], "1") == 0){ //if it is a function
            func_flag++;
            method_count = atoi(currScope[0][3]);
        }
        if (local_flag == 1){
            std::cout << "REQ = " << otherScope[val][2] << "" << std::endl;
            std::cout << "LOCAL FLAG = " << local_flag << "" << std::endl;
            if (strcmp(otherScope[val][2], "1") == 0){
                func_flag++;
                method_count = atoi(otherScope[0][3]);
            }
        }
    }
    return 0;
}
int search(char* term, int s){  // Looks for identifier in grammar to resolve token,.
    int l1 = 0;
    if (s == 1){
        // int size = sizeof(terminals) / sizeof(terminals[0]); 
        // CHECK FOR NON NULL FOR ALL ELEMENTS IN FIRST AND FOLLOW THAT ARE NULLALBLE
        l1 = 8; // fro identifier identification
        for (int i = 0;i < 34; i++){ // size 34 since only 34 strings (make CONSTANT)
            if (terminals[0][i] == term && strcmp(terminals[0][i], term) == 0){ //first == term means non null
                l1 = i;
                break;
            }
            if (i < 2){ // switch tables terminal and operators for setup of later stage
                if (terminals[0][i] == term && strcmp(term, number[0][i]) == 0){
                    l1 = 26;
                    break;
                } else if (operators[0][i] == term && strcmp(term, operators[0][i]) == 0){
                    l1 = 30;
                    break;
                }
            } else if (i < 6) { // as one traverses the terminal table
                if (operators[0][i] == term && strcmp(term,operators[0][i]) == 0){
                    l1 = 30;
                    break;
                }
            }
        }
    } else { // all else fails nonterminals (for further derivation to terminate)
        int size = sizeof(nonterminals) / sizeof(nonterminals[0]);
        for (int i = 0;i < size;i++){
            if (nonterminals[0][1] == term && strcmp(nonterminals[0][i], term) == 0){
                l1 = i;
                break;
            }
        }
    }
    return l1;
}
ASTNode* create(ASTNode* node, int x, int y){ // substantially updatd node creation
    int count = 0;
    //creates the initial node
    node->name = LHS[x][y];
    node->type = 1;
    node->searched = 1;
    char str_copy[100];
    strcpy(str_copy, RHS[x][y]);
    //std::cout << "" << str_copy << "" << std::endl;
    //splits it up by spaces, and creates new nodes for all of its children
    char* str = strtok(str_copy, " ");
    while (str != NULL){
        ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
        n->name = (char*)malloc(strlen(str)+1);
        strcpy(n->name, str);
        n->searched = 0;
        //std::cout << "" << n->name << "" << std::endl;
        int i = search(n->name, 1);
        if ((i == 8 && strcmp(n->name,headers[0][0]) == 0) || (i == 26 && strcmp(n->name, headers[0][1]) == 0) ||
         (i == 30 && strcmp(n->name, headers[0][2]) == 0) || strcmp(n->name,"#") == 0){ //terminal found
            //std::cout << "True, " << headers[0][0] << ""; // for deubugaing
            n->type = 0;
        } else if (i > 0 && i != 8 && i != 26 && i != 30){
            n->type = 0;
        } else {
            n->type = 1;
        }
        node->children[count] = n;
        count++;
        str = strtok(NULL, " ");
    }
    node->total_children = count;
    return node;
}
int recursion(ASTNode* root, int curr, int temp){ // Huge function. Done to resolve all symbols semantically (hopefully)
    int errrrr = 0;
    /*if M1 then check from the table */ // FROM UPDATED GRAMMAR
    if (x >= lineCount){
        return 1;
    }
    //std::cout << "\nCURRENT ROOT: " << root->name, curr << ", %d" << std::endl;
    if (root->children[curr]->searched == 0 && root->children[curr]->type == 1){ //if the leftmost child node hasn't been searched and is a non-terminal
        if (temp == 1){
            l1x = search(root->name, 0);
        } else {
        // DFS sarching
            l1x = search(root->children[curr]->name, 0);
        }
        //std::cout << "NON_TERMINAL" << std::endl;
        //std::cout << "" << root->name, l1x, l1y << ", %d, %d" << std::endl;

        if (ltable[l1x][l1y] == 0){ //ERROR (will get caught later): PANIC MODE: POP OUT ELEMENT AND MOVE TO NEXT ITEM IN EQUATION
            // I move to next item by not terminating prog
            //std::cout << "\nCURRENT ROOT: " << root->name, curr << ", %d" << std::endl;
            return 1;
        }
        recursion(create(root->children[curr], 0, ltable[l1x][l1y]-1), 0, temp); // done if no resolution

    } else if (root->children[curr]->type == 0) { // if type is a terminal (or epsilon for empty)
        //std::cout << "" << root->children[curr]->name, term, l1y << ", %s, %d" << std::endl;
        if (strcmp(root->children[curr]->name, term) == 0){
            std::cout << "\nMATCH!, " << term << "" << std::endl;
            if (strcmp(term, "def") == 0){ //if the token-lexeme pair is a new definition
                temp_s[0][2] = "1";
                temp_s[0][3] = "0";
                definition = 1;
            }
            if (strcmp(term, "int") == 0){ //if the token-lexeme pair is an integer
                temp_s[0][1] = "int";
                declaration = 1;
            }
            if (strcmp(term, "double") == 0){ //if the token-lexeme pair is a double
                temp_s[0][1] = "double";
                declaration = 1;
            }
            if (strcmp(term, "ID") == 0){ //if the token-lexeme pair is an identifier
                temp_s[0][0] = tokenpair[x][1];
                if (!isdigit(temp_s[0][0][0])){ //if the first digit of the identifier is not a digit (painful, but needed)
                    if (strcmp(temp_s[0][2], "-1") == 0){
                        temp_s[0][2] = "0";
                    }
                    for (int i = 0;i<6;i++){ // if , char found
                        std::cout << "" << temp_s[0][i] << ", ";
                    }
                    std::cout << "" << std::endl;
                    int val = symboltable(temp_s);
                    temp_s[0][0] = "-1";
                    for (int i = 2;i<6;i++){
                        temp_s[0][i] = "-1";
                    }
                } else {
                    int i = 0;
                    while (tokenpair[x][0][i]){
                        tokenpair[x][0][i] = tolower(tokenpair[x][0][i]); // comparision
                        i++;
                    }
                    //check to see if it needs to be type checked
                    if (type_flag == 0 && func_flag == 0){ //if flag 0 (first identifier that needs to be type checked)
                        if (strcmp(tokenpair[x][0], "int") == 0){ // if it is just a integer
                            type_check[0][0] = "int";
                        } else { // only else, if it is a double
                            type_check[0][0] = "double";
                        }
                        type_flag = 1;
                    } else if (type_flag == 1 && func_flag == 0){ //if the flag is on (second identifier that needs to be type checked)
                        if (strcmp(tokenpair[x][0], type_check[0][0]) == 0){ //type check is correct
                            std::cout << "TYPES ARE THE SAME!" << std::endl;
                        } else { //type check is incorrect
                            std::cout << "SEMANTIC ERROR" << std::endl;
                            fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                        }
                        type_flag = 0;
                    } else if (func_flag > 0){
                        if (method_count > 0){
                            if (strcmp(currScope[0][func_count], tokenpair[x][0]) == 0){
                                std::cout << "FUNCTION TYPES MATCH WITH INDEX " << func_count << "" << std::endl;
                            } else {
                                std::cout << "SEMANTIC ERROR DETECTED";
                                fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                            }
                        }
                    }
                }
            }
            if ((strcmp(term, "+") == 0) || (strcmp(term, "-") == 0)  // COMPARE ALL PRIMITIVE OPS
            || (strcmp(term, "*") == 0) || (strcmp(term, "/") == 0) 
            || (strcmp(term, "%") == 0) ) { //if there is a comparison value, override so it gets type checked with intial variable (hardcoded cause I'm lazy)
                type_flag = 1;
            }
            if (strcmp(term, ")") == 0){
                if (definition == 2){
                    definition = 0;
                    itoa(num, test[currTestVal], 10);
                    printf("%d", num);
                    otherScope[x_temp][3] = test[currTestVal];
                    currTestVal++;
                    //itoa(num, otherScope[x_temp][3], 10);
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
                    open--; // HANDLE EXISTING OPEN OTHERWISE ERROR
                }
                local_flag = 0;
            }
            if (strcmp(term, ";") == 0){
                declaration = 0;
            }
            if (strcmp(term, ",") == 0){ 
                if (method_count > 0){
                    method_count--;
                    func_count++;
                }
                open = 0;
            }

            if (strcmp(term, "(") == 0){
                open++; // CLOSED IF MATCHING SYMBOL FOUND LATER !!!!
            }
            if (strcmp(term, "fed") == 0){ //if this is the end of the method (erase local scope)
                std::cout << "LOCAL SCOPE RESULTS:" << std::endl;
                for (int i = 0;i<x_sem;i++){
                    printf("%s, %s\n", currScope[i][0], currScope[i][1]);
                }
                //erase the local scope
                for (int i = 0;i<10;i++){
                    for (int j = 0;j<7;j++){
                        currScope[i][j] = "-1"; 
                    }
                } 
                // fwip the current scope back to the global scope
                // Done via swaps
                char ***a = otherScope;
                otherScope = currScope;
                currScope = a;
                x_sem = 0;
                // FLips the current x value to the global x value
                std::cout << "FLIP" << std::endl;
                int b = x_sem;
                x_sem = x_temp;
                x_temp = b;
            }
            if (strcmp(term, "return") == 0){
                //check to see if it needs to be type checked
                if (type_flag == 0){ //if the flag is off (first identifier that needs to be type checked)
                    if (strcmp(otherScope[x_temp-1][1], "int") == 0){ //if it is just a integer
                        type_check[0][0] = "int";
                    } else { //else, if it is a double
                        type_check[0][0] = "double";
                    }
                    type_flag = 1;
                } else { //if the flag is on (second identifier that needs to be type checked)
                    if (strcmp(currScope[x_temp-1][1], type_check[0][0]) == 0){ //type check is correct
                        std::cout << "TYPES ARE THE SAME!" << std::endl;
                    } else { //type check is incorrect
                        std::cout << "SEMANTIC ERROR" << std::endl;
                        fprintf(error, "type checking error on line %s\n", tokenpair[x][2]);
                    }
                    type_flag = 0;
                }
            }
            x++;
            term = tokenpair[x][1];
            curr_id = term;
            if (term == NULL){
                //std::cout << "\nCURRENT ROOT: " << root->name, curr << ", %d" << std::endl;
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
            //std::cout << "" << term << "" << std::endl;
            //std::cout << "" << l1y << "" << std::endl;
            //std::cout << "Line number: " << x << "" << std::endl;
        } else if (strcmp(root->children[curr]->name, "#") == 0){ //if the comparison is epsilon
            return 0; // end

        } else if (strcmp(root->children[curr]->name, "#") != 0) { //If comparison doesn't match (error)
           // IF is an error in the program, move on to next item of the equation )Panic maode)
            temp = 1;
            //fprintf(error, "error on line %s\n", tokenpair[x][2]);
            //std::cout << "ERROR" << std::endl;       
            err++;
            //std::cout << "ltable = " << l1x, l1y << ", %d" << std::endl;
            if (ltable[l1x][l1y] == 0){
                //std::cout << "\nCURRENT ROOT: " << root->name, curr << ", %d" << std::endl;
                return 1;
            }
        }
    }
    //std::cout << "Current node: " << root->name << "" << std::endl;
    //std::cout << "total children = " << root->total_children, curr << ", current = %d" << std::endl;
    if (root->total_children <= curr+1){
        //std::cout << "\nCURRENT ROOT: " << root->name, curr << ", %d" << std::endl;
        return 1;
    }
    curr++;
    recursion(root, curr, temp);
    //std::cout << "\nCURRENT ROOT: " << root->name, curr << ", %d" << std::endl;
}
int syntax() {
    // Basically main, reused and repurposed from phase 2.
    // Open file using native C
    error = fopen("error.txt", "w");
    tokens = fopen("token_file.txt", "r");
    ll1 = fopen("ll1.txt", "r");
    // below flags + vars used for token allocation
    char line[256];
    char *temp[50];
    type_check[0][0] = "-1";
    local_flag = 0;
    curr_type = (char*)malloc(sizeof(char));
    while (fgets(line, sizeof(line), tokens)){
        char* string = strtok(line, separator);
        int tokenCount = 0;
        while (string != NULL){
            tokenpair[lineCount][tokenCount] = (char*)malloc(strlen(string) + 1);
            strcpy(tokenpair[lineCount][tokenCount], string);
            tokenCount++;
            string = strtok(NULL, separator);
            //std::cout << "" << tokenpair[lineCount][tokenCount-1] << "" << std::endl;
        }
        lineCount++;
    }
    for (int i=0;i<37;i++){ // cjamged ll1 table from befoper phase 2
        for (int j = 0;j<33;j++){
            fscanf(ll1, "\t%d", &ltable[i][j]);
            //printf("%d, ", ltable[i][j]);
        }
        //std::cout << "" << std::endl;
    }
    if (NULL == ltable){
        std::cout << "ll1 can't be opened " << std::endl;
        return -1;
    }
    //gets the first token found in token-lexeme pairs
    term = tokenpair[x][1];
    curr_id = term;
    //std::cout << "" << term << "";
    //searches all terminals to see where it is in LL1 table
    l1y = search(term, 1);
    //printf("%d", l1y);
    //create the root node of the AST (Program)
    ASTNode *root = (ASTNode*)malloc(sizeof(ASTNode)); // same change malloc to new
    root = create(root, 0, 0);
    for (int i = 0;i<10;i++){
        currScope[i] = (char**)malloc(7 * sizeof(char*));
        otherScope[i] = (char**)malloc(7 * sizeof(char*));
    }
    temp_s[0][0] = "-1";
    for (int i = 1;i<6;i++){
        temp_s[0][i] = "-1";
    }
    recursion(root, 0, 0); // this is where the fun begins
    std::cout << "GLOBAL SCOPE RESULTS:" << std::endl;
    for (int i = 0;i<x_sem;i++){
        printf("%s, %s, %s, %s, %s, %s\n", currScope[i][0],currScope[i][1],currScope[i][2],currScope[i][3],currScope[i][4],currScope[i][5]);
    }
    if (err == 0){
        std::cout << "Congratulations!  This is error free code."; // for user

    } else {
        std::cout << "Sorry, there are " << err << " error(s) in this code." << std::endl;
    }
    //std::cout << "SYMBOLS" << std::endl;
    while (symbols != NULL){
        //std::cout << "" << symbols->data << "" << std::endl;
        symbols = symbols->child;
    }
    free(curr_type);
    return 0; // return program ptr to Main (otherwise there is a huge problem)
} // end syntax

int main(){
    syntax(); // begin flow
    printf("\nTerminated!");
    return 0;
} // end main