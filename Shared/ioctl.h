#pragma once

#include <Windows.h>

// Helper macros
#define STRINGIFY(x)  #x
#define TOSTRING(x)   STRINGIFY(x)

#define AC_NAME				L"Sentinel"

#define DEVICE_NAME			L"\\Device\\Sentinel"
#define SYMLINK_NAME		L"\\DosDevices\\Sentinel"
#define SYMLINK_USERMODE	L"\\\\.\\Sentinel"

#define IOCTL_AC_HEARTBEAT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_AC_SET_POLICY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)


typedef struct _AC_POLICY {
    ULONG Version;

    BOOLEAN SpoofRdtsc;                         // Cheat timing attack defense
    BOOLEAN HideHypervisorBit;                  // Dont expose hypervisor bit
    BOOLEAN BlockOpenToCsrss;                   // Prevent tool handle hijack
    BOOLEAN BlockOpenToLsass;                   // Prevent tool handle hijack
    BOOLEAN StripAllRemoteProcessHandles;       // Prevent handle creation
    BOOLEAN BlockHandlesToProtectedPIDs;        // Protect user-defined PIDs (game, lsass)
    BOOLEAN BlockCheatTableFiles;               // Block *.ct
    BOOLEAN BlockDllInjectionFiles;             // Block suspicious DLL opens
    BOOLEAN LogMemoryScanPatterns;              // EPT read traps
    BOOLEAN MonitorCR3Switches;
    BOOLEAN LogFileOpensToProtectedPaths;

    WCHAR BlockedPaths[10][MAX_PATH];           // Optional denylist
    ULONG ProtectedPIDs[8];                     // Game process, explorer, etc.


    BOOLEAN KillGameOnViolation;                // If heartbeat stalls, terminate game
    BOOLEAN AlertOnViolation;                   // Alert on violation

} AC_POLICY, * PAC_POLICY;
