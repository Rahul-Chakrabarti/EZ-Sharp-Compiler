// By Rahul Chakrabarti
// If you need to code high lvl datastructure (like trees)
// Code it yourself for what is NEEDED.

// ERROR:

// Inbuilt libraries is good
#include <iostream>
#include <fstream>
#include <list>
#include <cstring> // Honestly, not sure if I need this
#include <cctype> // Check for primitive datatype (alpha, digit, space)
#include <utility> // pair
#include <cstdio>   // For FILE, fopen, fgets, fclose

//GRAMMAR
// <> "non terminal" --> anything encased in <> is not needed
// terminal = something ends (no children if its a tree)
// nonterminal = doesn't produce
// anything not encased (Such as first line . etc) is needed
// Program ends with . character

// Need to refactor this later

// Keywords:
//std::list<string> keywords = {"def", "fed", "int", "double", "if", "fi", "then", "else", "while", "do", "od", "print", "return", "or", "and", "not"};
// Alphabetical for binary search
char keywords[17][10] = {"and", "def", "do", "double", "else", "fed", "fi", "if", "int", "not", "od", "or", "print", "return", "then", "while"};
// Dependent on table, accepting states
// Letters in Keywords:
// a,b,d,e,f,h,i,l,n,o,p,r,s,t,u,w
// Used in DFA
int accepting[] = {0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // diff 18
// SINCE I HAVE 18 TOTAL STATES SO FIRST ARRAY SIZE IS 0-17
char tokenNames[17][11] = {"", "KEYWORD", "ID", "INT", "", "DOUBLE", "", "", "DOUBLE", "OPERATOR", "OPERATOR", "OPERATOR", "OPERATOR", "OPERATOR", "OPERATOR", "WHITESPACE", "DELIMITER"};
// Used C Char arrays for clean comparison throughout getNextToken

// Alphabet
// I didn't end up using these because list is a pain with comparison
std::list<char> alphabet = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
std::list<int> digits = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
std::list<char> brackets = {'[', ']'};
std::list<char> delimiter = {'(', ')', ',', '.', ';', '[', ']'};

// Operators
// char comparison[7][10] = {"<", ">", "==", "<=", ">=", "<>"};
std::list<int> assignment = {'='};
std::list<int> initialization = {'=', ';', ','};
std::list<int> math = {'+', '-', '*', '/', '%'};

// Token
// enum TokenType {KEYWORD, IDENTIFIER, OPERATOR, DELIMITER, INT, DOUBLE};

// Lexical analysis tools
int lineNumber = 1; // For Panic Mode
#define STATES 18
#define ASCII_CHARS 128

// Must be included for program to run
using namespace std;

// Might be implemented as a CLASS
// Needs read and write functionality (for file output namely)
// Double buffer of size 1024 to read file
// I'm very sorry but I didn't end up using the double buffer properly
char buffer1[1024]; // First buffer
char buffer2[1024]; // Second buffer
char* readBuffer;   // Points to the buffer being read
char* unusedBuffer; // Points to the inactive buffers
int dataSize; // How much is in the buffer

// Initialize my transition table (18 states, entire ASCII chart)
int transitionTable[STATES][ASCII_CHARS];  // DFA transition table

// CUSTOM STRING FUNCTIONS FROM SCRATCH

// Custom strcpy: copies src string into dest, including null terminator
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

// Custom strcmp: compares two strings
int my_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)(*s1) - (unsigned char)(*s2);
}

// Custom atoi: converts a string to an integer
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

// TODO write edgecases
int fileInput() {
    // Reads file to buffer
    // Reads until the end of the file
    // Done to update buffer for later computation
    std::ifstream in("Test1.cp");
    if (!in){
        // Not open
        return -1;
    }
    // Experimented with C++ centric file handling and got this abomination that works
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    // Add additional functionality if buffer is not big enough
    my_strcpy(readBuffer, contents.c_str());
    // Don't want to flush out buffer
    cout << readBuffer << '\n';
    dataSize = strlen(readBuffer);
    in.close();
    return 0;
}
int fileOutput() {
    // Writes to Error and Token file
    // Based on the Lexeme Token pairs
    // Basically unused for final implementation
    std::ofstream err("Error.txt");
    std::ofstream out("Token.txt");
    err.close();
    out.close();
    return 0;
}

// Load the transition table from .txt file
bool loadTransitionTable(const char* filename) {
    // Initialize my transition table (18 states, entire ASCII chart)
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening transition table");
        return false;
    }
    char line[1024];  // Buffer to read each line for TRANSITION TABLE
    int state = 0;
    while (fgets(line, sizeof(line), file) && state < STATES) {
        char* token = my_atok(line, " \t\n");  // Split by space, tab, newline
        int col = 0;
        while (token && col < ASCII_CHARS) {
            transitionTable[state][col] = my_atoi(token);  // Convert token to int
            token = my_atok(NULL, " \t\n");              // Move to next token
            col++;
        }
        if (col != ASCII_CHARS) {
            std::cerr << "Error: Incomplete row in transition table at state " << state << "\n";
            fclose(file);
            return false;
        }
        state++;
    }
    fclose(file);
    if (state != STATES) {
        std::cerr << "Error: Incomplete transition table (expected " << STATES << " states)\n";
        return false;
    }
    return true;
}

