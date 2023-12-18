#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http.h"

#define PORT 8080
#define BUFFER_SIZE 1000//104857600 malloc para que deje

void hadler(httpRequest* req, httpResponse* res) {
    char rootDir[] = "../app/";
    switch (req->method) {
    case GET: {
        char* fileName = (req->uri + 1)[0] == '\0'? "index.html" : req->uri + 1;
        char* fileUrl = malloc(strlen(fileName) + sizeof(rootDir));
        strcpy(fileUrl, rootDir);
        strcat(fileUrl, fileName);
        printf("%s", fileUrl);
        int fd = open(fileUrl, O_RDONLY);
        free(fileUrl);
        if (fd == -1) {
            perror("Error al abrir el archivo");
            res->statusCode = 404;
            res->ContentType = "text/palin";
            res->data = "404 not Found";
            res->dataSize = 14;
            end(req, res);
            break;
        } 

        struct stat file_stat;
        fstat(fd, &file_stat);
        long size = file_stat.st_size;

        res->data = malloc(size);
        res->dataSize = size;
        res->ContentType = get_mime_type(fileName);

        read(fd, res->data, size);
        close(fd);
        end(req, res);
        free(res->data);
        break;
    }
    case POST:
        break;
    } 

}

int main() {
    printf("Server listening on port %d\n", PORT);
    return runHttpServer(PORT, hadler);
}
