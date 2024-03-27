#include "include/hashMap.h"
#include "include/queue.h"
#include "include/stack.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum {
    INT,
    FLOAT,
    CHAR,
    STR,
    ARRAY_INT, 
    ARRAY_FLOAT, 
    HTML_VARS
} varType;

typedef struct {
    union {
        int intValue;
        float floatValue;
        char charValue;
        void* pointer;
    };
    varType type; 
    int size;
} HtmlVar;

typedef Map* HtmlVars;

typedef struct {
    char* str;
    int size;
    int insertionIndex;
    int endIndex;
    varType type;
} generatedStr;

typedef struct  {
    char* buffer;
    long size;
    long newSize;
    int index;
    int ifCount;
    int parenthesisCount;
    Stack forStack;
    bool open;
    bool arrayOpen;
    bool strOpen;
} BufferState;

typedef struct {
    int loopIndex;
    int arrayIndex;
    char* key;
    HtmlVar* array;
} ForNode;

HtmlVars newHtmlVars() {
    return (HtmlVars) newMap();
}

void HtmlVarsFree(HtmlVars vars) {
    mapFree(vars);
}

void bindVar(HtmlVars vars, char* key, void* value, varType type) {

    HtmlVar* var = malloc(sizeof(HtmlVar));

    switch(type) {
        case FLOAT:
            var->floatValue = *(float*) value;
            break;
        case INT:
            var->intValue = *(int*) value;
            break;
        case CHAR:
            var->charValue = *(char*) value;
            break;
        default:
            var->pointer = value;
    }

    var->type = type;
    var->size = 0;
    mapSet(vars, key, var);
}

void bindLoopable(HtmlVars vars, char* key, void* value, varType type, int size) {

    HtmlVar* var = malloc(sizeof(HtmlVar));
    var->pointer = value;
    var->type = type;
    var->size = size;
    mapSet(vars, key, var);
}

char* getNextWord(BufferState* bfSt) {
    
    bool inWord = false;

    int wordIndex = 0;
    int wordIncrease = 5;
    int wordSize = 0;

    char* buffer = bfSt->buffer;

    char* word = NULL;

    int i = bfSt->index;

    for (; i < bfSt->size; i++) {

        if (!strncmp(&buffer[i], "}}", 2)) {
            i += 2;
            bfSt->open = false;
            break;
        }

        switch (buffer[i]) {
        
            case '{':
                perror("wrong caracter, { in {{ }}");
                bfSt->index = -1;
                return NULL;
            case '}':
                perror("wrong caracter, } in {{ }}");
                bfSt->index = -1;
                return NULL;
            case '[':
                if (bfSt->arrayOpen) {
                    printf("%s\n", &bfSt->buffer[bfSt->index]);
                    perror("wrong caracter, [ in [ ]");
                    bfSt->index = -1;
                    return NULL;
                }
                bfSt->arrayOpen = true;
                i++;
                goto breakLoop;
            case ']':
                if (!bfSt->arrayOpen) {
                    perror("wrong caracter, ]");
                    bfSt->index = -1;
                    return NULL;
                }
                bfSt->arrayOpen = false;
                i++;
                goto breakLoop;
            case '(':
                bfSt->parenthesisCount++;
                i++;
                goto breakLoop;
            case ')':
                bfSt->parenthesisCount--;
                i++;
                goto breakLoop;
            case '!':
            case '=':
            case '>':
            case '<':
            case '|':
            case '&':
            case ':':
                goto breakLoop;
            case '\'':
            case '"':
                bfSt->strOpen = !bfSt->strOpen;
                i++;
                goto breakLoop;
            case ' ':
            case '\n':
                if (!inWord) {
                    continue;
                }
                goto breakLoop;
            default:
                inWord = true;
        }

        if (wordIndex >= wordSize - 1) {
            wordSize += wordIncrease;
            word = realloc(word, wordSize);

            for (int e = wordSize - wordIncrease; e < wordSize; e++) {
                word[e] = 0;
            }
        }

        word[wordIndex] = buffer[i];
        wordIndex++;
    }
    breakLoop:

    if (i >= bfSt->size) {
        perror("wrong format, word exceeds the file size");
        bfSt->index = -1;
        return NULL;
    }
    
    bfSt->index = i;
    return word;
}

