#include "include/hashMap.h"
#include "include/queue.h"

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
    bool open;
    bool arrayOpen;
    bool strOpen;
    bool condition;
} BufferState;

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
            case ')':
            case '!':
            case '=':
            case '>':
            case '<':
            case '|':
            case '&':
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

    bfSt->index++;

    return word;
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

            word = getNextWord(bfSt);

            if (word) {
                free(word);
                perror("wrong format, HTML_VARS need ' or \" in []");
                return false;
            } else if (bfSt->index == -1) {
                return false;
            } else if (!bfSt->strOpen) {
                perror("wrong format, HTML_VARS need ' or \" in []");
                return false;
            }

            word = getStr(bfSt);

            if (!word) {
                return false;
            }

            char* next = getNextWord(bfSt);
            
            if (next) {

                perror("wrong format, to many arguments in []");
                free(next);
                return false;

            } else if (bfSt->arrayOpen) {

                if (!bfSt->open) {
                    perror("wrong format, {{}} closed before closing []");
                } else {
                    perror("wrong format, varToStr, HTML_VARS");
                }
                
                return false;
            }


            HtmlVar* htmlVar = mapGet(var->pointer, word);
            free(word);

            return varToStr(bfSt, htmlVar, gStr);

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

int8_t eval(BufferState* bfSt, HtmlVars vars) {

    void* values[2];
    bool freeValues[2] = {false};
    varType type;
    int conditonIndex;

    for (int i = 0; i < 2; i++) {

        char* value = getNextWord(bfSt);

        if (bfSt->strOpen) {
            
            type = STR;

            if (value) {
                free(value);
                perror("wrong str format in condition");
                return -1;
            }

            value = getStr(bfSt);

            if (!value) {
                perror("in eval");
                return -1;
            }

            values[i] = value;

        } else {

            if (!value) {
                perror("in eval");
                return -1;
            }

            if (value[0] >= '0' && value[0] <= '9') {
                bool floatt = false;
                int e;
                for (e = 0; value[e] >= '0' && value[e] <= '9' && value[e]; e++) {
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


                freeValues[i] = true;
                double* numValue = malloc(sizeof(double));

                if (floatt) {
                    type = FLOAT;
                    *numValue = atof(value); 
                } else {
                    type = INT;   
                    *numValue = atoi(value); 
                }

                values[i] = numValue;

            } else {

                HtmlVar* var = mapGet(vars, value);
                // hacer algo con cada caso de typo
                type = var->type;
                values[i] = var->pointer;
            }

            free(value);
        }

        if (i == 0) { 
            char* next = getNextWord(bfSt);
            if (next) {
                perror("something beofre condition");
                free(next);
                return -1;
            }

            conditonIndex = bfSt->index;

            if (bfSt->buffer[bfSt->index + 1] == '=') {
                bfSt->index += 2;
            } else { 
                switch (bfSt->buffer[bfSt->index]) {
                case '=': 
                case '!': 
                    perror("wrong condition");
                    return -1;
                default:
                    bfSt->index++;
            }}
        }
    }

    char* next = getNextWord(bfSt);

    if (next) {
        perror("something beofre logic operator or )");
        free(next);
        return -1;
    }

    uint8_t action = 0;
    if (!strncmp(&bfSt->buffer[bfSt->index], "&&", 2)) {
        action = 1; 
    } else if (!strncmp(&bfSt->buffer[bfSt->index], "||", 2)) {
        action = 2;
    } else if (bfSt->buffer[bfSt->index] == ')') {
        action = 3;
    }

    if (!action) {
        perror("condition is incomplete");
        return -1;
    }

    switch (bfSt->buffer[conditonIndex]) {
        // si es str otra cosa
        // cuando se recursibe que pasa si hay '('
        // free values
        case '=':
            switch (action) {
                case 1: 
                    return *(double *) values[0] == *(double *) values[1] && eval(bfSt, vars);
                case 2:
                    return *(double *) values[0] == *(double *) values[1] || eval(bfSt, vars);
                case 3:
                    return *(double *) values[0] == *(double *) values[1];
            }
        case '!':
            switch (action) {
                case 1: 
                    return *(double *) values[0] != *(double *) values[1] && eval(bfSt, vars);
                case 2:
                    return *(double *) values[0] != *(double *) values[1] || eval(bfSt, vars);
                case 3:
                    return *(double *) values[0] != *(double *) values[1];
            }
        case '>':
            switch (action) {
                case 1: 
                    return *(double *) values[0] >= *(double *) values[1] && eval(bfSt, vars);
                case 2:
                    return *(double *) values[0] >= *(double *) values[1] || eval(bfSt, vars);
                case 3:
                    return *(double *) values[0] >= *(double *) values[1];
            }
        case '<':
            switch (action) {
                case 1: 
                    return *(double *) values[0] <= *(double *) values[1] && eval(bfSt, vars);
                case 2:
                    return *(double *) values[0] <= *(double *) values[1] || eval(bfSt, vars);
                case 3:
                    return *(double *) values[0] <= *(double *) values[1];
            }
 
        default:
            perror("wrong condition");
            return -1;
    }
}

void ignoreText(BufferState* bfSt, Queue* ignoreTextIndexs, bool ifOrFor) {

    char blockType[4] = {0}, end[4] = {0};

    int blockCount = 1;

    if (ifOrFor) {
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

            if (!strcmp(end, word)) {

                blockCount--;

                if (!blockCount) {

                    if (bfSt->open) {
                        char* next = NULL;

                        if ((next = getNextWord(bfSt))) {
                            free(next);
                            printf("extrar word in %s", end);
                            *i = -1;
                            return;
                        } else if (bfSt->open) { 
                            printf("%s wrong closed", blockType);
                            *i = -1;
                            return;
                        }
                    }

                    enqueue(ignoreTextIndexs, (void*) *i);

                    break;
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

char* getHtml(char* fileName, HtmlVars vars) {

    int fd = open(fileName, O_RDONLY);

    if (fd == -1) {
        return NULL; 
    }
    
    BufferState bfSt;

    bfSt.arrayOpen = false;
    bfSt.strOpen = false;
    bfSt.ifCount = 0;

    struct stat file_stat;
    fstat(fd, &file_stat);
    long size = file_stat.st_size;
    bfSt.newSize = size;
    bfSt.size = size;

    bfSt.buffer = malloc(size + 1);
    read(fd, bfSt.buffer, size);
    bfSt.buffer[size] = 0;
    
    Queue* ignoreTextIndexs = newQueue();
    Queue* gStrs = newQueue();

    bfSt.index = 0;
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
            
            } else if (!strcmp("if", word)) {

                if (getNextWord(&bfSt)) {
                    perror("word after if");
                    return NULL;
                } else if (bfSt.index == -1) {
                    return NULL;
                }

                if (bfSt.buffer[bfSt.index] != '(') {
                    perror("not ( after if");
                    return NULL;
                }

                bfSt.index++;
                bfSt.ifCount++;

                enqueue(ignoreTextIndexs, (void*) insertionPoint);

                switch (eval(&bfSt, vars)) {
                case -1:
                    return NULL;
                case true:
                    enqueue(ignoreTextIndexs, (void*) bfSt.index);
                    break;
                case false:
                    ignoreText(&bfSt, ignoreTextIndexs, true);

                    if (*i == -1) {
                        return NULL;
                    }
                    break;
                }
    
            } else if (!strcmp("else", word)) {
                
                // else if ?

                if (!bfSt.ifCount) {
                    perror("else without if");
                    return NULL;
                }
                // cerrar elese

                enqueue(ignoreTextIndexs, (void*) bfSt.index);

                if (ignoreTextIndexs->length % 2) {
                    // buscar fi ignorando
                } else {
                    // seguir
                }
                
            } else if (!strcmp("fi", word)) {

                if (ignoreTextIndexs->length % 2) {
                    enqueue(ignoreTextIndexs, (void*) insertionPoint);
                }                 

                enqueue(ignoreTextIndexs, (void*) bfSt.index);

            } else {

                generatedStr* gStr = malloc(sizeof(generatedStr));
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
    int ignoreIndex = (long) dequeue(ignoreTextIndexs);

    if (!gStr) {
        return bfSt.buffer;
    }

    char* html = calloc(bfSt.newSize + 1, sizeof(char));

    // recursivo ??
    // for y if en el ignore
    for (int htmlIndex = 0, buffIndex = 0; buffIndex < bfSt.size; buffIndex++) {

        if (ignoreIndex == buffIndex)  {
            buffIndex = (long) dequeue(ignoreTextIndexs);
            ignoreIndex = (long) dequeue(ignoreTextIndexs);
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
    // queda if y loops 
    HtmlVars vars = newHtmlVars();
    int num = 13; 
    int arr[] = {18, 9, 5};
    float arrf[] = {18.8, 9, 5};

    HtmlVars vars2 = newHtmlVars();
    int ostia = 112;
    char texto[] = "mucho texto";
    bindVar(vars2, "ostia", &ostia, INT);
    bindVar(vars2, "num", &num, INT);
    bindVar(vars2, "texto }a", texto, STR);

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

    if (html) {
        printf("%s", html);
    } else {
        printf("html NULL\n");
    }

    free(html);
    HtmlVarsFree(vars);

    return 0;
}

