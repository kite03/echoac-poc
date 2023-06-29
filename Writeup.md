# A PoC and writeup on vulnerabilties discovered in echo.ac's driver.

## Background
echo.ac is a commercial "screensharing tool", marketed and developed mostly for the Minecraft PvP community, but also used by some other game communities, such as Rust.
A "screensharing tool" is a program developed to "assist" server admins in identifying if someone's using cheats or similar banned external tools ingame - As such, these programs execute numerous intrusive scans on users computer, while being very vague of what they data collect and why.

We will go into more detail about the data scanned by echo.ac and what it's seemingly doing with the data later.

# The Main Memory IOCTL Bug (CVE ID requested)
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

## What does echo do with all this data?
Echo.ac is pretty quiet about the data it collects and why. Their own ToS on their website or their own scanning app does **not** tell us what data is scanned and what exactly they will do with it.\
As of writing, archived here: 
- Website 1: https://cdn.gls.cx/41fb30ea18f9631b
- Website 2: https://cdn.gls.cx/a665190b66c33c66 
- App (Same ToS as website): https://cdn.gls.cx/fec66d0aee8e0206

The only hints we get are the result pages people publicly share in their discord. An example of which can be found here: https://scan.echo.ac/cd36a1b1-9744-41b8-b8b3-83ca9bff1cfd. This software is clearly running many intrusive scans and storing it on echo.ac's servers, while being very unclear about exactly what they store.

Other people have pointed out in my tweet that this level of intrusive scanning poses a serious privacy risk.

![lyrus](https://cdn.gls.cx/480ab37787ff990c)

Especially considering there is the real risk that their could be very sensitive data in the memory of scanned processes, as pointed out again here by other users.

![bulbasaur](https://cdn.gls.cx/e57fcaaef3e93fb6)

**Another very concerning point is that since bulbasaur's tweet was posted, they removed the line in their website about storing the memory of processes on the computer, but yet they still do it!**

At the time of writing, this was their updated "What do we log?" FAQ: ![faq](https://cdn.gls.cx/777c3b6e8dbb01d9)\
Notice the now missing statement on process memory. Shocking.

# Company Response
- On the 29th of June, 2023 I (protocol) contacted the CEO of `Inspect Element Ltd.`(which own echo.ac) about disclosing my findings before going public, His response to me was personally - pretty appalling. Our conversation was as follows (Screenshots).
- Part 1
- ![One](https://cdn.gls.cx/a44490cf0eb17eb0)
- Part 2
- ![Two](https://cdn.gls.cx/18f75e7b3f40e0f8)
- Part 3
- ![Three](https://cdn.gls.cx/d837c73f27054b84)
- Part 4
- ![Four](https://cdn.gls.cx/e2b770497bc06754)
- Finally, And a screenshot of his profile. 
- ![Profile](https://cdn.gls.cx/e62aeb70bb05cf79)
- After this conversation, I was then **banned from their discord server.** for the crime of...trying to help resolve a vulnerability? Wonderful.
- ![Banned](https://cdn.gls.cx/376db515dc954124)
- Overall I felt pretty brushed off by his response, which is disappointing, as I was trying to amicably help resolve this issue, even considering the past interactions echo.ac and his developers had with me (Documented later on).

# My (protocol) previous interactions with echo.ac publicly
On the 24th of May, 2023 I posted a tweet simply sharing some concerns I had with echo.ac, sharing a tria.ge link (https://tria.ge/230524-sdapbsda39/behavioral1#report) and my personal thoughts on echo.ac and screensharing as a whole.

Said tweet is located here: https://twitter.com/WindowsKernel/status/1661424238803156997, screenshotted version here: https://cdn.gls.cx/5c91602e6d41fec2.

At the time, my tweet was only interacted with by my own followers, and did not garner many likes. However, shortly after - The echo.ac CEO (Josh) and some developers + friends of echo.ac responded with what can only really be described as hatemail and bullying. 

Most of these can be viewed as replies on the tweet, but I will screenshot them and post them for archival purposes.
- Firstly, I received this message from Josh, the CEO in my DMs on his personal account. ![Josh DM](https://cdn.gls.cx/3d259502f60b4d18)
- I was then blocked by him on echo.ac's official company account, which I find pretty childish. ![Blocked](https://cdn.gls.cx/a667763dbcbae333)
- I also received copious amounts of hatemail and harassment on the tweet, which was weird as none of these people followed me.
  - ![Reply 1](https://cdn.gls.cx/76b2e38cd4abf81f)
  - ![Reply 2](https://cdn.gls.cx/f8dd33f1ccfb1cf0)
  - ![Reply 3](https://cdn.gls.cx/c7fb67f78f6610ad)
  - This account was inactive for years, and made its first replies to me. ![Reply 4](https://cdn.gls.cx/c5f75a0871cba519)
  - This user owns his own "Screensharing tool". ![Reply 5](https://cdn.gls.cx/a96ec0ef6f8e7e46)
- Here's a list of most of their staff and support team, which you can correlate to some of the replies I received. 
  - ![List](https://cdn.gls.cx/29ca995001be5872)

Quite frankly, echo.ac as a whole is ran very unprofessionally - especially apparent when their own developers, community support, and CEO starts using insults against me as a defence. Lovely.