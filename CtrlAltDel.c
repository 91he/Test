#include "windows.h"
#include <strsafe.h>

__declspec(dllexport) BOOL SimulateAltControlDel()
{
    HDESK   hdeskCurrent;
    HDESK   hdesk;
    HWINSTA hwinstaCurrent;
    HWINSTA hwinsta;

    // 
    // Save the current Window station
    // 
    hwinstaCurrent = GetProcessWindowStation();
    if (hwinstaCurrent == NULL)
        return FALSE;
    // 
    // Save the current desktop
    // 
    hdeskCurrent = GetThreadDesktop(GetCurrentThreadId());
    if (hdeskCurrent == NULL)
        return FALSE;
    // 
    // Obtain a handle to WinSta0 - service must be running
    // in the LocalSystem account
    // 
    hwinsta = OpenWindowStation("winsta0", FALSE,
            WINSTA_ACCESSCLIPBOARD   |
            WINSTA_ACCESSGLOBALATOMS |
            WINSTA_CREATEDESKTOP     |
            WINSTA_ENUMDESKTOPS      |
            WINSTA_ENUMERATE         |
            WINSTA_EXITWINDOWS       |
            WINSTA_READATTRIBUTES    |
            WINSTA_READSCREEN        |
            WINSTA_WRITEATTRIBUTES);
    if (hwinsta == NULL)
        return FALSE;
    // 
    // Set the windowstation to be winsta0
    // 

    if (!SetProcessWindowStation(hwinsta))
        return FALSE;

    // 
    // Get the default desktop on winsta0
    // 
    hdesk = OpenDesktop("Winlogon", 0, FALSE,
            DESKTOP_CREATEMENU |
            DESKTOP_CREATEWINDOW |
            DESKTOP_ENUMERATE    |
            DESKTOP_HOOKCONTROL  |
            DESKTOP_JOURNALPLAYBACK |
            DESKTOP_JOURNALRECORD |
            DESKTOP_READOBJECTS |
            DESKTOP_SWITCHDESKTOP |
            DESKTOP_WRITEOBJECTS);
    if (hdesk == NULL)
        return FALSE;

    // 
    // Set the desktop to be "default"
    // 
    if (!SetThreadDesktop(hdesk))
        return FALSE;

    PostMessage(HWND_BROADCAST,WM_HOTKEY,0,MAKELPARAM(MOD_ALT|MOD_CONTROL,VK_DELETE));


    // 
    // Reset the Window station and desktop
    // 
    if (!SetProcessWindowStation(hwinstaCurrent))
        return FALSE;

    if (!SetThreadDesktop(hdeskCurrent))
        return FALSE;

    // 
    // Close the windowstation and desktop handles
    // 
    if (!CloseWindowStation(hwinsta))
        return FALSE;
    if (!CloseDesktop(hdesk))
        return FALSE;
    return TRUE;
}