int swapBuffers() {
    // Swaps the buffer here in case size is not enough  
    std::swap(readBuffer, unusedBuffer);
    return 0;
}

int checkKeyword(char currentToken[]){
    // Need to edit with NEW DFA
    // Implements binary search since list is ordered as mentioned in class
    // Requires list of keywords probably to function (ie find first letter)
    printf("\n%s\n", currentToken);
    int l = 0; 
    int r = 18 - 1; // 17 possible key
    // Letters in Keywords:
    // a,b,d,e,f,h,i,l,n,o,p,r,s,t,u,w
    // Binary search
    while (l <= r) { 
        // Calculating mid 
        int m = l + (r - l) / 2; 
        // Initialize result
        int result = 9000; 
        if (my_strcmp(currentToken, keywords[m]) == 0) 
            result = 0; 
        // Check if x is present at mid 
        if (result == 0) 
            // Intended return
            return m; 
        // If x greater, ignore left half 
        if (my_strcmp(currentToken, (keywords[m])) > 0)
            l = m + 1; 
        // If x is smaller, ignore right half 
        else
            r = m - 1; 
    } 
    // Bad Termination (iterative base case)
    return -1;     
}

int getNextToken(FILE* table, FILE* output, FILE* error, int currentState) {
    // Initialize my transition table (18 states, entire ASCII chart)
    // Stuck on line number 1
    // Need to edit alot and with NEW DFA

    // For some reason I can't simply run the table once in another method. It has to be done here.
    // If I remove fscanf I have a buffer overflow
    // I think I rely too much this being here for the rest of my implementation which is why its breaking
    int transitionTable[STATES][ASCII_CHARS];
    for (int i=0;i<STATES;i++){
        for (int j = 0;j<ASCII_CHARS;j++){
            // fscanf is a huge dependency for the rest of my code
            fscanf(table, "\t%d", &transitionTable[i][j]);
        }
    }
    // OUTPUT KEY:
    // ACCEPT / REJECT for VALID TOKEN
    // KEYWORD / NOT for VALID KEYWORD
    // I attempted to make functionality for printing char by char in file. It's buggy to say the least.  

    // I use lexeme index for bookkeeping purposes based on my implementation of grammar
    int currentLexemeIndex = 0;
    // This can be as big as I need since some variables don't have a definite size limit
    char currentLexeme[50] = "";
    // For output
    const char* parser;
    for (int j = 0; j < dataSize; j++) {
        char symbol = readBuffer[j];
        // Symbol with type int 128 in ascii chart
        int ascii = (int)symbol;
        // Current and Next state represent and derived from state in transitionTable
        int nextState = transitionTable[currentState][ascii];
        printf("currentState = %d, nextState = %d, lexeme = %s, symbol = %c\n", currentState, nextState, currentLexeme, symbol);
        if (nextState == 16){ // EDIT THIS ONE
        // 16 Whitespace ; ascii 59 
            if (accepting[currentState] == 1){
                if (currentState == 1){
                    int keywordIndex = checkKeyword(currentLexeme);
                    if (keywordIndex != -1){
                        printf("The keyword %s has index %d, \n", currentLexeme, keywordIndex);
                        parser = "KEYWORD";
                    } 
                    else {
                        printf("NOT KEYWORD\n");
                        parser = "ID";
                    }
                } 
                else {
                    parser = tokenNames[currentState];
                }
                fprintf(output, "%s|%s|%d\n", parser, currentLexeme, lineNumber);  
                printf("ACCEPT\n");
                // Here, I wanted to make it such that when you hit a ; AND whitespace it counts another line.
                // If I do AND instead of OR I feel like the still erroneous output is more erroneous than just
                // Sticking with OR
                if (symbol == '\n' || symbol == '\0' || symbol == ';'){
                    lineNumber = lineNumber + 1;
                }
                currentState = 0;
                for (int i = 0;i<10;i++){ // Digit
                    currentLexeme[i] = '\0';
                }
                currentLexemeIndex = 0;
               
            } 
            else {
                if (currentState == 0){ // start
                    if (symbol == '\n' || symbol == '\0' || symbol == ';'){
                        lineNumber = lineNumber + 1;
                    }
                    continue;
                }
                printf("REJECT\n");
                currentState = 0;
                for (int i = 0;i<10;i++){
                    currentLexeme[i] = '\0';
                }
                currentLexemeIndex = 0;
            }   
        } 
        else if (nextState == 17){
            //Delimiter (Since I treat brackets like that)
            if (accepting[currentState] == 1){
                if (currentState == 1){
                    int keywordIndex = checkKeyword(currentLexeme);
                    if (keywordIndex != -1){
                        printf("The keyword %s has index %d, \n", currentLexeme, keywordIndex);
                        parser = "KEYWORD";
                    } 
                    else {
                        printf("NOT KEYWORD\n");
                        parser = "ID";
                    }
                } 
                else {
                    parser = tokenNames[currentState];
                }
                fprintf(output, "%s|%s|%d\n", parser, currentLexeme, lineNumber);
                printf("ACCEPT\n");
                currentState = 0;
                for (int i = 0;i<10;i++){
                    currentLexeme[i] = '\0';
                }
                currentLexemeIndex = 0;
                parser = "DELIMITER";
                currentLexeme[0] = symbol;
                fprintf(output, "%s|%s|%d\n", parser, currentLexeme, lineNumber);
                currentLexeme[0] = '\0';
            } 
            else {
                if (currentState == 0){
                    parser = "DELIMITER";
                    currentLexeme[0] = symbol;
                    fprintf(output, "%s|%s|%d\n", parser, currentLexeme, lineNumber);
                    currentLexeme[0] = '\0';
                    continue;
                }
                printf("REJECT\n");
                currentState = 0;
                for (int i = 0;i<10;i++){
                    currentLexeme[0] = '\0';
                }
                currentLexemeIndex = 0;
            }
        } 
        else if (nextState == -1){
            //PANIC MODE
            if (currentState == 1){
                int keywordIndex = checkKeyword(currentLexeme); // Continue process
                if (keywordIndex != -1){
                    printf("The keyword %s has index %d, \n", currentLexeme, keywordIndex);
                    parser = "KEYWORD";
                } 
                else {
                    printf("NOT KEYWORD\n");
                    parser = "ID";
                }
            } 
            else if (currentState == 0){
                continue;
            } 
            else {
                fprintf(error, "ERROR ON LINE %d\n", lineNumber);
                parser = tokenNames[currentState];
            }
            fprintf(output, "%s|%s|%d\n", parser, currentLexeme, lineNumber);
            currentState = 0;
            for (int i = 0;i<10;i++){
                currentLexeme[i] = '\0';
            }
            currentLexemeIndex = 0;
        } 
        else if (nextState == 0){
            if (currentState == 1){
                int keywordIndex = checkKeyword(currentLexeme);
                if (keywordIndex != -1){
                    printf("The keyword %s has index %d, \n", currentLexeme, keywordIndex);
                    parser = "KEYWORD";
                } 
                else {
                    printf("NOT KEYWORD\n");
                    parser = "ID";
                }
            } 
            else {
                parser = tokenNames[currentState];
            }
            fprintf(output, "%s|%s|%d\n", parser, currentLexeme, lineNumber);
            currentState = nextState;
            for (int i = 0;i<10;i++){
                currentLexeme[i] = '\0';
            }
            currentLexemeIndex = 0;
        } 
        else {
            currentState = nextState;
            currentLexeme[currentLexemeIndex] = symbol;
            currentLexemeIndex = currentLexemeIndex + 1;
            currentLexeme[currentLexemeIndex] = '\0';
        }
    }
    // SWAP (retired this)
    // char* temp = unusedBuffer;
    // unusedBuffer = readBuffer;
    // readBuffer = temp;
    // Intented Termination of method
    return 0;
}

