#ifndef NPP_FUNCS_H
#define NPP_FUNCS_H

#include <dlfcn.h>
#include "npapi.h"
#include "npn_funcs.h"
#include "npfunctions.h"

extern NPP_NewProcPtr My_NPP_New;
extern NPP_DestroyProcPtr My_NPP_Destroy;
extern NPP_SetWindowProcPtr My_NPP_SetWindow;
extern NPP_NewStreamProcPtr My_NPP_NewStream;
extern NPP_DestroyStreamProcPtr My_NPP_DestroyStream;
extern NPP_WriteReadyProcPtr My_NPP_WriteReady;
extern NPP_WriteProcPtr My_NPP_Write;
extern NPP_StreamAsFileProcPtr My_NPP_StreamAsFile;
extern NPP_PrintProcPtr My_NPP_Print;
extern NPP_HandleEventProcPtr My_NPP_HandleEvent;
extern NPP_URLNotifyProcPtr My_NPP_URLNotify;
extern NPP_GetValueProcPtr My_NPP_GetValue;
extern NPP_SetValueProcPtr My_NPP_SetValue;
extern NPP_GotFocusPtr My_NPP_GotFocus;
extern NPP_LostFocusPtr My_NPP_LostFocus;
extern NPP_URLRedirectNotifyPtr My_NPP_URLRedirectNotify;
extern NPP_ClearSiteDataPtr My_NPP_ClearSiteData;
extern NPP_GetSitesWithDataPtr My_NPP_GetSitesWithData;
extern NPP_DidCompositePtr My_NPP_DidComposite;

#endif
