/*CP471 Lexical Analysis Code (Phase 1)
By: Ethan Sadler
#210557460
*/
//To change testing file, edit the fopen function on line 432
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int accept_states[] = {0,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1,1}; //list of all accepting states (0 if not accepting, 1 if accepting)

//keywords in alphabetical order
char keywords[17][10] = {"and", "def", "do", "double", "else", "fed", "fi", "if", "int", "not", "od", "or", "print", "return", "then", "while"}; 

int line_number = 1; //counter for the number of times enter is pressed (keeps count of the current line of code)

//what the token names should be depending on the what the ending state is (non-accepting state = "", KEYWORD = index 1, etc)
char token_names[17][10] = {"", "KEYWORD", "ID", "OPERATOR", "OPERATOR", "OPERATOR", "OPERATOR", "", "OPERATOR", "OPERATOR", "INT", "", "DOUBLE", "", "", "DOUBLE", "DELIMITER"};

//initialize buffers, buffer indices, buffer pointers, buffer size
char buffer1[1024] = "";
char buffer2[1024] = "";
int buffer1Index;
int buffer2Index;
char* usedBuffer;
char* unusedBuffer;
int buff_index = 0;
size_t bufferSize = sizeof buffer1;

//initialize current lexeme string and index
char currLexeme[100] = "";
int currLexeme_index = 0;

//initialize input file
FILE* input;

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

//method to fill the empty buffer with characters from the input string
int fillBuffer(FILE* input, int index){
    int i,j,max; 

    char c;
    //if the first buffer is empty
    if (buffer1Index == 0) {
        max = sizeof(buffer1);
        for (i = 0; i < max; i++) { //for every element in the first buffer
            c = fgetc(input); //gets character from the input file
            //if you have reached the end of the file
            if (c == EOF) { 
                buffer1[i] = '\0';  // Null-terminate the buffer
                break; //end loop
            }
            buffer1[i] = c; //append character to first buffer
            buffer1Index++;  // Increment first buffer index

        }
        return buffer1Index + index; //return the current input file index
    //if the second buffer is empty
    } else if (buffer2Index == 0){
        max = sizeof(buffer2);
        for (i = 0; i < max; i++) {  //for every element in the second buffer
            c = fgetc(input); //gets character from the input file
            //if you reached the end of the file
            if (c == EOF) {
                buffer2[i] = '\0';  // Null-terminate the second buffer
                break; //end loop
            }
            buffer2[i] = c; //append character to the second buffer
            buffer2Index++;  // Increment second buffer index
        }
        return buffer2Index + index; //return the current input file index
    }
  
}

//method to set usedBuffer pointer to the non-empty buffer
void chooseBuffer(){

    //if the first buffer is empty
    if (buffer1Index == 0){
        //set buffer2 as current buffer
        usedBuffer = buffer2;
        unusedBuffer = buffer1;
    //if the second buffer is empty
    } else {
        //set buffer1 as current buffer
        usedBuffer = buffer1;
        unusedBuffer = buffer2;
    }

    return;
}

//method to clear the usedBuffer and reset its index
void emptyBuffer(){
   //if the usedBuffer is the first buffer
   if (usedBuffer == buffer1){
        //reset first buffer and its index
        memset(buffer1, 0, bufferSize);
        buffer1Index = 0;
    //if the usedBuffer is the second buffer
    } else {
        //reset second buffer and its index
        memset(buffer2, 0, bufferSize);
        buffer2Index = 0;
    }

   return;

}

//method to check if the identifier is a keyword, using binary search
int checkKeyword(char currToken[]){
    //printf("\n%s\n", currToken);

    //initializes left and right variables
    int l = 0; 
    int r = 17 - 1; 
  
    // Loop to implement Binary Search 
    while (l <= r) { 
        
        // Calculatiing mid 
        int m = l + (r - l) / 2; 
  
        // Some random value assigned 
        // as 0 belongs to index 
        int res = -1000; 
  
        if (my_strcmp(currToken, keywords[m]) == 0) 
            res = 0; 
  
        // Check if x is present at mid 
        if (res == 0) 
            return m; 
  
        // If x greater, ignore left half 
        if (my_strcmp(currToken, (keywords[m])) > 0) 
            l = m + 1; 
  
        // If x is smaller, ignore right half 
        else
            r = m - 1; 
    } 
  
    return -1; 
}

