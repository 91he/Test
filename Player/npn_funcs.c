#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "npn_funcs.h"
#include "frapi.h"

#define	SPECIAL_IDENTIFIER  (NPIdentifier)0x0FEEBBCC
#define SPECIAL_METHOD_NAME "swhxCall"
#define FLASH_REQUEST		"__flash__request"

//extern struct thread_pool *g_pool;
extern FRManager g_FRManager;

NPObject *My_NPAllocate(NPP npp, NPClass *aClass){
    printf("%s\n", __func__);
    NPObject *ret = malloc(sizeof(NPObject));
    //ret->_class = &__gen_class;
    ret->referenceCount = 1;

    return ret;
}

static void My_NPDeallocate(NPObject *npobj){
    printf("%s\n", __func__);
}

static void My_NPInvalidate(NPObject *npobj){
    printf("%s\n", __func__);
}

bool My_NPHasMethod(NPObject *npobj, NPIdentifier name){
    printf("%s\n", __func__);
    return true;
}

bool My_NPInvoke(NPObject *npobj, NPIdentifier name,
                 const NPVariant *args, uint32_t argCount,
                 NPVariant *result){
    printf("%s\n", __func__);
    return true;
}

bool My_NPInvokeDefault(NPObject *npobj,
                        const NPVariant *args,
                        uint32_t argCount,
                        NPVariant *result){
    printf("%s\n", __func__);
    return true;
}

bool My_NPHasProperty(NPObject *npobj, NPIdentifier name){
    printf("%s\n", __func__);
    return true;
}
bool My_NPGetProperty(NPObject *npobj, NPIdentifier name,
                      NPVariant *result){
    printf("%s\n", __func__);
    return true;
}
bool My_NPSetProperty(NPObject *npobj, NPIdentifier name,
                      const NPVariant *value){
    printf("%s\n", __func__);
    return true;
}
bool My_NPRemoveProperty(NPObject *npobj, NPIdentifier name){
    printf("%s\n", __func__);
    return true;
}
bool My_NPEnumeration(NPObject *npobj, NPIdentifier **value,
                      uint32_t *count){
    printf("%s\n", __func__);
    return true;
}
bool My_NPConstruct(NPObject *npobj,
                    const NPVariant *args,
                    uint32_t argCount,
                    NPVariant *result){
    printf("%s\n", __func__);
    return true;
}

static NPClass __gen_class = {
    NP_CLASS_STRUCT_VERSION,
    My_NPAllocate,
    My_NPDeallocate,
    My_NPInvalidate,
    My_NPHasMethod,
    My_NPInvoke,
    My_NPInvokeDefault,
    My_NPHasProperty,
    My_NPGetProperty,
    My_NPSetProperty,
    My_NPRemoveProperty,
    My_NPEnumeration,
    My_NPConstruct,
};

static NPObject __window = { &__gen_class, 1 };
static NPObject __location = { &__gen_class, 1};
static NPObject __top = { &__gen_class, 1 };
static NPObject __top_location = { &__gen_class, 1 };



struct MyNPID{
    unsigned int num;
    char **idstr;
};

static struct MyNPID g_NPID = {0, NULL};

static NPIdentifier AddNPID(const char *name){
    int i;

    if(!strcmp(name, SPECIAL_METHOD_NAME))
        return SPECIAL_IDENTIFIER;

    for(i = 0; i < g_NPID.num; i++){
        if(!strcmp(g_NPID.idstr[i], name))
            return (NPIdentifier)(i + 1);
    }

    g_NPID.num++;
    g_NPID.idstr = realloc(g_NPID.idstr, g_NPID.num);
    g_NPID.idstr[i] = strdup(name);

    return g_NPID.num;
}

static unsigned int npstream_write(NPP instance, NPStream *stream, char *buf, unsigned int len){
    unsigned int tmplen = 0;

    while(len){
        tmplen = len > 4096 ? 4096 : len;

        My_NPP_WriteReady(instance, stream);
        My_NPP_Write(instance, stream, stream->end, tmplen, buf);

        buf += tmplen;
        len -= tmplen;
        stream->end += tmplen;
    }

    return len;
}