int getIndex(BufferState* bfSt) {

    char* word = getNextWord(bfSt);

    if (!word) {
        perror("getIndex, e1");
        return -1; 
    }

    if (bfSt->arrayOpen) {

        if (!bfSt->open) {
            perror("wrong format, {{}} closed before closing []");
            return -1;
        }

        char* next = getNextWord(bfSt);
        
        if (next) {

            perror("wrong format, to many arguments in []");
            free(next);
            return -1;

        } else if (bfSt->arrayOpen) {

            if (!bfSt->open) {
                perror("wrong format, {{}} closed before closing []");
            } else {
                perror("wrong format, getIndex, e3");
            }
            
            return -1;
        }

    }

    int num = 0;

    char* str = word;

    while (*str >= '0' && *str <= '9' && *str != 0) {
        num = num * 10 + (*str - '0');
        str++;
    }

    if (*str != 0) {
        perror("wrong index");
        return -1; 
    }

    free(word);

    return num;
}

char* getStr(BufferState* bfSt) {

    int wordSize = 5;
    char* word = calloc(wordSize, sizeof(char));

    for (int i = 0; bfSt->index < bfSt->size && bfSt->buffer[bfSt->index] != '\'' && bfSt->buffer[bfSt->index] != '"'; bfSt->index++, i++) {

        if (i >= wordSize - 1) {
            wordSize += 5;
            word = realloc(word, wordSize);
            for (int j = wordSize - 5; j < wordSize; j++) {
                word[j] = 0;
            }
        }

        word[i] = bfSt->buffer[bfSt->index];
    }

    if (bfSt->size <= bfSt->index) {
        perror("wrong format, word exceeds the file size");
        free(word);
        return NULL;
    }

    bfSt->strOpen = false;
    bfSt->index++;

    return word;
}

HtmlVar* htmlVarsGet(BufferState* bfSt, HtmlVars vars) {

    char* word = getNextWord(bfSt);

    if (word) {
        free(word);
        perror("wrong format, HTML_VARS need ' or \" in []");
        return NULL;
    } else if (bfSt->index == -1) {
        return NULL;
    } else if (!bfSt->strOpen) {
        perror("wrong format, HTML_VARS need ' or \" in []");
        return NULL;
    }

    word = getStr(bfSt);

    if (!word) {
        return NULL;
    }

    char* next = getNextWord(bfSt);
    
    if (next) {

        perror("wrong format, to many arguments in []");
        free(next);
        return NULL;

    } else if (bfSt->arrayOpen) {

        if (!bfSt->open) {
            perror("wrong format, {{}} closed before closing []");
        } else {
            perror("wrong format, varToStr, HTML_VARS");
        }
        
        return NULL;
    }


    HtmlVar* htmlVar = mapGet(vars, word);
    free(word);

    return htmlVar;
}

bool varToStr(BufferState* bfSt, HtmlVar* var, generatedStr* gStr) {

    if (!var) {
        perror("var not found");
        return false;
    }
    
    char* word;
    if (!bfSt->open) {
        gStr->endIndex = bfSt->index - 1; 
    } else if (!bfSt->arrayOpen) {
        char* word = getNextWord(bfSt);

        if (word) {
            free(word);
            perror("wrong format, varToStr, e1");
            return false;
        } else if (!bfSt->arrayOpen && bfSt->open) {
            perror("wrong format, varToStr, e2");
            return false;
        }

        gStr->endIndex = bfSt->index - 1;
    }

    int index;
    gStr->type = var->type;
    switch (var->type) {
        case INT:
            gStr->str = calloc(20, 1);
            gStr->size = sprintf(gStr->str, "%d", var->intValue);
            break;
        case FLOAT:
            gStr->str = calloc(20, 1);
            gStr->size = sprintf(gStr->str, "%f", var->floatValue);
            break;
        case CHAR:
            gStr->str = malloc(1);
            gStr->size = 1;
            gStr->str[0] = var->charValue;
            break;
        case STR:
            gStr->str = var->pointer;

            if (var->size == 0) {
                gStr->size = strlen(gStr->str);
            } else {
                gStr->size = var->size;
            }

            break;
        case HTML_VARS:
            return varToStr(bfSt, htmlVarsGet(bfSt, var->pointer), gStr);
        case ARRAY_INT:

            index = getIndex(bfSt);

            if (index == -1) {
                return false;
            }
            
            HtmlVar intVar = {
                .type = INT, 
                .intValue = ((int*) var->pointer)[index]
            };

            return varToStr(bfSt, &intVar, gStr);

        case ARRAY_FLOAT:
            index = getIndex(bfSt);

            if (index == -1) {
                return false;
            }

            HtmlVar floatVar = {
                .type = FLOAT, 
                .intValue = ((float*) var->pointer)[index]
            };

            return varToStr(bfSt, &floatVar, gStr);
        default:
            perror("wrong type or format");
            return false;
    }
    
    bfSt->newSize += gStr->size;
    return true;
}