//method to go through the current buffer, and assign token-lexeme pairs for the entire buffer
int getNextToken(FILE *trans_tab, int transition[17][128], int currState, FILE* output, FILE* error){
    //intialize index, size of buffer and writer
    int ind = 0;
    int size = 1024;
    char* writer; //string to store the token, so it can be written to token_file.txt
    
    //goes through all elements in the current buffer
    while (ind < size && usedBuffer[ind] != '\0' ){
        
        char symbol = usedBuffer[ind]; //gets current symbol in the buffer
        
        int ascii = (int)symbol; //gets corresponding ascii value of the symbol

        int nextState = transition[currState][ascii]; //gets nextState using the current state and the ascii value from the transition table

        //printf("currState = %d, nextState = %d, lexeme = %s, char = %c\n", currState, nextState, currLexeme, symbol);
        //if the current symbol is a whitespace character
        if (nextState == 7){
            //if the current state is an accepting state
            if (accept_states[currState] == 1){
                //if the lexeme could be a keyword
                if (currState == 1){
                    //initialize keyword_index, calls checkKeyword method to find if identifier is a keyword
                    int keyword_index = checkKeyword(currLexeme);
                    //if the identifier is a keyword (-1 = not in keyword list)
                    if (keyword_index != -1){
                        //assign token writer as KEYWORD
                        //printf("The keyword index for this lexeme is %d, with %s\n", keyword_index, currLexeme);
                        writer = "KEYWORD";
                    //if the identifier is not a keyword
                    } else {
                        //assign token writer as ID
                        //printf("NOT A KEYWORD\n");
                        writer = "ID";
                    }
                //if the lexeme can't be a keyword
                } else {
                    //assign token writer as string in token_names, using the current state's index
                    writer = token_names[currState];
                }
                //print the token-lexeme pair, along with the line number, to token_file.txt
                fprintf(output, "%s|%s|%d\n", writer, currLexeme, line_number);  
                //printf("Accepted!\n");

                //if there is a new line
                if (symbol == '\n'){
                    //increment line_number counter
                    line_number = line_number + 1;
                    //printf("Enter counter ++\n");
                }
                //set current state = 0, clear lexeme string and reset lexeme index
                currState = 0;
                for (int i = 0;i<10;i++){
                    currLexeme[i] = '\0';
                }
                currLexeme_index = 0;
            
            //if the current state is not an accepting state  
            } else {
                //if current state is the starting state
                if (currState == 0){
                    //if there is a new line
                    if (symbol == '\n'){
                        //increment line_number counter
                        line_number = line_number + 1;
                        //printf("Enter counter ++\n");
                    }
                    //move to the next value in the buffer, and continue the loop
                    ind = ind + 1;
                    continue;
                }
                //if the current state is not 0, reject it as a token
                //printf("Rejected!\n");
                //set current state = 0, clear lexeme string and reset lexeme index
                currState = 0;
                for (int i = 0;i<10;i++){
                    currLexeme[i] = '\0';
                }
                currLexeme_index = 0;
            }

            
        //if the nextState is a delimiter
        } else if (nextState == 16){
            //if the current state is an accepting state
            if (accept_states[currState] == 1){
                //if the lexeme could be a keyword
                if (currState == 1){
                    //initialize keyword_index, calls checkKeyword method to find if identifier is a keyword
                    int keyword_index = checkKeyword(currLexeme);
                    //if the identifier is a keyword (-1 = not in keyword list)
                    if (keyword_index != -1){
                        //assign token writer as KEYWORD
                        //printf("The keyword index for this lexeme is %d, with %s\n", keyword_index, currLexeme);
                        writer = "KEYWORD";
                    //if the identifier is not a keyword
                    } else {
                        //assign token writer as ID
                        //printf("NOT A KEYWORD\n");
                        writer = "ID";
                    }
                //if the lexeme can't be a keyword
                } else {
                    //assign token writer as string in token_names, using the current state's index
                    writer = token_names[currState];
                }
                //print the token-lexeme pair, along with the line number, to token_file.txt
                fprintf(output, "%s|%s|%d\n", writer, currLexeme, line_number);  
                //printf("Accepted!\n");

                //reset the current state, clear the lexeme string and reset lexeme index
                currState = 0;
                for (int i = 0;i<10;i++){
                    currLexeme[i] = '\0';
                }
                currLexeme_index = 0;
            
                //set the token writer to delimiter
                writer = "DELIMITER";
                //set the first character in lexeme string to the symbol
                currLexeme[0] = symbol;

                //print the token writer, current lexeme and line number to token_file.txt
                fprintf(output, "%s|%s|%d\n", writer, currLexeme, line_number);
                //reset current lexeme string
                currLexeme[0] = '\0';

            //if the current state is not an accepting state
            } else {
                //if the current state is 0
                if (currState == 0){
                    
                    //set the token writer to delimiter
                    writer = "DELIMITER";
                    //set the first character in lexeme string to the symbol
                    currLexeme[0] = symbol;

                    //print the token writer, current lexeme and line number to token_file.txt
                    fprintf(output, "%s|%s|%d\n", writer, currLexeme, line_number);
                    //reset current lexeme string
                    currLexeme[0] = '\0';

                    //move to next value in the buffer and continue loop
                    ind = ind + 1;
                    continue;
                }
                
                //if the current state is not 0, reject it as a token
                //printf("Rejected!\n");
                //set current state = 0, clear lexeme string and reset lexeme index
                currState = 0;
                for (int i = 0;i<10;i++){
                    currLexeme[i] = '\0';
                }
                currLexeme_index = 0;
            }
        //if the nextState is an error
        } else if (nextState == -1){
            //PANIC MODE
            //if the lexeme could be a keyword
            if (currState == 1){
                //initialize keyword_index, calls checkKeyword method to find if identifier is a keyword
                int keyword_index = checkKeyword(currLexeme);
                //if the identifier is a keyword (-1 = not in keyword list)
                if (keyword_index != -1){
                    //assign token writer as KEYWORD
                    //printf("The keyword index for this lexeme is %d, with %s\n", keyword_index, currLexeme);
                    writer = "KEYWORD";
                //if the identifier is not a keyword
                } else {
                    //assign token writer as ID
                    //printf("NOT A KEYWORD\n");
                    writer = "ID";
                }
            //if the currents state is the starting state
            } else if (currState == 0){
                //print that there is an error on the line number into error.txt
                fprintf(error, "ERROR ON LINE %d\n", line_number);
                
                //move to the next element in the buffer, and continue the loop
                ind = ind + 1;
                continue;
            
            // if the current state is something else
            } else {
                //print there is an error on the line number into error.txt
                fprintf(error, "ERROR ON LINE %d\n", line_number);
                //assign token writer as string in token_names, using the current state's index
                writer = token_names[currState];
            }
            //print the token-lexeme pair previous to the error to token_file.txt
            fprintf(output, "%s|%s|%d\n", writer, currLexeme, line_number);
            
            //go back to the previous element in the buffer
            ind = ind - 1;

            //reset current state, erase lexeme string and reset lexeme index
            currState = 0;

            for (int i = 0;i<10;i++){
                currLexeme[i] = '\0';
            }
            currLexeme_index = 0;
        //if the nextState is the starting state
        } else if (nextState == 0){
            //if the lexeme could be a keyword
            if (currState == 1){
                //initialize keyword_index, calls checkKeyword method to find if identifier is a keyword
                int keyword_index = checkKeyword(currLexeme);
                //if the identifier is a keyword (-1 = not in keyword list)
                if (keyword_index != -1){
                    //assign token writer as KEYWORD
                    //printf("The keyword index for this lexeme is %d, with %s\n", keyword_index, currLexeme);
                    writer = "KEYWORD";
                //if the identifier is not a keyword
                } else {
                    //assign token writer as ID
                    //printf("NOT A KEYWORD\n");
                    writer = "ID";
                }
            //if the lexeme can't be a keyword
            } else {
                //assign token writer as string in token_names, using the current state's index
                writer = token_names[currState];
            }
            //print token-lexeme pair to token_file.txt, with the line number
            fprintf(output, "%s|%s|%d\n", writer, currLexeme, line_number);
            
            //go back to previous element in the buffer
            ind = ind - 1;
            //assign the current state to the next state
            currState = nextState;
            //reset the lexeme string, reset lexeme index
            for (int i = 0;i<10;i++){
                currLexeme[i] = '\0';
            }
            currLexeme_index = 0;


        //if the next state is anything else
        } else {

            //assign current state to the next state
            currState = nextState;
            //append the symbol to the lexeme string
            currLexeme[currLexeme_index] = symbol;
            //increment lexeme index by 1
            currLexeme_index = currLexeme_index + 1;
            //assign null value to end of the new lexeme string
            currLexeme[currLexeme_index] = '\0';
        }
        //increment index by 1
        ind = ind + 1;
    
    }
    //return the current state
    return currState;
}