gboolean stream_write(gpointer user_data){
    NPStream stream;
    uint16_t stype;
    url_data *data = user_data;

    memset(&stream, 0,sizeof(stream));
    stream.end = 0;
    stream.url = data->url;
    stream.notifyData = data->notifyData;

    My_NPP_NewStream(data->instance, "application/x-shockwave-flash", &stream, 0, &stype);
    if(data->data->offset)
        if(data->httpType == CURL_HTTP_POST){
            int len = data->data->offset;
            char *tmp = data->data->buf;
            if(!strncmp(tmp, "HTTP/1.1", 8) && (tmp = strstr(tmp, "\r\n\r\n"))){
                tmp += 4;
                len -= tmp - data->data->buf;
            }
            //printf("%d ++ %s", data->data->offset - (tmp - data->data->buf), tmp);
            if(!tmp) tmp = data->data->buf;
            //printf("post: %d+++%s\n", len, data->data->buf);
            npstream_write(data->instance, &stream, tmp, len);
        }else{
            npstream_write(data->instance, &stream, data->data->buf, data->data->offset);
        }
    My_NPP_DestroyStream(data->instance, &stream, data->npcode);//TODO
    My_NPP_URLNotify(data->instance, data->url, data->npcode, data->notifyData);

    free(data->url);
    if(data->data->buf) free(data->data->buf);
    free(data->data);
    if(data->postData) free(data->postData);
    free(data);

    return FALSE;
}

static void *url_worker(void *arg){
    url_data *data = arg;
    buf_data *bdata = malloc(sizeof(buf_data));
    bdata->offset = 0;
    bdata->size = 0;
    bdata->buf = NULL;
    bdata->postData = data->postData;

    data->data = bdata;
    //struct list_head *list = malloc(sizeof(struct list_head));

    //INIT_LIST_HEAD(list);
    if(data->httpType == CURL_HTTP_GET){
        //printf("%s\n", data->url);
        data->npcode = get_url(data->url, bdata);
    }else{
        data->npcode = post_url(data->url, bdata);
    }

    //TODO
    gdk_threads_add_idle(stream_write, arg);

    return NULL;
}

static char *StrFromNPID(NPIdentifier id){
    unsigned int iid = (unsigned int)id;

    if(id == SPECIAL_IDENTIFIER)
        return SPECIAL_METHOD_NAME;
    
    if(iid < 1 || iid > g_NPID.num)
        return NULL;
    
    return g_NPID.idstr[iid - 1];
}

static bool MatchNPID(NPIdentifier id, char *name){
    char *str = StrFromNPID(id);

    return str ? !strcmp(str, name) : false;
}

