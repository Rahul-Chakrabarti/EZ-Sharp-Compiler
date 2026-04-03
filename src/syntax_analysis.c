/*CP471 Syntax Analysis Code (Phase 2)
By: Ethan Sadler
#210557460
*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

FILE* ll1;
FILE* tokens; //token lexeme pairs from phase 1
FILE* first;
FILE* follow; 
FILE* error; //error log file

char *tokenpair[500][3];
char *separator = "|"; //global variable for separator terminal
char *space = " ";
int x = 0;
char* term = " ";
int lineCount = 0;

int l1y = 0;
int l1x = 0;
int err = 0;
char* curr_type;
char* curr_id;
int symbol_count = 0;

int ltable[37][32];

char* scope[100][100] = {"Global", "Procedure Parameter", "Inner"};
int scope_index = 0;
//updated grammar
char* LHS[100][100] = {"Program", "FDECLS", "FDECLS", "FDECLS2", "FDECLS2", "FDEC", "PARAMS", "PARAMS", "PARAMS2", "PARAMS2", "FNAME", "DECLARATIONS", "DECLARATIONS", "DECLARATIONS2", "DECLARATIONS2", "DECL", "TYPE", "TYPE", "VARLIST", "VARLIST2", "VARLIST2", "STATEMENT_SEQ", "STATEMENT_SEQ2", "STATEMENT_SEQ2", "STATEMENT", "STATEMENT", "STATEMENT2", "STATEMENT2", "STATEMENT", "STATEMENT", "STATEMENT", "STATEMENT", "EXPR", "EXPR2", "EXPR2", "EXPR2", "TERM", "TERM2", "TERM2", "TERM2", "TERM2", "FACTOR", "FACTOR", "FACTOR", "FACTOR2","FACTOR2", "EXPRSEQ", "EXPRSEQ", "EXPRSEQ2", "EXPRSEQ2", "BEXPR", "BEXPR2", "BEXPR2", "BTERM", "BTERM2", "BTERM2", "BFACTOR", "BFACTOR", "VAR", "VAR2", "VAR2", "M1", "M2", "M3", "M4", "M5"};
char* RHS[100][100] = {"FDECLS DECLARATIONS STATEMENT_SEQ .", "FDEC ; FDECLS2", "#", "FDEC ; FDECLS2", "#", "def TYPE FNAME ( PARAMS ) DECLARATIONS STATEMENT_SEQ fed", "TYPE VAR PARAMS2", "#", ", PARAMS", "#", "ID", "DECL ; DECLARATIONS2", "#", "DECL ; DECLARATIONS2", "#", "TYPE VARLIST M3", "int", "double", "VAR VARLIST2", ", VARLIST", "#", "STATEMENT STATEMENT_SEQ2", "; STATEMENT_SEQ", "#", "VAR M1 = EXPR M2", "if BEXPR then STATEMENT_SEQ STATEMENT2", "fi", "else STATEMENT_SEQ fi", "while BEXPR do STATEMENT_SEQ od", "print EXPR", "return EXPR", "#", "TERM EXPR2", "+ TERM EXPR2", "- TERM EXPR2", "#", "FACTOR TERM2", "* FACTOR TERM2", "/ FACTOR TERM2", "% FACTOR TERM2", "#", "ID M4 FACTOR2 M4 M5", "NUMBER", "( EXPR )", "VAR2", "( EXPRSEQ )", "EXPR EXPRSEQ2", "#", ", EXPRSEQ", "#", "BTERM BEXPR2", "or BTERM BEXPR2", "#", "BFACTOR BTERM2", "and BFACTOR BTERM2", "#", "not BFACTOR", "( EXPR COMP EXPR )", "ID VAR2", "[ EXPR ]", "#", "#", "#", "#", "#", "#"};

//first and follow sets
char* firstset[100][100] = {".|def|if|while|print|return|;|int|double|id", "def", "def", "def", "int|double", ",", "id", "int|double", "int|double", "int|double", "int|double", "id", ",", "if|while|print|return|;|id", ";", "if|while|print|return|id", "fi|else", "id|number|(", "+|-", "id|number|(", "(|[", "id|number|(", ",", "not|(", "or", "not|(", "and", "not|(", "id", "["};
char* followset[100][100] = {"$", ".|if|while|print|return|;|id|int|double", ".|if|while|print|return|;|id|int|double", ";", ")", ")", "(", ".|if|while|print|return|;|id|fed", ".|if|while|print|return|;|id|fed", ";", ";", ".|fed|fi|else|od", ".|fed|fi|else|od", ".|fed|;|fi|else|od", ".|fed|;|fi|else|od", ".|fed|;|)|,|comp|]|fi|else|od", ".|fed|;|)|,|comp|]|fi|else|od", ".|fed|;|+|-|)|,|comp|]|fi|else|od", ".|fed|;|+|-|)|,|comp|]|fi|else|od", ".|fed|;|+|-|*|/|%|)|,|comp|]|fi|else|od", ".|fed|;|+|-|*|/|%|)|,|comp|]|fi|else|od", ")", ")", "then|do", "then|do", "then|do|or", "then|do|or", "then|do|or|and", ")|,|;|=", ".|fed|;|+|-|&|/|%|)|,|=|comp|]|fi|else|od"};

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

// custom strlen function
size_t my_strlen(const char* str) {
    size_t length = 0;
    while (*str++) {
        length++;
    }
    return length;
}
int is_sync_token(const char* token) {
    return my_strcmp(token, ";") == 0 ||
           my_strcmp(token, "fi") == 0 ||
           my_strcmp(token, "else") == 0 ||
           my_strcmp(token, ".") == 0 ||
           my_strcmp(token, "od") == 0 ||
           my_strcmp(token, "def") == 0 ||
           my_strcmp(token, "print") == 0 ||
           my_strcmp(token, "return") == 0 ||
           my_strcmp(token, "if") == 0 ||
           my_strcmp(token, "while") == 0 ||
           my_strcmp(token, "$") == 0;
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
ASTNode* create_dummy_node(const char* name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->name = (char*)malloc(my_strlen(name) + 16);
    sprintf(node->name, "<%s_error>", name);
    node->type = 1;
    node->searched = 1;
    node->total_children = 0;
    return node;
}

ASTNode* panic_search(char* s){

    if (my_strcmp(s,"if") == 0){
        l1y = 12;
        l1x = 15;
        s = "STATEMENT";

    } 
    ASTNode *no = (ASTNode*)malloc(sizeof(ASTNode));
    return create(no, l1x, l1y);

}
int recursionyay(ASTNode* root, int curr, int temp){
    /*if M1 then check from the table */
   // verybadprogramming:
    if (x >= lineCount){
        printf("LINE COUNTER");
        return 1;
    }
    printf("\nCURRENT ROOT: %s, %d\n", root->name, curr);
    
    
   
    if (root->children[curr]->searched == 0 && root->children[curr]->type == 1){ //if the leftmost child node hasn't been searched and is a non-terminal
        //continue parsing via DFS
        l1x = search(root->children[curr]->name, 0);
        
        printf("NON_TERMINAL\n");
        printf("%s, %d, %d\n", root->children[curr]->name, l1x, l1y);
        printf("%d", ltable[l1x][l1y]);
        if (ltable[l1x][l1y] == 0){ //ERROR PANIC MODE: SKIP LEXEMES UNTIL YOU FIND SYNCHRONIZED TOKEN
            printf("\n0");
            fprintf(error, "Syntax error on line %s", tokenpair[x][2]);
            err++;
            while (x < lineCount) {
                term = tokenpair[x][1];
                curr_id = term;
                if (term == NULL) {
                     break;
                }

                l1y = search(term, 1);
                if (l1y == 8) {
                     term = "ID";
                }
                else if (l1y == 26) {
                    term = "NUMBER";
                }
                else if (l1y == 30) {
                    term = "COMP";
                }

                if (is_sync_token(term)) {
                    break;
                }
                x++;
            }
            root->children[curr] = create_dummy_node(root->children[curr]->name);
            l1x = search(root->children[curr]->name, 0);
            return 1;
        }

       temp = recursionyay(create(root->children[curr], 0, ltable[l1x][l1y]-1), 0, 1);
       if (temp == 0){
        root->children[curr] = create_dummy_node(root->children[curr]->name);
       }
    } else if (root->children[curr]->type == 0) { //if it is a terminal (or epsilon)

        printf("%s, %s, %d\n", root->children[curr]->name, term, l1y);
        if (my_strcmp(root->children[curr]->name, term) == 0){
            printf("\nMATCH!, %s\n", term);
            
            // if (my_strcmp(term, "def") == 0 || my_strcmp(term, "int") == 0|| my_strcmp(term, "double") == 0){ //if the term matched is a definition
            //     printf("curr: %s\n", curr_type);
            //     if (my_strcmp(curr_type, "def") != 0){
                    
            //         curr_type = term;
            //     }
            // } else if (my_strcmp(term, "ID") == 0){ //if the term matched is an identifier
                
            //     printf("current type: %s, %s, %s\n", curr_id, curr_type, scope[0][scope_index]);
            //     add_symboltable(); //add the identifier into the symbol table
            // } else if (my_strcmp(term, "fed") == 0){
            //     symbols = symbols->parent; //go to outer scope for the symbol table
            //     scope_index--;
                
            // }
            x++;
            term = tokenpair[x][1];
            curr_id = term;
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
            printf("%s\n", term);
            printf("%d\n", l1y);
            printf("Line number: %d\n", x);
            
        } else if (my_strcmp(root->children[curr]->name, "#") == 0){ //if the comparison is epsilon
            return 1; //immediately return    
        } else if (my_strcmp(root->children[curr]->name, "#") != 0) { //If comparison doesn't match (error)
           //print that there is an error in the program, move on to next item of the equation
            

            fprintf(error, "error on line %s\n", tokenpair[x][2]);
            printf("ERROR\n");
            
            err++;
            while (x < lineCount) {
                term = tokenpair[x][1];
                curr_id = term;
                if (term == NULL) {
                     break;
                }

                l1y = search(term, 1);
                if (l1y == 8) {
                     term = "ID";
                }
                else if (l1y == 26) {
                    term = "NUMBER";
                }
                else if (l1y == 30) {
                    term = "COMP";
                }

                if (is_sync_token(term)) {
                    break;
                }
                x++;
            }

            root->children[curr] = create_dummy_node(root->children[curr]->name);
            l1x = search(root->children[curr]->name, 0);
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

            return 1;
            
        }

    }
    printf("Current node: %s\n", root->name);
    printf("total children = %d, current = %d\n", root->total_children, curr);
    if (root->total_children <= curr+1){
        //printf("indeed, it does\n");
        //printf("\nCURRENT ROOT: %s, %d\n", root->name, curr);
        return 1;
    }
    
    curr++;
    return recursionyay(root, curr, temp);
    
    //printf("\nCURRENT ROOT: %s, %d\n", root->name, curr);

}

int syntax() {

    
    error = fopen("error.txt", "w");
    tokens = fopen("token_file.txt", "r");
    ll1 = fopen("ll1.txt", "r");

    char line[256];
    char *temp[50];
    
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

    printf("tab = %d", ltable[2][0]);
    
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
    
    recursionyay(root, 0, 1);

    if (err == 0){
       printf("Congratualations!  There were no errors in this code!\n");

    } else {
        printf("Sorry, there were %d error(s) in this code.\n", err);
    }
    
    printf("SYMBOLS\n");
    while (symbols != NULL){
        printf("%s\n", symbols->data);
        symbols = symbols->child;
    }
    free(curr_type);

    return 0;

}

int main(){
    syntax();
    printf("\nDone!");
    return 0;
}