#include <iostream>
#include "DriverTypes.h"

class DriverInterface {
public:
    HANDLE hDevice;

    // On instantiation, get a handle to the driver and execute our first IOCTL call.
    DriverInterface() {
        hDevice = CreateFileA(
                "\\\\.\\EchoDrv",
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                NULL,
                NULL
        );

        //If driver handle failed to open print message and return
        if (hDevice == INVALID_HANDLE_VALUE) {
            std::cout << "Invalid handle on CreateFileA!" << std::endl;
            //Get the last error from windows for CreateFile
            std::cout << "Error code: " << GetLastError() << std::endl;
        }

        // Yes, this buffer seems useless - but without it the driver BSOD's the PC.
        //Create a buffer to have data returned to.
        void* buf = (void*)malloc(4096);

        //Call IOCTL that sets the PID variable and gets past the DWORD check
        //0x9e6a0594 - IOCTL Code
        BOOL success = DeviceIoControl(hDevice, 0x9e6a0594, NULL, NULL, buf, 4096, NULL, NULL);
        if (!success) {
            std::cout << "DeviceIOControl 0x9e6a0594 failed!" << std::endl;
            std::cout << "Error code: " << GetLastError() << std::endl;

            CloseHandle(hDevice);
            return;
        }

        //We don't need that buffer anymore
        free(buf);
    }

    ~DriverInterface() {
        CloseHandle(hDevice);
    }

    // Next, get a HANDLE to the desired process through the driver.
    HANDLE get_handle_for_pid(DWORD pid) {
        // IOCTL Code - 0xe6224248

        k_get_handle param{};
        // Process ID to get handle for
        param.pid = pid;

        // Access to be granted on the returned handle
        param.access = GENERIC_ALL;

        // Do DeviceIoControl call
        BOOL success = DeviceIoControl(hDevice, 0xe6224248, &param, sizeof(param), &param, sizeof(param), NULL, NULL);
        if (!success) {
            std::cout << "DeviceIOControl 0xe6224248 failed!" << std::endl;
            std::cout << "Error code: " << GetLastError() << std::endl;
            return INVALID_HANDLE_VALUE;
        }

        // Return the handle given by the driver.
        return param.handle;
    }

    // A simple function to read memory using the driver.
    BOOL read_memory_raw(void* address, void* buf, size_t len, HANDLE targetProcess) {
        k_param_readmem req{};
        req.fromAddress = (void*)address;
        req.length = len;
        req.targetProcess = targetProcess;
        req.toAddress = (void*)buf;

        BOOL success = DeviceIoControl(hDevice, 0x60a26124, &req, sizeof(k_param_readmem), &req, sizeof(k_param_readmem), NULL, NULL);
        return success;
    }

    void Shutdown() {
        CloseHandle(hDevice);
   }
};