NPError My_NPN_GetURLNotify(NPP instance, const char* url,
                            const char* target, void* notifyData){
    fprintf(stderr, "%s-%d: %p, %s\n", __func__, __LINE__, target, url);
    if(target && !strcmp("_blank", target)){
        My_NPP_URLNotify(instance, url, NPRES_DONE, notifyData);
    }else if(!memcmp(url, "javascript:", 11)){
        NPStream stream;
        char buf[256];

        sprintf(buf, "file:///%X.html__flashplugin_unique__", instance);

        stream.url = url;
        stream.notifyData = notifyData;
        stream.end = strlen(buf);

        My_NPP_NewStream(instance, "text/html", &stream, 0, NULL);

        My_NPP_WriteReady(instance, &stream);
        My_NPP_Write(instance, &stream, 0, stream.end, buf);

        My_NPP_URLNotify(instance, url, NPRES_DONE, notifyData);
        My_NPP_DestroyStream(instance, &stream, NPRES_DONE);
    }else if(!target){
        url_data *data = malloc(sizeof(url_data));

        data->httpType = CURL_HTTP_GET;
        data->instance = instance;
        data->url = strdup(url);
        data->notifyData = notifyData;
        data->postData = NULL;

        pool_add(g_FRManager.pool, url_worker, data);
    }

    return NPERR_NO_ERROR;
}
NPError My_NPN_GetURL(NPP instance, const char* url,
                      const char* target){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
NPError My_NPN_PostURLNotify(NPP instance, const char* url,
                             const char* target, uint32_t len,
                             const char* buf, NPBool file,
                             void* notifyData){
    fprintf(stderr, "%s-%d: %s\n", __func__, __LINE__, url);
    if(target) return NPERR_NO_ERROR;

    url_data *data = malloc(sizeof(url_data));

    data->httpType = CURL_HTTP_POST;
    data->instance = instance;
    data->url = strdup(url);
    data->notifyData = notifyData;
    data->postData = (char*)malloc(len);

    memcpy(data->postData, buf, len);

    pool_add(g_FRManager.pool, url_worker, data);

	return NPERR_NO_ERROR;
}
NPError My_NPN_PostURL(NPP instance, const char* url,
                       const char* target, uint32_t len,
                       const char* buf, NPBool file){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
NPError My_NPN_RequestRead(NPStream* stream, NPByteRange* rangeList){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
NPError My_NPN_NewStream(NPP instance, NPMIMEType type,
                         const char* target, NPStream** stream){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
int32_t My_NPN_Write(NPP instance, NPStream* stream,
                     int32_t len, void* buffer){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return len;
}
NPError My_NPN_DestroyStream(NPP instance, NPStream* stream,
                             NPReason reason){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
void My_NPN_Status(NPP instance, const char* message){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return;
}
const char* My_NPN_UserAgent(NPP instance){
    //fprintf(stderr, "%s-%d: \n", __func__, __LINE__);

    return "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/43.0.2357.65 Safari/537.36";
    //return "Mozilla/5.0 (X11; Linux x86_64; rv:38.0) Gecko/20100101 Firefox/38.0";
    //return "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.2.13) Gecko/20101206 Ubuntu/10.10 (maverick) Firefox/3.6.13";
}
void* My_NPN_MemAlloc(uint32_t size){
    //fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return malloc(size);
}
void My_NPN_MemFree(void* ptr){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    free(ptr);
 
	return;
}
uint32_t My_NPN_MemFlush(uint32_t size){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return size;
}
void My_NPN_ReloadPlugins(NPBool reloadPages){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return;
}
NPError My_NPN_GetValue(NPP instance, NPNVariable variable, void *value){
    //fprintf(stderr, "%s-%d: %d\n", __func__, __LINE__, variable);
    switch(variable){
        case NPNVnetscapeWindow:
        case NPNVSupportsXEmbedBool:
            *(int*)value = 1;
            break;
        case NPNVToolkit:
            *(int*)value = NPNVGtk2;
            break;
        default:
            *(int*)value = 0;
            break;
    }

	return NPERR_NO_ERROR;
}
NPError My_NPN_SetValue(NPP instance, NPPVariable variable, void *value){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
void My_NPN_InvalidateRect(NPP instance, NPRect *invalidRect){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return;
}
void My_NPN_InvalidateRegion(NPP instance, NPRegion invalidRegion){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return;
}
void My_NPN_ForceRedraw(NPP instance){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return;
}
NPIdentifier My_NPN_GetStringIdentifier(const NPUTF8* name){
    fprintf(stderr, "%s-%d: %s\n", __func__, __LINE__, name);
    //TODO
    return AddNPID(name);
}
void My_NPN_GetStringIdentifiers(const NPUTF8** names, int32_t nameCount, NPIdentifier* identifiers){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return;
}
NPIdentifier My_NPN_GetIntIdentifier(int32_t intid){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return (NPIdentifier)0;
}
bool My_NPN_IdentifierIsString(NPIdentifier identifier){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return true;
}
NPUTF8* My_NPN_UTF8FromIdentifier(NPIdentifier identifier){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return "";
}
int32_t My_NPN_IntFromIdentifier(NPIdentifier identifier){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return 0;
}
NPObject* My_NPN_CreateObject(NPP npp, NPClass *aClass){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    NPObject *obj = NULL;
    if(aClass->allocate)
        obj = aClass->allocate(npp, aClass);
    else
        obj = (NPObject*)malloc(sizeof(NPObject));

    obj->_class = aClass;
    obj->referenceCount = 1;

    return obj;
}
NPObject* My_NPN_RetainObject(NPObject *obj){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    if(obj) obj->referenceCount++;

    return obj;
}
void My_NPN_ReleaseObject(NPObject *obj){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    if(!obj || --obj->referenceCount != 0) return;
    
    if(obj->_class->deallocate){
        obj->_class->deallocate(obj);
        return;
    }else if(obj->_class->invalidate){
        obj->_class->invalidate(obj);
    }
    free(obj);
}
bool My_NPN_Invoke(NPP npp, NPObject* obj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    if(MatchNPID(methodName, FLASH_REQUEST) && args[0].type == NPVariantType_String){
        if(argCount == 1){
            return true;
        }else if(argCount = 3 &&
                args[1].type == NPVariantType_String &&
                args[2].type == NPVariantType_String){
            return true;
        }
    }else if(MatchNPID(methodName, "_DoFSCommand") && argCount == 2 &&
                args[0].type == NPVariantType_String &&
                args[1].type == NPVariantType_String){
        //TODO
        return true;
    }else if(obj == &__top_location){ //TODO
        if(MatchNPID(methodName, "toString")){
            result->type = NPVariantType_String;
            result->value.stringValue.UTF8Characters = strdup("chrome://global/content/console.xul");
            result->value.stringValue.UTF8Length = (int)strlen(result->value.stringValue.UTF8Characters);
        }
        return true;
    }else if(obj == &__location){ //TODO
        if(MatchNPID(methodName, "toString")){
            char tmp[1024];
            result->type = NPVariantType_String;
            sprintf(tmp,"file:///%X.html__flashplugin_unique__",methodName);
            result->value.stringValue.UTF8Characters = strdup(tmp);
            result->value.stringValue.UTF8Length = strlen(tmp);
        }
    }else if(MatchNPID(methodName, "__flash_getWindowLocation")){
        result->type = NPVariantType_Object;
        result->value.objectValue = &__location;
        My_NPN_RetainObject(&__location);
        return true;
    }else if(MatchNPID(methodName, "__flash_getTopLocation")){
        result->type = NPVariantType_Object;
        result->value.objectValue = &__top_location;
        My_NPN_RetainObject(&__top_location);
        return true;
    }

    return false;
}
bool My_NPN_InvokeDefault(NPP npp, NPObject* obj, const NPVariant *args, uint32_t argCount, NPVariant *result){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return true;
}
bool My_NPN_Evaluate(NPP npp, NPObject *obj, NPString *script, NPVariant *result){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return true;
}
bool My_NPN_GetProperty(NPP npp, NPObject *obj, NPIdentifier propertyName, NPVariant *result){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return true;
}
bool My_NPN_SetProperty(NPP npp, NPObject *obj, NPIdentifier propertyName, const NPVariant *value){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return true;
}
bool My_NPN_RemoveProperty(NPP npp, NPObject *obj, NPIdentifier propertyName){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return true;
}
bool My_NPN_HasProperty(NPP npp, NPObject *obj, NPIdentifier propertyName){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return true;
}
bool My_NPN_HasMethod(NPP npp, NPObject *obj, NPIdentifier propertyName){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return true;
}
void My_NPN_ReleaseVariantValue(NPVariant *variant){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return;
}
void My_NPN_SetException(NPObject *obj, const NPUTF8 *message){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return;
}
void My_NPN_PushPopupsEnabledState(NPP instance, NPBool enabled){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return;
}
void My_NPN_PopPopupsEnabledState(NPP instance){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return;
}
bool My_NPN_Enumerate(NPP npp, NPObject *obj, NPIdentifier **identifier, uint32_t *count){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return true;
}
void My_NPN_PluginThreadAsyncCall(NPP instance,
                                  void (*func) (void *),
                                  void *userData){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return;
}
bool My_NPN_Construct(NPP npp, NPObject* obj, const NPVariant *args, uint32_t argCount, NPVariant *result){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
    return true;
}
NPError My_NPN_GetValueForURL(NPP instance, NPNURLVariable variable,
                              const char *url, char **value,
                              uint32_t *len){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
NPError My_NPN_SetValueForURL(NPP instance, NPNURLVariable variable,
                              const char *url, const char *value,
                              uint32_t len){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
NPError My_NPN_GetAuthenticationInfo(NPP instance,
                                     const char *protocol,
                                     const char *host, int32_t port,
                                     const char *scheme,
                                     const char *realm,
                                     char **username, uint32_t *ulen,
                                     char **password,
                                     uint32_t *plen){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
uint32_t My_NPN_ScheduleTimer(NPP instance,
                              uint32_t interval,
                              NPBool repeat,
                              void (*timerFunc)(NPP npp, uint32_t timerID)){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return interval;
}
void My_NPN_UnscheduleTimer(NPP instance, uint32_t timerID){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return;
}
NPError My_NPN_PopUpContextMenu(NPP instance, NPMenu* menu){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
NPBool My_NPN_ConvertPoint(NPP instance, double sourceX, double sourceY,
                           NPCoordinateSpace sourceSpace, double *destX,
                           double *destY, NPCoordinateSpace destSpace){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return true;
}
NPBool My_NPN_HandleEvent(NPP instance, void *event, NPBool handled){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return true;
}
NPBool My_NPN_UnfocusInstance(NPP instance, NPFocusDirection direction){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return true;
}
void My_NPN_URLRedirectResponse(NPP instance, void* notifyData, NPBool allow){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return;
}
NPError My_NPN_InitAsyncSurface(NPP instance, NPSize *size,
                                NPImageFormat format, void *initData,
                                NPAsyncSurface *surface){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
NPError My_NPN_FinalizeAsyncSurface(NPP instance, NPAsyncSurface *surface){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return NPERR_NO_ERROR;
}
void My_NPN_SetCurrentAsyncSurface(NPP instance, NPAsyncSurface *surface, NPRect *changed){
    fprintf(stderr, "%s-%d: \n", __func__, __LINE__);
	return;
}

NPNetscapeFuncs browserFuncs = {
    .size                   = sizeof(NPNetscapeFuncs),
    .version                = 21,
    .geturl                 = My_NPN_GetURL,
    .posturl                = My_NPN_PostURL,
    .requestread            = My_NPN_RequestRead,
    .newstream              = My_NPN_NewStream,
    .write                  = My_NPN_Write,
    .destroystream          = My_NPN_DestroyStream,
    .status                 = My_NPN_Status,
    .uagent                 = My_NPN_UserAgent,
    .memalloc               = My_NPN_MemAlloc,
    .memfree                = My_NPN_MemFree,
    .memflush               = My_NPN_MemFlush,
    .reloadplugins          = My_NPN_ReloadPlugins,
    //.getJavaEnv             = My_NPN_GetJavaEnv,
    //.getJavaPeer            = My_NPN_GetJavaPeer,
    .getJavaEnv             = (void*)1013,
    .getJavaPeer            = (void*)1014,
    .geturlnotify           = My_NPN_GetURLNotify,
    .posturlnotify          = My_NPN_PostURLNotify,
    .getvalue               = My_NPN_GetValue,
    .setvalue               = My_NPN_SetValue,
    .invalidaterect         = My_NPN_InvalidateRect,
    .invalidateregion       = My_NPN_InvalidateRegion,
    .forceredraw            = My_NPN_ForceRedraw,
    .getstringidentifier    = My_NPN_GetStringIdentifier,
    .getstringidentifiers   = My_NPN_GetStringIdentifiers,
    .getintidentifier       = My_NPN_GetIntIdentifier,
    .identifierisstring     = My_NPN_IdentifierIsString,
    .utf8fromidentifier     = My_NPN_UTF8FromIdentifier,
    .intfromidentifier      = My_NPN_IntFromIdentifier,
    .createobject           = My_NPN_CreateObject,
    .retainobject           = My_NPN_RetainObject,
    .releaseobject          = My_NPN_ReleaseObject,
    .invoke                 = My_NPN_Invoke,
    .invokeDefault          = My_NPN_InvokeDefault,
    .evaluate               = My_NPN_Evaluate,
    .getproperty            = My_NPN_GetProperty,
    .setproperty            = My_NPN_SetProperty,
    .hasproperty            = My_NPN_HasProperty,
    .hasmethod              = My_NPN_HasMethod,
    .releasevariantvalue    = My_NPN_ReleaseVariantValue,
    .setexception           = My_NPN_SetException,
    .pushpopupsenabledstate = My_NPN_PushPopupsEnabledState,
    .poppopupsenabledstate  = My_NPN_PopPopupsEnabledState,
    .enumerate              = My_NPN_Enumerate,
    .pluginthreadasynccall  = My_NPN_PluginThreadAsyncCall,
    .construct              = My_NPN_Construct,
    .getvalueforurl         = My_NPN_GetValueForURL,
    .setvalueforurl         = My_NPN_SetValueForURL,
    .getauthenticationinfo  = My_NPN_GetAuthenticationInfo,
    .scheduletimer          = My_NPN_ScheduleTimer,
    .unscheduletimer        = My_NPN_UnscheduleTimer,
    .popupcontextmenu       = My_NPN_PopUpContextMenu,
    .convertpoint           = My_NPN_ConvertPoint,
    .handleevent            = My_NPN_HandleEvent,
    .unfocusinstance        = My_NPN_UnfocusInstance,
    .urlredirectresponse    = My_NPN_URLRedirectResponse,
    .initasyncsurface       = My_NPN_InitAsyncSurface,
    .finalizeasyncsurface   = My_NPN_FinalizeAsyncSurface,
    .setcurrentasyncsurface = My_NPN_SetCurrentAsyncSurface
};
