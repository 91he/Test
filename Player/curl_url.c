#include <string.h>
#include "curl_url.h"

static size_t write_func(char *ptr, size_t size, size_t n, void *data){
    char *tmp;
    int len = size * n;
    stream_data *pdata = data;


    if(pdata->isheader){
        //printf("%s", ptr);
        if(strstr(ptr, "HTTP/1.1 30"))
            *pdata->redir = true;
        if(*pdata->redir && (tmp = strstr(ptr, "Location: "))){
            tmp += sizeof("Location:");
            *strstr(tmp, "\r\n") = 0;
            //printf("+++++%s\n", tmp);
            curl_easy_setopt(pdata->curl, CURLOPT_URL, tmp);
        }
    }else{
        char *buf;
        int size;
        int offset;
        if(*pdata->redir) return len;
        if(len == 0) return 0;

        buf = pdata->data->buf;
        offset = pdata->data->offset;
        size = pdata->data->size;

        while(size < offset + len){
            size = size ? 2 * size : 1024;
            buf = realloc(buf, size);
        }
        memcpy(buf + offset, ptr, len);

        pdata->data->size = size;
        pdata->data->buf = buf;
        pdata->data->offset = offset + len;
    }

    return len;
}

int get_url(char *url, void *data, char *cookie){
    int ret;
    bool redir;;
    stream_data hdata;
    stream_data bdata;

    CURL *curl = curl_easy_init();

    hdata.isheader = true;
    hdata.redir = &redir;
    hdata.curl = curl;

    bdata.isheader = false;
    bdata.redir = &redir;
    bdata.data = data;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); 
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2);
    //curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.2.13) Gecko/20101206 Ubuntu/10.10 (maverick) Firefox/3.6.13");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bdata);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
    //curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_func);
    //curl_easy_setopt(curl, CURLOPT_HEADERDATA, &hdata);
    //curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "/tmp/tmp.txt");
    //curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/tmp/tmp.txt");

    ret = NPRES_DONE;
    //printf("%s\n", url);
    //int i = 0;
    //do{
        //    i++;
        //    redir = false;
        if(CURLE_OK != curl_easy_perform(curl))
            ret = NPRES_NETWORK_ERR;
        long code;
        double size;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &size);
        fprintf(stderr, "CODE: %ld, %lf\t%s\n", code, size, url);
    //}while(0);

    curl_easy_cleanup(curl);

    //buf_data *bd = data;
    //printf("length: %d\n", bd->offset);
    /*
    if(bd->buf){
        printf("%s\n", url);
        printf("++++++%s\n", bd->buf);
        printf("=========================\n");
    }
    */

    return ret;
}

int post_url(char *url, void *data, char *cookie){
    int ret;
    int len;
    char *tmp;
    char *a, *b;
    bool redir;;
    stream_data hdata;
    stream_data bdata;
    struct curl_slist *list = NULL;
    char *pdata = ((buf_data*)data)->postData;

    sscanf(strstr(pdata, "Content-Length:"), "%*[^0-9]%d", &len);
    tmp = strstr(pdata, "\r\n\r\n") + 4;
    *(tmp - 2) = 0;
    a = b = pdata;
    while(a = strstr(b, "\r\n")){
        *a = 0;
        list = curl_slist_append(list, b);
        b = a + 2;
    }

    CURL *curl = curl_easy_init();

    hdata.isheader = true;
    hdata.redir = &redir;
    hdata.curl = curl;

    bdata.isheader = false;
    bdata.redir = &redir;
    bdata.data = data;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); 
    //curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2);
    //curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, tmp);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);
    //curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    //curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bdata);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    //curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_func);
    //curl_easy_setopt(curl, CURLOPT_HEADERDATA, &hdata);
    //curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "/tmp/tmp.txt");
    //curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/tmp/tmp.txt");

    ret = NPRES_DONE;
    if(CURLE_OK != curl_easy_perform(curl))
        ret = NPRES_NETWORK_ERR;

    long code;
    double size;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &size);
    fprintf(stderr, "POST+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++CODE: %ld, %lf\t%s\n", code, size, url);

    curl_easy_cleanup(curl);
    curl_slist_free_all(list);

    //buf_data *bd = data;
    //printf("post ------ length: %d\n", bd->offset);
    //if(bd->buf)
    //    printf("%s\n", bd->buf);
    
    return ret;
}