int8_t eval(BufferState* bfSt, HtmlVars vars, int8_t action, bool previousCon) {

    void* values[2] = {0};
    bool freeValues[2] = {true, true};
    bool caracter = false;
    bool str[2] = {false};
    bool num[2] = {true, true};
    bool equal = false;
    int conditonIndex;

    for (int i = 0; i < 2; i++) {

        //printf("%s\n", &bfSt->buffer[bfSt->index]);
        char* value = getNextWord(bfSt);

        if (bfSt->strOpen) {
            
            if (value) {
                free(value);
                perror("wrong str format in condition");
                return -1;
            }

            value = getStr(bfSt);
            if (!value) {
                perror("in eval str");
                return -1;
            }

            values[i] = value;

            if (value[1]) {
                num[i] = false;
            }

            str[i] = true;

        } else if (!value) { 
            perror("error in eval");
            return -1;
        } else if (value[0] >= '0' && value[0] <= '9') {

            bool floatt = false;
            int e;
            for (e = 0; (value[e] >= '0' && value[e] <= '9' && value[e]) || value[e] == '.' ; e++) {
                if (value[e] == '.') {
                    if (floatt) {
                        perror("2 . in a number");
                        return -1;
                    }

                    floatt = true;
                }
            }

            if (value[e]) { 
                perror("a variable cannot start with a number");
                return -1;
            }

            double* numValue = malloc(sizeof(double));

            if (floatt) {
                *numValue = atof(value); 
            } else {
                *numValue = atoi(value); 
            }

            values[i] = numValue;

        } else {

            HtmlVar* var = mapGet(vars, value);

            do {
                if (!var) {
                    printf("var '%s' not found\n", value);
                    return -1;
                }

                int index;

                double* numValue = malloc(sizeof(double));
                switch (var->type) {
                    case INT:
                        *numValue = var->intValue;
                        values[i] = numValue;
                        break;
                    case FLOAT:
                        *numValue = var->floatValue;
                        values[i] = numValue;
                        break;
                    case CHAR:
                        *numValue = var->charValue;
                        values[i] = numValue;
                        caracter = true;
                        break;
                    case STR:
                        str[i] = true;
                        values[i] = var->pointer;

                        if (((char*) values[i])[1]) {
                            num[i] = false;
                        }
                        freeValues[i] = false;
                        break;
                    case HTML_VARS:
                        var =  htmlVarsGet(bfSt, var->pointer);
                        break;
                    case ARRAY_INT:
                        index = getIndex(bfSt);

                        if (index == -1) {
                            return false;
                        }

                        HtmlVar intVar = {
                            .type = INT, 
                            .intValue = ((int*) var->pointer)[index]
                        };

                        var = &intVar;
                        
                        break;
                    case ARRAY_FLOAT:
                        index = getIndex(bfSt);

                        if (index == -1) {
                            return false;
                        }

                        HtmlVar floatVar = {
                            .type = FLOAT, 
                            .intValue = ((float*) var->pointer)[index]
                        };

                        var = &floatVar;

                        break; 
                }
            } while (!values[i]);
        }

        if (i == 0) { 
            char* next = getNextWord(bfSt);
            if (next) {
                perror("something beofre condition");
                free(next);
                return -1;
            }

            conditonIndex = bfSt->index;

            switch (bfSt->buffer[bfSt->index]) {
                case '=':
                case '!':
                    if (bfSt->buffer[bfSt->index + 1] == '=') {
                        equal = true;
                        bfSt->index++;
                    } else { 
                        perror("wrong condition");
                        return -1;
                    }
             
                case '>':
                case '<':
                    bfSt->index++;
                    break;

                default:
                    goto breakLoop;
                    break;
            }
        }
    }
    breakLoop:;

    int parenthesis = bfSt->parenthesisCount;
    char* next = getNextWord(bfSt);

    if (next) {
        perror("something beofre logic operator or )");
        free(next);
        return -1;
    } else if (bfSt->index == -1) {
        return -1;
    }

    uint8_t nextAction = 0;
    if (!bfSt->parenthesisCount) {
        if (bfSt->open) {
            next = getNextWord(bfSt);

            if (next || bfSt->open) {
                perror("something beofre }}");
                return -1;
            } else if (bfSt->index == -1) {
                return -1;
            }
        }
        nextAction = 3;
    } else {

        if (parenthesis > bfSt->parenthesisCount) {

            next = getNextWord(bfSt);

            if (next) {
                perror("something beofre logic operator");
                free(next);
                return -1;
            } else if (bfSt->index == -1) {
                return -1;
            }
            
        }

        if (!strncmp(&bfSt->buffer[bfSt->index], "&&", 2)) {
            nextAction = 1; 
            bfSt->index += 2;
        } else if (!strncmp(&bfSt->buffer[bfSt->index], "||", 2)) {
            nextAction = 2;
            bfSt->index += 2;
        }
    } 

    if (!nextAction) {
        perror("condition is incomplete");
        return -1;
    }

    #define EVAL(condition) \
        bool con = condition; \
        if (freeValues[0]) { \
            free(values[0]); \
        } \
        if (freeValues[1]) { \
            free(values[1]); \
        } \
        switch (action) { \
            case 1: \
                con = previousCon && con; \
                break; \
            case 2: \
                con = previousCon || con; \
                break; \
        } \
        switch (nextAction) { \
            case 1: \
            case 2: \
                return eval(bfSt, vars, nextAction, con); \
            default: \
                return con; \
         \
        } \

    #define  EVAL_IF_NUM(condition) \
        if (num[0] && num[1]) { \
            \
            double n1, n2; \
            if (str[0]) { \
                n1 = ((char*) values[0])[0]; \
            } else { \
                n1 = *(double*) values[0]; \
            } \
            \
            if (str[1]) { \
                n2 = ((char*) values[1])[0]; \
            } else { \
                n2 = *(double*) values[1]; \
            } \
            EVAL(n1 condition n2); \
        }

    switch (bfSt->buffer[conditonIndex]) {
        case '=':
            EVAL_IF_NUM(==);

            if (caracter && (str[0] || str[1])) {
                EVAL(false);
            }

            if (str[0] && str[1]) {
                EVAL(!strcmp(values[0], values[1]));
            } else {
                perror("can't compare str with not str");
                return -1;
            }
        case '!':
            EVAL_IF_NUM(!=);

            if (caracter && (str[0] || str[1])) {
                EVAL(true);
            }

            if (str[0] && str[1]) {
                EVAL(strcmp(values[0], values[1]));
            } else {
                perror("can't compare str with not str");
                return -1;
            }
        case '>':

            if (equal) {
                EVAL_IF_NUM(>=);
                perror("can't compara str with >=");
            } else {
                EVAL_IF_NUM(>);
                perror("can't compara str with >");
            }

            return -1;

        case '<':
            if (equal) {
                EVAL_IF_NUM(<=);
                perror("can't compara str with <=");
            } else {
                EVAL_IF_NUM(<);
                perror("can't compara str with <");
            }

            return -1;

        default:
            perror("wrong condition");
            return -1;
    }
}

