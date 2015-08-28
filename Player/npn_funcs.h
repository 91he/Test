#ifndef NPN_FUNC_H
#define NPN_FUNC_H

#include "pool.h"
#include "npapi.h"
#include "curl_url.h"
#include "npp_funcs.h"
#include "npfunctions.h"
extern NPNetscapeFuncs browserFuncs;

NPError My_NPN_GetURLNotify(NPP instance, const char* url,
                            const char* target, void* notifyData);
NPError My_NPN_GetURL(NPP instance, const char* url,
                      const char* target);
NPError My_NPN_PostURLNotify(NPP instance, const char* url,
                             const char* target, uint32_t len,
                             const char* buf, NPBool file,
                             void* notifyData);
NPError My_NPN_PostURL(NPP instance, const char* url,
                       const char* target, uint32_t len,
                       const char* buf, NPBool file);
NPError My_NPN_RequestRead(NPStream* stream, NPByteRange* rangeList);
NPError My_NPN_NewStream(NPP instance, NPMIMEType type,
                         const char* target, NPStream** stream);
int32_t My_NPN_Write(NPP instance, NPStream* stream,
                     int32_t len, void* buffer);
NPError My_NPN_DestroyStream(NPP instance, NPStream* stream,
                             NPReason reason);
void My_NPN_Status(NPP instance, const char* message);
const char* My_NPN_UserAgent(NPP instance);
void* My_NPN_MemAlloc(uint32_t size);
void My_NPN_MemFree(void* ptr);
uint32_t My_NPN_MemFlush(uint32_t size);
void My_NPN_ReloadPlugins(NPBool reloadPages);
NPError My_NPN_GetValue(NPP instance, NPNVariable variable, void *value);
NPError My_NPN_SetValue(NPP instance, NPPVariable variable, void *value);
void My_NPN_InvalidateRect(NPP instance, NPRect *invalidRect);
void My_NPN_InvalidateRegion(NPP instance, NPRegion invalidRegion);
void My_NPN_ForceRedraw(NPP instance);
NPIdentifier My_NPN_GetStringIdentifier(const NPUTF8* name);
void My_NPN_GetStringIdentifiers(const NPUTF8** names, int32_t nameCount, NPIdentifier* identifiers);
NPIdentifier My_NPN_GetIntIdentifier(int32_t intid);
bool My_NPN_IdentifierIsString(NPIdentifier identifier);
NPUTF8* My_NPN_UTF8FromIdentifier(NPIdentifier identifier);
int32_t My_NPN_IntFromIdentifier(NPIdentifier identifier);
NPObject* My_NPN_CreateObject(NPP npp, NPClass *aClass);
NPObject* My_NPN_RetainObject(NPObject *obj);
void My_NPN_ReleaseObject(NPObject *obj);
bool My_NPN_Invoke(NPP npp, NPObject* obj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
bool My_NPN_InvokeDefault(NPP npp, NPObject* obj, const NPVariant *args, uint32_t argCount, NPVariant *result);
bool My_NPN_Evaluate(NPP npp, NPObject *obj, NPString *script, NPVariant *result);
bool My_NPN_GetProperty(NPP npp, NPObject *obj, NPIdentifier propertyName, NPVariant *result);
bool My_NPN_SetProperty(NPP npp, NPObject *obj, NPIdentifier propertyName, const NPVariant *value);
bool My_NPN_RemoveProperty(NPP npp, NPObject *obj, NPIdentifier propertyName);
bool My_NPN_HasProperty(NPP npp, NPObject *obj, NPIdentifier propertyName);
bool My_NPN_HasMethod(NPP npp, NPObject *obj, NPIdentifier propertyName);
void My_NPN_ReleaseVariantValue(NPVariant *variant);
void My_NPN_SetException(NPObject *obj, const NPUTF8 *message);
void My_NPN_PushPopupsEnabledState(NPP instance, NPBool enabled);
void My_NPN_PopPopupsEnabledState(NPP instance);
bool My_NPN_Enumerate(NPP npp, NPObject *obj, NPIdentifier **identifier, uint32_t *count);
void My_NPN_PluginThreadAsyncCall(NPP instance,
                                  void (*func) (void *),
                                  void *userData);
bool My_NPN_Construct(NPP npp, NPObject* obj, const NPVariant *args, uint32_t argCount, NPVariant *result);
NPError My_NPN_GetValueForURL(NPP instance, NPNURLVariable variable,
                              const char *url, char **value,
                              uint32_t *len);
NPError My_NPN_SetValueForURL(NPP instance, NPNURLVariable variable,
                              const char *url, const char *value,
                              uint32_t len);
NPError My_NPN_GetAuthenticationInfo(NPP instance,
                                     const char *protocol,
                                     const char *host, int32_t port,
                                     const char *scheme,
                                     const char *realm,
                                     char **username, uint32_t *ulen,
                                     char **password,
                                     uint32_t *plen);
uint32_t My_NPN_ScheduleTimer(NPP instance,
                              uint32_t interval,
                              NPBool repeat,
                              void (*timerFunc)(NPP npp, uint32_t timerID));
void My_NPN_UnscheduleTimer(NPP instance, uint32_t timerID);
NPError My_NPN_PopUpContextMenu(NPP instance, NPMenu* menu);
NPBool My_NPN_ConvertPoint(NPP instance, double sourceX, double sourceY,
                           NPCoordinateSpace sourceSpace, double *destX,
                           double *destY, NPCoordinateSpace destSpace);
NPBool My_NPN_HandleEvent(NPP instance, void *event, NPBool handled);
NPBool My_NPN_UnfocusInstance(NPP instance, NPFocusDirection direction);
void My_NPN_URLRedirectResponse(NPP instance, void* notifyData, NPBool allow);
NPError My_NPN_InitAsyncSurface(NPP instance, NPSize *size,
                                NPImageFormat format, void *initData,
                                NPAsyncSurface *surface);
NPError My_NPN_FinalizeAsyncSurface(NPP instance, NPAsyncSurface *surface);
void My_NPN_SetCurrentAsyncSurface(NPP instance, NPAsyncSurface *surface, NPRect *changed);

#endif
