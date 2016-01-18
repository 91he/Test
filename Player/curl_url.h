#ifndef CURL_URL_H
#define CURL_URL_H

//#include <gtk/gtk.h>
#include <unistd.h>
#include <stdbool.h>
#include <curl/curl.h>
#include "npapi.h"
#include "list.h"

typedef enum{
    CURL_HTTP_GET,
    CURL_HTTP_POST
}CURL_HTTP_TYPE;

typedef struct header_data{
    bool *redir;
    CURL *curl;
}header_data;

typedef struct body_data{
    bool *redir;
    char *data;
}body_data;

typedef struct buf_data{
    unsigned int offset;
    unsigned int size;
    char *buf;
    char *postData;
}buf_data;

typedef struct stream_data{
    bool isheader;
    bool *redir;
    union{
        CURL *curl;
        buf_data *data;
    };
}stream_data;

typedef struct url_data{
    CURL_HTTP_TYPE httpType;
    NPP instance;
    char *url;
    void *notifyData;
    char *postData;
    buf_data *data;
    int npcode;
}url_data;

int get_url(char *url, void *data, char *cookie);
int post_url(char *url, void *data, char *cookie);

#endif