int* mallocSetInt(int setValue) {
    int* num_p = malloc(sizeof(int));
    *num_p = setValue;
    return num_p;
}

void ignoreText(BufferState* bfSt, Queue* ignoreTextIndexs, bool isIf) {

    bfSt->ifCount--;
    char blockType[4] = {0}, end[4] = {0};

    int blockCount = 1;

    if (isIf) {
        strcpy(blockType, "if");
        strcpy(end, "fi");
    } else {
        strcpy(blockType, "for");
        strcpy(end, "rof");
    }

    for (int* i = &bfSt->index; *i < bfSt->size; (*i)++) {

        if (!strncmp(&bfSt->buffer[*i], "{{", 2)) {
            
            bfSt->open = true;

            (*i) += 2;

            char* word = getNextWord(bfSt);

            if (!word && bfSt->index == -1) {
                return;
            }

            bool end_b = !strcmp(end, word);
            if (end_b || !strcmp("else", word)) {

                blockCount--;

                if (!blockCount) {

                    if (bfSt->open) {

                        printf("%s", &bfSt->buffer[*i]);
                        char* next = NULL;

                        if ((next = getNextWord(bfSt))) {
                            free(next);
                            printf("extra word in %s ", end);
                            *i = -1;
                            return;
                        } else if (bfSt->open) { 
                            printf("%s wrong closed", blockType);
                            *i = -1;
                            return;
                        }
                    }

                    enqueue(ignoreTextIndexs, mallocSetInt(*i));

                    break;
                }

                if (!end_b) {
                    blockCount++;
                }

            } else if (!strcmp(blockType, word)) {
                blockCount++;
            }

        }
    }

    if (bfSt->index >= bfSt->size) {
        printf("wrong format, %s never end\n", blockType);
        bfSt->index = -1;
        return;
    }

}

