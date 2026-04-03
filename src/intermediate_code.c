/*CP471 Intermediate Code Generation (Phase 4)
By: Ethan Sadler
#210557460
*/
//NOT DONE YET
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stddef.h>

#define MAX_CHILDREN 10
#define MAX_CODE 1024

typedef enum { Function, If, Return, Call, BinaryOp, Print, IntLiteral, Identifier } NodeType;

typedef struct ASTNode {
    NodeType type;
    char value[64];
    struct ASTNode* children[MAX_CHILDREN];
    int childCount;
} ASTNode;

typedef struct {
    char op[16];
    char dst[64];
    char src1[64];
    char src2[64];
    char label[64];
} TAC;


TAC code[MAX_CODE];
int codeIndex = 0;
int tempCounter = 0;
int labelCounter = 0;


// custom strlen function
size_t my_strlen(const char* str) {
    size_t length = 0;
    while (*str++) {
        length++;
    }
    return length;
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
char* newTemp() {
    char* buffer = (char*)malloc(64);
    sprintf(buffer, "t%d", ++tempCounter);
    return buffer;
}

char* newLabel() {
    char* buffer = (char*)malloc(64);
    sprintf(buffer, "lab%d", ++labelCounter);
    return buffer;
}

void emit(const char* op, const char* dst, const char* src1, const char* src2, const char* label) {
    my_strcpy(code[codeIndex].op, op);
    my_strcpy(code[codeIndex].dst, dst);
    my_strcpy(code[codeIndex].src1, src1);
    my_strcpy(code[codeIndex].src2, src2);
    my_strcpy(code[codeIndex].label, label);
    codeIndex++;
}

char* gen(ASTNode* node);

char* gen(ASTNode* node) {
    if (!node) return "";

    if (node->type == IntLiteral) {
        char* t = newTemp();
        emit("", t, node->value, "", "");
        return t;
    } else if (node->type == Identifier) {
        return node->value;
    } else if (node->type == BinaryOp) {
        char* left = gen(node->children[0]);
        char* right = gen(node->children[1]);
        char* t = newTemp();
        emit(node->value, t, left, right, "");
        return t;
    } else if (node->type == Call) {
        for (int i = node->childCount - 1; i >= 0; i--) {
            char* arg = gen(node->children[i]);
            emit("push", arg, "", "", "");
        }
        char* t = newTemp();
        emit("call", t, node->value, "", "");
        for (int i = 0; i < node->childCount; i++) {
            emit("pop", "_", "", "", "");
        }
        return t;
    } else if (node->type == Return) {
        char* val = gen(node->children[0]);
        emit("", "fp-4", val, "", "");
        char label[64];
        sprintf(label, "exit%s", node->value);
        emit("b", "", "", "", label);
        return "";
    } else if (node->type == If) {
        char* cond = gen(node->children[0]);
        char* labTrue = newLabel();
        char* labFalse = newLabel();
        char* labExit = newLabel();

        emit("cmp", cond, "0", "", "");
        emit("bne", "", "", "", labTrue);
        emit("b", "", "", "", labFalse);

        emit("label", "", "", "", labTrue);
        gen(node->children[1]);
        emit("b", "", "", "", labExit);

        emit("label", "", "", "", labFalse);
        if (node->childCount > 2) gen(node->children[2]);

        emit("label", "", "", "", labExit);
        return "";
    } else if (node->type == Print) {
        char* val = gen(node->children[0]);
        emit("print", val, "", "", "");
        return "";
    } else if (node->type == Function) {
        emit("label", "", "", "", node->value);
        emit("label", "", "", "", "Begin");
        emit("push", "LR", "", "", "");
        emit("push", "FP", "", "", "");
        for (int i = 0; i < node->childCount; i++) {
            gen(node->children[i]);
        }
        char label[64];
        sprintf(label, "exit%s", node->value);
        emit("label", "", "", "", label);
        emit("pop", "FP", "", "", "");
        emit("pop", "PC", "", "", "");
    }
    return "";
}

void printTAC() {
    for (int i = 0; i < codeIndex; i++) {
        TAC* line = &code[i];
        if (my_strcmp(line->op, "label") == 0) {
            printf("%s:\n", line->label);
        } else if (my_strcmp(line->op, "b") == 0 || my_strcmp(line->op, "beq") == 0 || my_strcmp(line->op, "bgt") == 0 || my_strcmp(line->op, "bne") == 0) {
            printf("%s %s\n", line->op, line->label);
        } else if (my_strcmp(line->op, "cmp") == 0) {
            printf("cmp %s, %s\n", line->dst, line->src1);
        } else if (my_strcmp(line->op, "push") == 0 || my_strcmp(line->op, "pop") == 0) {
            printf("%s {%s}\n", line->op, line->dst);
        } else if (my_strcmp(line->op, "call") == 0) {
            printf("%s = BL %s\n", line->dst, line->src1);
        } else if (my_strcmp(line->op, "print") == 0) {
            printf("print(%s)\n", line->dst);
        } else if (my_strlen(line->src2) == 0) {
            printf("%s = %s\n", line->dst, line->src1);
        } else {
            printf("%s = %s %s %s\n", line->dst, line->src1, line->op, line->src2);
        }
    }
}

ASTNode* createNode(NodeType type, const char* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    my_strcpy(node->value, value);
    node->childCount = 0;
    return node;
}

void addChild(ASTNode* parent, ASTNode* child) {
    if (parent->childCount < MAX_CHILDREN) {
        parent->children[parent->childCount++] = child;
    }
}

ASTNode* parseExpression(char** str);
ASTNode* parsePrimary(char** str);
ASTNode* parseBinary(char** str);

ASTNode* parsePrimary(char** str) {
    while (isspace(**str)) (*str)++;
    if (isdigit(**str)) {
        char num[64]; int i = 0;
        while (isdigit(**str)) num[i++] = *(*str)++;
        num[i] = '\0';
        return createNode(IntLiteral, num);
    } else if (isalpha(**str)) {
        char ident[64]; int i = 0;
        while (isalnum(**str)) ident[i++] = *(*str)++;
        ident[i] = '\0';
        if (**str == '(') {
            (*str)++;
            ASTNode* call = createNode(Call, ident);
            while (**str != ')') {
                ASTNode* arg = parseExpression(str);
                addChild(call, arg);
                if (**str == ',') (*str)++;
            }
            (*str)++;
            return call;
        } else {
            return createNode(Identifier, ident);
        }
    }
    return NULL;
}

ASTNode* parseBinary(char** str) {
    ASTNode* left = parsePrimary(str);
    while (**str == '+' || **str == '*') {
        char op = *(*str)++;
        ASTNode* right = parsePrimary(str);
        ASTNode* binOp = createNode(BinaryOp, (char[]){op, '\0'});
        addChild(binOp, left);
        addChild(binOp, right);
        left = binOp;
    }
    return left;
}

ASTNode* parseExpression(char** str) {
    ASTNode* left = parseBinary(str);

    while (**str) {
        if (my_strcmp(*str, ">=") == 0 || my_strcmp(*str, "<=") == 0 || my_strcmp(*str, "==") == 0 || my_strcmp(*str, "!=") == 0) {
            char op[3]; my_strcpy(op, *str); op[2] = '\0'; (*str) += 2;
            ASTNode* right = parseBinary(str);
            ASTNode* comp = createNode(BinaryOp, op);
            addChild(comp, left);
            addChild(comp, right);
            left = comp;
        } else if (**str == '>' || **str == '<') {
            char op[2]; op[0] = *(*str)++; op[1] = '\0';
            ASTNode* right = parseBinary(str);
            ASTNode* comp = createNode(BinaryOp, op);
            addChild(comp, left);
            addChild(comp, right);
            left = comp;
        } else {
            break;
        }
    }
    return left;
}

int main(int argc, char* argv[]) {
    char buffer[1024];
    if (argc > 1) {
        FILE* f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Error opening file: %s\n", argv[1]);
            return 1;
        }
        fread(buffer, 1, sizeof(buffer), f);
        fclose(f);
    } else {
        printf("Enter expression: ");
        fgets(buffer, sizeof(buffer), stdin);
    }
    char* p = buffer;
    ASTNode* expr = parseExpression(&p);
    ASTNode* printNode = createNode(Print, "");
    addChild(printNode, expr);

    gen(printNode);
    printTAC();

    return 0;
}