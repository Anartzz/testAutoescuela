#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/stat.h>

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "include/http.h"

#define BUFFER_SIZE 1000//104857600 malloc para que deje
#define headerSize 100;

int methodHash(char* buffer) {
    int cont = 0;
    for (int i = 0; buffer[i] != ' ' && i < 8; i++) {
        cont += buffer[i];
    }
    return cont;
}

const char* get_mime_type(const char* file) {
    const char *file_ext = strrchr(file, '.');
    if (!file_ext) {
        return "application/octet-stream";
    }
    file_ext++;

    if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0) {
        return "text/html";
    } else if (strcasecmp(file_ext, "txt") == 0) {
        return "text/plain";
    } else if (strcasecmp(file_ext, "css") == 0) {
        return "text/css";
    } else if (strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0) {
        return "image/jpeg";
    } else if (strcasecmp(file_ext, "png") == 0) {
        return "image/png";
    } else if (strcasecmp(file_ext, "js") == 0) {
        return "text/javascript";
    } else {
        return "application/octet-stream";
    }
}

httpRequest* getHttpRequest(char* buffer) {
    httpRequest* headers = malloc(sizeof(httpRequest));
    int i = 0;
    int cont = 0;
    for (; buffer[i] != ' ' && i < 8; i++) {
        cont += buffer[i];
    }
    i++;
    headers->method = cont;
    cont = 0;

    for (;buffer[i] != ' ' && buffer[i] != '?'; i++) {
        cont++;
    }

    char* uri = malloc((cont + 1) * sizeof(char));
    for (int e = 0; e < cont; e++) {
        uri[e] = buffer[i - cont + e];
    }
    uri[cont] = '\0';
    headers->uri = uri;

    if (buffer[i] == '?') {
        i++;
        cont = 0;
        for (; buffer[i] != ' '; i++) {
            cont++;        
        }
        char* questionMark = malloc((cont + 1) * sizeof(char));
        for (int e = 0; e < cont; e++) {
            questionMark[e] = buffer[i - cont + e];
        }
        questionMark[cont] = '\0';
        headers->questionMark = questionMark;
    } else {
        headers->questionMark = NULL;
    }


    return headers;
}

void freeHttpRequest(httpRequest* request) {
    if (request->questionMark) {
        free(request->questionMark);
    }
    free(request->uri);
    free(request);
}

void end(httpRequest* request, httpResponse* response) {
    freeHttpRequest(request);
    int responseSize = response->dataSize + headerSize;
    char* strResponse = malloc(responseSize);

    responseSize = snprintf(strResponse, responseSize, "HTTP/1.1 %i\nContent-Type: %s\ncharset=UTF-8\n\n%s", 
            response->statusCode,
            response->ContentType,
            (char*) response->data
            );
    printf("\n%s\n", strResponse);
    write(response->clientFd, strResponse, responseSize);

    free(strResponse);
    close(response->clientFd);

    free(response);
}

int runHttpServer(int PORT, void (*requestHandler)(httpRequest*, httpResponse*)) {
    struct sockaddr_in clientAddr, serverAddr = {
        .sin_family = AF_INET, 
        .sin_addr.s_addr = INADDR_ANY, 
        .sin_port = htons(PORT)
    };

    int clientFd, serverFd = socket(AF_INET, SOCK_STREAM, 0);
    int reuseAddr = 1;

    if (serverFd < 0) {
        perror("socket failed");
        return 1;
    }

    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) == -1) {
        perror("Error al habilitar SO_REUSEADDR");
        return 1;
    }

    if (bind(serverFd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind failed");
        return 1;
    }

    if (listen(serverFd, 10) < 0) {
        perror("listen failed");
        return 1;
    }

    while (1) {
        socklen_t clientAddrLen = sizeof(clientAddr);
        if ((clientFd = accept(serverFd, (struct sockaddr*) &clientAddr, &clientAddrLen)) < 0) {
            perror("accetp failed");
            return 1;
        }

        char buffer[BUFFER_SIZE];
        int msgLen = read(clientFd, buffer, BUFFER_SIZE); 

        if (msgLen == BUFFER_SIZE) {
            perror("to many data");
            continue;
        }

        if (msgLen == -1) {
            perror("read failed");
            write(clientFd, "Error", 5);
            close(clientFd);
            continue;
        }

        httpResponse* response = malloc(sizeof(httpResponse));
        response->clientFd = clientFd;
        response->statusCode = 200;
        response->ContentType = "text/palin";
        printf("%s", buffer);
        requestHandler(getHttpRequest(buffer), response);
    }

    close(serverFd);
    return 0;
}

