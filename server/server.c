#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "include/http.h"

#define PORT 80
#define BUFFER_SIZE 1000//104857600 malloc para que deje
#define rootDir "../app/"


void send404(httpRequest* req, httpResponse* res) {
    res->statusCode = 404;
    res->ContentType = "text/palin";
    res->data = "404 not Found";
    res->dataSize = 14;
    end(req, res);
}

void getHandler(httpRequest* req, httpResponse* res) {
    if (strstr(req->uri, "..") != NULL) {
        res->statusCode = 403;
        res->data = "403";
        res->dataSize = 3;
        end(req, res);
        return;
    }
    char* fileName = (req->uri + 1)[0] == '\0'? "index.html" : req->uri + 1;
    char* fileUrl = calloc(strlen(fileName), sizeof(rootDir));
    strcpy(fileUrl, rootDir);
    strcat(fileUrl, fileName);
    int fd = open(fileUrl, O_RDONLY);
    free(fileUrl);
    if (fd == -1) {
        send404(req, res);
        return;
    } 

    struct stat file_stat;
    fstat(fd, &file_stat);
    long size = file_stat.st_size;

    char* data = calloc(size, sizeof(char));
    res->data = data;
    res->dataSize = size;
    res->ContentType = get_mime_type(fileName);

    read(fd, data, size);
    close(fd);
    end(req, res);
    free(data);
}

void postHandler(httpRequest* req, httpResponse* res) {
    if (strcmp(req->uri, "/test") == 0) {
        res->data = "<h1> hola </h1>";
        res->dataSize = 20;
        res->ContentType = "text/html";
        end(req, res);
        return;
    }
    send404(req, res);
}

void handler(httpRequest* req, httpResponse* res) {
    printf("%s\n", req->uri);
    if (req->questionMark) {
        printf("%s\n", req->questionMark);
    }
    switch (req->method) {
        case GET: 
            getHandler(req, res);
            break; 
        case POST:
            postHandler(req, res);
            break;
        default: 
            send404(req, res);
    } 
}

int main() {
    printf("Server listening on port %d\n", PORT);
    return runHttpServer(PORT, handler);
}