#define checkClose(bfSt, onError) { \
    BufferState* bfSt_p = (BufferState*) bfSt; \
    if (bfSt_p->open) { \
        char* next = getNextWord(bfSt_p); \
        if (next) { \
            free(next); \
            perror("something before }}"); \
            return onError; \
        } else if (bfSt_p->index == -1) { \
            return onError; \
        } \
         \
        if (bfSt_p->open) { \
            perror("{{ not close"); \
            return onError; \
        } \
    } \
}

bool handleIf(BufferState* bfSt, HtmlVars vars, Queue* ignoreTextIndexs, int insertionPoint) {
    /* 
    TODO 
        en el eval:
        - parentesis
        - un solo valor sin == ni nada 
    */
    if (getNextWord(bfSt)) {
        perror("word after if");
        return false;
    } else if (bfSt->index == -1) {
        return false;
    }

    if (bfSt->parenthesisCount != 1) {
        if (bfSt->parenthesisCount > 0) {
            perror("unclosed parentheses");
        } else {
            perror("not ( after if");
        }
        return false;
    }

    enqueue(ignoreTextIndexs, mallocSetInt(insertionPoint));

    switch (eval(bfSt, vars, 3, false)) {
    case -1:
        return false;
    case true:
        enqueue(ignoreTextIndexs, mallocSetInt(bfSt->index));
        break;
    case false:
        ignoreText(bfSt, ignoreTextIndexs, true);

        if (bfSt->index == -1) {
            return false;
        }
        break;
    }
    
    return true;
}

