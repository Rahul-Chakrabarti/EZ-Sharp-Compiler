// By Rahul Chakrabarti
/* This code implements a predictive parser using a custom made LL(1) table.
    Requires lexical analysis to be run first.
    The LL(1) table has already been created in such a way in order to:
        - Remove Left Recursion
        - Remove Left Factors
        - Create First and Follow Sets
*/

// Dependencies include: Token.txt, Error.txt, ll1.txt.
//NOTE: CODE WILL NOT WORK IF TOKEN FILE (Token.txt) INCLUDED IS BLANK

/* ll1.txt is a file containing the raw data needed to formulate the correct 
LL1 table in the source code. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <cstring> 

FILE* ll1;
FILE* tokens; // token lexeme pairs lexical analysis output for input
FILE* first;
FILE* follow; 
FILE* error; //error log file

char *tokenpair[100][100];
char *separator = "|"; //global variable for separator terminal
char *space = " ";
int x = 0;
char* term = " ";
int lineCount = 0;

int l1y = 0;
int l1x = 0;
int err = 0; // renamed to avoid confusion with error file

int ltable[32][32]; // this is probably redudant but though its inefficient it makes 
// the later stages less of a headache

// updated grammar in code for easy retrieval rather than file
// done for robust implementation now and later (since i dont really know exactly what im doing)
char* LHS[100][100] = {"Program", "FDECLS", "FDECLS", "FDECLS2", "FDCLS2", "FDEC", "PARAMS", "PARAMS", "PARAMS2", "PARAMS2", "FNAME", "DECLARATIONS", "DECLARATIONS", "DECLARATIONS2", "DECLARATIONS2", "DECL", "TYPE", "TYPE", "VARLIST", "VARLIST2", "VARLIST2", "STATEMENT_SEQ", "STATEMENT_SEQ2", "STATEMENT_SEQ2", "STATEMENT", "STATEMENT", "STATEMENT2", "STATEMENT2", "STATEMENT", "STATEMENT", "STATEMENT", "STATEMENT", "EXPR", "EXPR2", "EXPR2", "EXPR2", "TERM", "TERM2", "TERM2", "TERM2", "TERM2", "FACTOR", "FACTOR", "FACTOR", "FACTOR2","FACTOR2", "EXPRSEQ", "EXPRSEQ", "EXPRSEQ2", "EXPRSEQ2", "BEXPR", "BEXPR2", "BEXPR2", "BTERM", "BTERM2", "BTERM2", "BFACTOR", "BFACTOR", "VAR", "VAR2", "VAR2"};
char* RHS[100][100] = {"FDECLS DECLARATIONS STATEMENT_SEQ .", "FDEC ; FDECLS2", "#", "FDEC ; FDECLS2", "#", "def TYPE FNAME ( PARAMS ) DECLARATIONS STATEMENT_SEQ fed", "TYPE VAR PARAMS2", "#", ", PARAMS", "#", "ID", "DECL ; DECLARATIONS2", "#", "DECL ; DECLARATIONS2", "#", "TYPE VARLIST", "int", "double", "VAR VARLIST2", ", VARLIST", "#", "STATEMENT STATEMENT_SEQ2", "; STATEMENT_SEQ", "#", "VAR = EXPR", "if BEXPR then STATEMENT_SEQ STATEMENT2", "fi", "else STATEMENT_SEQ fi", "while BEXPR do STATEMENT_SEQ od", "print EXPR", "return EXPR", "#", "TERM EXPR2", "+ TERM EXPR2", "- TERM EXPR2", "#", "FACTOR TERM2", "* FACTOR TERM2", "/ FACTOR TERM2", "% FACTOR TERM2", "#", "ID FACTOR2", "NUMBER", "( EXPR )", "VAR2", "( EXPRSEQ )", "EXPR EXPRSEQ2", "#", ", EXPRSEQ", "#", "BTERM BEXPR2", "or BTERM BEXPR2", "#", "BFACTOR BTERM2", "and BFACTOR BTERM2", "#", "not BFACTOR", "( EXPR COMP EXPR )", "ID VAR2", "[ EXPR ]", "#"};

// first and follow sets
char* firstset[100][100] = {".|def|if|while|print|return|;|int|double|id", "def", "def", "def", "int|double", ",", "id", "int|double", "int|double", "int|double", "int|double", "id", ",", "if|while|print|return|;|id", ";", "if|while|print|return|id", "fi|else", "id|number|(", "+|-", "id|number|(", "(|[", "id|number|(", ",", "not|(", "or", "not|(", "and", "not|(", "id", "["};
char* followset[100][100] = {"$", ".|if|while|print|return|;|id|int|double", ".|if|while|print|return|;|id|int|double", ";", ")", ")", "(", ".|if|while|print|return|;|id|fed", ".|if|while|print|return|;|id|fed", ";", ";", ".|fed|fi|else|od", ".|fed|fi|else|od", ".|fed|;|fi|else|od", ".|fed|;|fi|else|od", ".|fed|;|)|,|comp|]|fi|else|od", ".|fed|;|)|,|comp|]|fi|else|od", ".|fed|;|+|-|)|,|comp|]|fi|else|od", ".|fed|;|+|-|)|,|comp|]|fi|else|od", ".|fed|;|+|-|*|/|%|)|,|comp|]|fi|else|od", ".|fed|;|+|-|*|/|%|)|,|comp|]|fi|else|od", ")", ")", "then|do", "then|do", "then|do|or", "then|do|or", "then|do|or|and", ")|,|;|=", ".|fed|;|+|-|&|/|%|)|,|=|comp|]|fi|else|od"};

// all terminals in the grammar
char* terminals[34][100] = {"$", ".", ";", "def", "(",  ")", "fed", ",", "ID", "int", "double", "=", "if", "then", "fi", "else", "while",
    "do", "od", "print", "return", "+", "-", "*", "/", "%", "NUMBER", "or", "and", "not", "COMP", "[", "]", "#"};
char* operators[7][100] = {"<", ">", "=", "==", ">=", "<="};
char* number[3][100] = {"INT", "DOUBLE"};
char* headers[4][100] = {"ID", "NUMBER", "COMP"};

// all nonterminals in the grammar
char* nonterminals[33][100] = {"Program", "FDECLS", "FDECLS2", "FDEC", "PARAMS", "PARAMS2", "FNAME", "DECLARATIONS", "DECLARATIONS2",
    "DECL", "TYPE", "VARLIST", "VARLIST2", "STATEMENT_SEQ", "STATEMENT_SEQ2", "STATEMENT", "STATEMENT2", "EXPR", "EXPR2", "TERM",
    "TERM2", "FACTOR", "FACTOR2", "EXPRSEQ", "EXPRSEQ2", "BEXPR", "BEXPR2", "BTERM", "BTERM2", "BFACTOR", "VAR", "VAR2"};

// created data structure for an AST Node
typedef struct ASTNode {
    char *name;
    int type; // set to 0 if it is a terminal, 1 if it is a non-terminal
    int searched; // set to 0 if it hasn't been searched yet, 1 if it has
    struct ASTNode* children[15];
    int total_children;
} ASTNode;

int search(char* term, int s){
    int size = sizeof(terminals) / sizeof(terminals[0]);
    int l1 = 0;
    if (s == 1){
        l1 = 8; //for identifier
        for (int i = 0;i<size-1;i++){
            if (strcmp(terminals[0][i], term) == 0){
                l1 = i;
                break;
            }
            if (i < 2){
                if (strcmp(term, number[0][i]) == 0){
                    l1 = 26;
                    break;
                } else if (strcmp(term, operators[0][i]) == 0){
                    l1 = 30;
                    break;
                }
            } else if (i < 6) {
                if (strcmp(term,operators[0][i]) == 0){
                    l1 = 30;
                    break;
                }
            }
        }
    } else {
        for (int i = 0;i<size-1;i++){
            if (strcmp(nonterminals[0][i], term) == 0){
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
    strcpy(str_copy, RHS[x][y]);
    //printf("%s\n", str_copy);
    // Splits str up delimiting w/ spaces, make nodes for its children
    // This is done for eliminating the problem space (the contents of the line) for simpler computation
    char* str = strtok(str_copy, " ");
    while (str != NULL){
        ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
        n->name = (char*)malloc(strlen(str)+1);
        strcpy(n->name, str);
        n->searched = 0;
        int i = search(n->name, 1);
        if ((i == 8 && strcmp(n->name,headers[0][0]) == 0) || (i == 26 && strcmp(n->name, headers[0][1]) == 0) ||
         (i == 30 && strcmp(n->name, headers[0][2]) == 0) || strcmp(n->name,"#") == 0){ //terminal found
            //printf("True, %s", headers[0][0]);
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

void recursion(ASTNode* root, int curr){
    if (x >= lineCount){
        return;
    }
    printf("\nCURRENT ROOT: %s, %d\n", root->name, curr);
    if (root->children[curr]->searched == 0 && root->children[curr]->type == 1){ //if the leftmost child node hasn't been searched and is a non-terminal
        // DFS implemented to parse
        l1x = search(root->children[curr]->name, 0);
        printf("NON_TERMINAL\n");
        printf("%s, %d, %d\n", root->name, l1x, l1y);
        
        if (ltable[l1x][l1y] == 0){ // ERROR (will get caught later): PANIC MODE: POP OUT ELEMENT AND MOVE TO NEXT ITEM IN EQUATION
            return;
        }
        recursion(create(root->children[curr], 0, ltable[l1x][l1y]-1), 0);
    } else if (root->children[curr]->type == 0) { //if it is a terminal (or epsilon)
        printf("%s, %s, %d\n", root->children[curr]->name, term, l1y);
        if (strcmp(root->children[curr]->name, term) == 0){
            printf("\nMATCH!, %s\n", term);
            x++;
            term = tokenpair[x][1];
            if (term == NULL){
                return;
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
        } else if (strcmp(root->children[curr]->name, "#") != 0) { //If comparison doesn't match (error)
           //print that there is an error in the program, move on to next item of the equation
            fprintf(error, "error on line %s\n", tokenpair[x][2]);
            err++;
        }
    }
    //printf("Current node: %s", root->name);
    printf("total children = %d, current = %d\n", root->total_children, curr);
    if (root->total_children <= curr+1){
        //printf("indeed, it does\n");
        return;
    }
    
    curr++;
    recursion(root, curr);
}

int syntaxAnalyser() {
    error = fopen("Error.txt", "w"); // Shares error file with previous phase (and next phases)
    tokens = fopen("Token.txt", "r"); // Needed for code. Lexical is prerequisite
    ll1 = fopen("ll1.txt", "r");
    char line[256];
    char *temp[50];
    
    while (fgets(line, sizeof(line), tokens)){
        char* string = strtok(line, separator);
        int tokenCount = 0;
        while (string != NULL){
            tokenpair[lineCount][tokenCount] = (char*)malloc(strlen(string) + 1);
            // check this
            strcpy(tokenpair[lineCount][tokenCount], string);
            tokenCount++;
            string = strtok(NULL, separator);
        }
        lineCount++;
    }

    for (int i=0;i<32;i++){ // SAME LENGTH AS LL1 TABLE
        for (int j = 0;j<32;j++){
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
    //printf("%s", term);
    //searches all terminals to see where it is in LL1 table
    l1y = search(term, 1);
    //starts to create whole AST
    
    //create the root node of the AST (Program)
    ASTNode *root = (ASTNode*)malloc(sizeof(ASTNode));
    root = create(root, 0, 0);
    recursion(root, 0); // this is where the fun begins

    if (err == 0){
        printf("Congratulations!  This is error free code.");
    } else {
        printf("Sorry, there are %d error(s) in this code.", err);
    }
    return 0;
}

int main(){
    // input handling C++ change cuz faster and more consistnet with lexical (C is easier though)
    syntaxAnalyser();
    printf("\nTerminated."); // sanity check for good termination
    return 0;
}