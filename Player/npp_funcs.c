#include "npp_funcs.h"

static void *handle;
NP_InitializeFunc My_NP_Initialize;
NP_ShutdownFunc My_NP_Shutdown;
//NP_GetMIMEDescriptionFunc My_NP_GetMIMEDescription;

NPP_NewProcPtr My_NPP_New;
NPP_DestroyProcPtr My_NPP_Destroy;
NPP_SetWindowProcPtr My_NPP_SetWindow;
NPP_NewStreamProcPtr My_NPP_NewStream;
NPP_DestroyStreamProcPtr My_NPP_DestroyStream;
NPP_WriteReadyProcPtr My_NPP_WriteReady;
NPP_WriteProcPtr My_NPP_Write;
NPP_StreamAsFileProcPtr My_NPP_StreamAsFile;
NPP_PrintProcPtr My_NPP_Print;
NPP_HandleEventProcPtr My_NPP_HandleEvent;
NPP_URLNotifyProcPtr My_NPP_URLNotify;
NPP_GetValueProcPtr My_NPP_GetValue;
NPP_SetValueProcPtr My_NPP_SetValue;
NPP_GotFocusPtr My_NPP_GotFocus;
NPP_LostFocusPtr My_NPP_LostFocus;
NPP_URLRedirectNotifyPtr My_NPP_URLRedirectNotify;
NPP_ClearSiteDataPtr My_NPP_ClearSiteData;
NPP_GetSitesWithDataPtr My_NPP_GetSitesWithData;
NPP_DidCompositePtr My_NPP_DidComposite;

void init_npp(NPPluginFuncs *pPluginFuncs){
    My_NPP_New               = pPluginFuncs->newp;
    My_NPP_Destroy           = pPluginFuncs->destroy;
    My_NPP_SetWindow         = pPluginFuncs->setwindow;
    My_NPP_NewStream         = pPluginFuncs->newstream;
    My_NPP_DestroyStream     = pPluginFuncs->destroystream;
    My_NPP_WriteReady        = pPluginFuncs->writeready;
    My_NPP_Write             = pPluginFuncs->write;
    My_NPP_StreamAsFile      = pPluginFuncs->asfile;
    My_NPP_Print             = pPluginFuncs->print;
    My_NPP_HandleEvent       = pPluginFuncs->event;
    My_NPP_URLNotify         = pPluginFuncs->urlnotify;
    My_NPP_GetValue          = pPluginFuncs->getvalue;
    My_NPP_SetValue          = pPluginFuncs->setvalue;
    My_NPP_GotFocus          = pPluginFuncs->gotfocus;
    My_NPP_LostFocus         = pPluginFuncs->lostfocus;
    My_NPP_URLRedirectNotify = pPluginFuncs->urlredirectnotify;
    My_NPP_ClearSiteData     = pPluginFuncs->clearsitedata;
    My_NPP_GetSitesWithData  = pPluginFuncs->getsiteswithdata;
    My_NPP_DidComposite      = pPluginFuncs->didComposite;
}
#if 0
__attribute__((constructor))
static
#endif
void constructor_npfuncs(){
    NPPluginFuncs pluginFuncs;

    handle = dlopen("/usr/lib64/libflashplayer.so", RTLD_LAZY | RTLD_LOCAL);
    //handle = dlopen("/usr/lib64/flash-plugin/libflashplayer.so", RTLD_LAZY | RTLD_LOCAL);
    My_NP_Initialize = dlsym(handle, "NP_Initialize");
    My_NP_Shutdown = dlsym(handle, "NP_Shutdown");
    //My_NP_GetMIMEDescription = dlsym(handle, "NP_GetMIMEDescription");

    browserFuncs.size = sizeof(browserFuncs);
    My_NP_Initialize(&browserFuncs, &pluginFuncs);
    init_npp(&pluginFuncs);
}
#if 0
__attribute__((destructor))
static
#endif
void destructor_npfuncs(){
    My_NP_Shutdown();
    dlclose(handle);
}
