#ifndef http
#define http

typedef struct {
    int method;
    char* uri;
    float version;
} httpRequest;

typedef struct {
    int clientFd;
    int statusCode;
    const char* ContentType;
    void* data;
    int dataSize;
    float version;
} httpResponse;

enum methods {
    GET = 224,
    POST = 326,
    PUT = 249,
    DELETE = 435,
    HEAD = 274,
    OPTIONS = 556,
    PATCH = 368,
    CONNECT = 522,
    TRACE = 367, 
    PROPFIND = 610
};

int runHttpServer(int PORT, void (*requesHandler)(httpRequest*, httpResponse*));

void end(httpRequest* request, httpResponse* response);

const char* get_mime_type(const char* file);

#endif