char* getHtml(char* fileName, HtmlVars vars) {

    int fd = open(fileName, O_RDONLY);

    if (fd == -1) {
        return NULL; 
    }
    
    BufferState bfSt = {0};
    Queue* ignoreTextIndexs = newQueue();
    Queue* gStrs = newQueue();

    struct stat file_stat;
    fstat(fd, &file_stat);
    long size = file_stat.st_size;
    bfSt.newSize = size;
    bfSt.size = size;

    bfSt.buffer = malloc(size + 1);
    read(fd, bfSt.buffer, size);
    bfSt.buffer[size] = 0;

    int* i = &bfSt.index;

    while (*i < size) {

        if (!strncmp(&bfSt.buffer[*i], "{{", 2)) {

            int insertionPoint = *i;
            bfSt.open = true;

            (*i) += 2;

            char* word = getNextWord(&bfSt);

            if (!word && *i == -1) {
                return NULL;
            }

            if (!strcmp("for", word)) {

                // TODO range()

                if (getNextWord(&bfSt)) {
                    perror("word after for");
                    return NULL;
                } else if (bfSt.index == -1) {
                    return NULL;
                }

                if (bfSt.parenthesisCount != 1) {
                    if (bfSt.parenthesisCount > 0) {
                        perror("unclosed parentheses");
                    } else {
                        perror("not ( after for");
                    }
                    return NULL;
                }

                char* key = getNextWord(&bfSt);
                char* word;

                if (!key) {
                    perror("wrong caracter after for");
                    return NULL;
                }

                if (bfSt.buffer[bfSt.index] != ':') {

                    word = getNextWord(&bfSt);
                    if (word) {
                        free(word);
                        perror("something before :");
                        return NULL;
                    }

                    if (bfSt.buffer[bfSt.index] != ':') {
                        perror("not : in for or in wrong place");
                        return NULL;
                    }
                }

                char* array = getNextWord(&bfSt);

                if (!array) {
                    return NULL;
                }

                if (bfSt.parenthesisCount) {
                    
                    word = getNextWord(&bfSt);
                    if (word) {
                        free(word);
                        perror("something before )");
                        return NULL;
                    }

                    if (bfSt.parenthesisCount) {
                        perror("parenthesis not closed");
                        return NULL;
                    }
                }

                checkClose(&bfSt, NULL);

                HtmlVar* arr = mapGet(vars, array);
                free(array);

                if (!arr->size) {
                    ignoreText(&bfSt, ignoreTextIndexs, false);

                    if (bfSt.index == -1) {
                        return NULL;
                    }

                } else {
                    
                    HtmlVar* newVar = calloc(1, sizeof(*newVar));
                    ForNode* node = calloc(1, sizeof(*node));
                    node->array = arr;
                    node->arrayIndex = 0;
                    node->loopIndex = bfSt.index;
                    node->key = key;
                    stPush(&bfSt.forStack, node);

                    switch (arr->type) {
                        case STR:
                            newVar->type = CHAR;
                            newVar->charValue = ((char*) arr->pointer)[0];
                            break;
                        case ARRAY_INT:
                            newVar->type = INT;
                            newVar->intValue = ((int*) arr->pointer)[0];
                            break;
                        case ARRAY_FLOAT:
                            newVar->type = FLOAT;
                            newVar->floatValue = ((float*) arr->pointer)[0];
                            break;
                        default:
                            perror("not an array");
                            return NULL;
                    }

                    mapSet(vars, key, newVar);
                }

            } else if (!strcmp("rof", word)) {

                ForNode* node = stPeek(&bfSt.forStack);

                if (!node) {
                    perror("rof without opening for");
                    return NULL;
                }

                node->arrayIndex++;

                if (node->arrayIndex == node->array->size) {

                    enqueue(ignoreTextIndexs, mallocSetInt(insertionPoint));
                    char* next = getNextWord(&bfSt);

                    if (next) {
                        free(next);
                        perror("rof something before }}");
                        return NULL;
                    }

                    enqueue(ignoreTextIndexs, mallocSetInt(bfSt.index));

                    free(node->key);
                    free(node);
                    stPop(&bfSt.forStack);

                } else {
                    
                    HtmlVar* var = mapGet(vars, node->key);

                    switch (node->array->type) {
                        case STR:
                            var->charValue = ((char*) node->array->pointer)[0];
                            break;
                        case ARRAY_INT:
                            var->intValue = ((int*) node->array->pointer)[0];
                            break;
                        case ARRAY_FLOAT:
                            var->floatValue = ((float*) node->array->pointer)[0];
                            break;
                    }

                    *i = node->loopIndex;
                }
                 
            } else if (!strcmp("if", word)) {
                if (!handleIf(&bfSt, vars, ignoreTextIndexs, insertionPoint)) {
                    return NULL;
                }
                bfSt.ifCount++;
            } else if (!strcmp("else", word)) {
                
                if (!bfSt.ifCount) {
                    perror("else without if");
                    return NULL;
                }

                enqueue(ignoreTextIndexs, mallocSetInt(insertionPoint));

                
                bool onlyElse = true;
                if (!bfSt.open) {
                    char* next = getNextWord(&bfSt);
                    if (strcmp("if", next)) {
                        if (!handleIf(&bfSt, vars, ignoreTextIndexs, insertionPoint)) {
                            return NULL;
                        }
                        onlyElse = false;
                    } else {
                        if (next) {
                            free(next);
                            perror("something before }}");
                            return NULL;
                        } else if (bfSt.index == -1) {
                            return NULL;
                        }

                        if (bfSt.open) {
                            perror("{{ not close");
                            return NULL;
                        }
                    }
                }

                if (!onlyElse) {
                    if (ignoreTextIndexs->length % 2) {
                        ignoreText(&bfSt, ignoreTextIndexs, true);

                        if (bfSt.index == -1) {
                            return NULL;
                        }

                    } else {
                        enqueue(ignoreTextIndexs, mallocSetInt(bfSt.index));
                    }
                }

                
            } else if (!strcmp("fi", word)) {

                bfSt.ifCount--;
                if (ignoreTextIndexs->length % 2 == 0) {
                    enqueue(ignoreTextIndexs, mallocSetInt(insertionPoint));
                }                 

                checkClose(&bfSt, NULL);
                enqueue(ignoreTextIndexs, mallocSetInt(bfSt.index));

            } else {

                generatedStr* gStr = malloc(sizeof(*gStr));
                gStr->insertionIndex = insertionPoint;
                enqueue(gStrs, gStr);

                if (!varToStr(&bfSt, mapGet(vars, word), gStr)) {
                    free(word);
                    return NULL;
                }
            }

            free(word);
            
        } else {
            (*i)++;
        }
    }

    //return bfSt.buffer;

    generatedStr* gStr = dequeue(gStrs);
    int* ignoreIndex_p = (int*) dequeue(ignoreTextIndexs);
    int ignoreIndex = *ignoreIndex_p; 
    free(ignoreIndex_p);

    if (!gStr) {
        return bfSt.buffer;
    }

    char* html = calloc(bfSt.newSize + 1, sizeof(char));

    for (int htmlIndex = 0, buffIndex = 0; buffIndex < bfSt.size; buffIndex++) {

        if (ignoreIndex == buffIndex)  {

            ignoreIndex_p = (int*) dequeue(ignoreTextIndexs);
            buffIndex = *ignoreIndex_p;
            free(ignoreIndex_p);

            ignoreIndex_p = (int*) dequeue(ignoreTextIndexs);

            if (ignoreIndex_p) {
                ignoreIndex = *ignoreIndex_p;
                free(ignoreIndex_p);
            } else {
                ignoreIndex = 0;
            }
        }

        if (gStr && gStr->insertionIndex == buffIndex) {

            for (int e = 0; e < gStr->size; e++, htmlIndex++) {
                html[htmlIndex] = gStr->str[e];
            }

            buffIndex = gStr->endIndex;

            switch (gStr->type) {
            case INT:
            case FLOAT:
            case CHAR:
                free(gStr->str);
            }

            free(gStr);
            gStr = dequeue(gStrs);
        } else {
            html[htmlIndex] = bfSt.buffer[buffIndex];
            htmlIndex++;
        }
    }

    queueFree(ignoreTextIndexs);
    queueFree(gStrs);

    return html;
}

