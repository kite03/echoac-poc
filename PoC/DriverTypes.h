#pragma once
#include <Windows.h>
#include <cstdint>

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