int lexicalAnalysis() {
    // variable to determine input size (when to terminate)
    // defined in the buffer
    // Edit path for further testing
    readBuffer = buffer1;
    unusedBuffer = buffer2;
    // Do preliminary file work
    fileInput();
    int currentState = 0;
    FILE *output;
    FILE *error;
    output = fopen("Token.txt", "w");
    error = fopen("Error.txt", "w");
    FILE *table;
    table = fopen("transitionTable.txt", "r");
    // calls getNextToken in a loop WITHIN getNextToken until input is over
    getNextToken(table, output, error, currentState);
    // Perform classification of tokens to construct pairs
    // Panic Mode
    // Write results files and subsequently terminate
    fclose(output);
    fclose(error);
    //fileOutput();
    return 0;
}

// Where the execution of compiler begins
int main() {
    // All work done in lexicalAnalysis
    cout<< "Begin";
    const char* filename = "transitionTable.txt";
    if (loadTransitionTable(filename)) {
        std::cout << "Transition table loaded successfully!\n";
        // Print start state transitions
        for (int column = 0; column < ASCII_CHARS; column++) {\
            // For testing purposes
            std::cout << transitionTable[0][column] << " ";
        }
        std::cout << "\n";
    } 
    else {
        std::cerr << "Failed to load transition table.\n";
        // Terminates if transition table is not proper
        // Since my implementation is completely dependent on table
        return -1;
    }
    lexicalAnalysis();
    // Intended termination
    cout << "Terminated";
    return 0;
}

// Errors:
// I didn't include <> in my transition table. I have to do that next time.
// (I didn't know that <> means not equal until February 3rd
// I also have to edit my dfa for more functionality with <, > and ;
// Misses some symbols such as those 3 above
// Line number functionality is semi complete
// Need to edit my getNextToken looping (not really need to but it will be better then)

// remove left recursion from assigned grammar and do syntax analysis