//method to perform lexical analysis on an input file
int lexicalAnalysis(){
    //initilize variables
    int currState = 0; 
    buff_index = 0;
    int buff_size = 1024;
    int more = 0;
    
    //initialize transition table
    int transition[17][128];


    //initialize files
    FILE *trans_tab;

    FILE *output;
    FILE *error;
    

    //open the files
    trans_tab = fopen("tt.txt", "r");
    output = fopen("token_file.txt", "w");
    input = fopen("TestSyn.cp", "r");
    error = fopen("error.txt", "w");
    
    //import transition table from file into transition array
    for (int i=0;i<17;i++){
        for (int j = 0;j<128;j++){
            fscanf(trans_tab, "\t%d", &transition[i][j]);
        }
    }
    if (NULL == trans_tab){
        printf("File can't be opened \n");
        return -1;
    }

    //fills the first buffer from the input file
    buff_index = fillBuffer(input, buff_index);
   

    //if there is more characters in the file not read by first buffer
    if (buff_index >= buff_size){
        //fill the second buffer from the input file
        buff_index = fillBuffer(input, buff_index);

        //if we are still not at the end of the file
        if (!feof(input)){
            //trigger 'more' flag, incidcating there is still more to be read
            more = 1;
        }
    }

    //set the first buffer as the current buffer, second as the non-used buffer
    chooseBuffer();

    while (1) {
        // if both buffers are empty and we have reached the end of the file
        if (buffer1Index == 0 && buffer2Index == 0 && feof(input)) {
            break;  // end loop
        }
        //get token-lexeme pairs from the current buffer
        currState = getNextToken(trans_tab, transition, currState, output, error);
        
        //printf("%s\n", currLexeme);
        //clear the current buffer (since we have already gotten the token-lexeme pairs)
        emptyBuffer();

        // if there are still more characters from the file to be read
        if (more == 1) {
            //set the unused buffer as the used buffer, and vice versa
            chooseBuffer();

            // Refill the unused/empty buffer
            fillBuffer(input, buff_index);  

            //if the file has been read in full
            if (feof(input)){
                //clear the 'more' flag
                more = 0;  
            }
        //if there is still characters in either of the buffers
        } else if (buffer1Index > 0 || buffer2Index > 0){
            //set usedBuffer to the buffer with characters in it
            chooseBuffer();
        }
    }
    return 0;
}

//main
int main(){
    //run lexical analysis
    lexicalAnalysis();
    
    printf("\nDone!");

    return 0;
}