int main() {
    HtmlVars vars = newHtmlVars();
    int num = 13; 
    int arr[] = {18, 9, 5};
    float arrf[] = {18.8, 9, 5};

    HtmlVars vars2 = newHtmlVars();
    int ostia = 112;

    bindVar(vars2, "ostia", &ostia, INT);
    bindVar(vars2, "num", &num, INT);
    bindVar(vars2, "texto }a", "mucho texto", STR);

    bindVar(vars, "txt", "hola", STR);

    bindVar(vars, "porro", vars2, HTML_VARS);

    bindVar(vars, "arr", &arr, ARRAY_INT);
    bindVar(vars, "arrf", &arrf, ARRAY_FLOAT);
    bindVar(vars, "hola", &num, INT);
    bindVar(vars, "jiji", &num, INT);
    char Y = 'Y';
    bindVar(vars, "Y", &Y, CHAR);
    float joder = 5.5;
    bindVar(vars, "joderrrr", &joder, FLOAT);
    bindVar(vars, "yepa", "yepa es mi nombre favorito", STR);
    char* html = getHtml("read.html", vars);
    bindVar(vars, "yepa", "yepa es mi nombre favorito", STR);

    if (html) {
        printf("%s", html);
    } else {
        printf("html NULL\n");
    }

    free(html);
    HtmlVarsFree(vars);

    return 0;
}

