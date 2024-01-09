![image-removebg-preview1](https://github.com/kite03/echoac-poc/assets/67329371/9c5c89ca-d0b4-443c-84a2-97f94fb85657)

# CVE-2023-38817
## A PoC and writeup on vulnerabilties discovered in echo.ac's driver.

# CVE Info
- Number: CVE-2023-38817
- Vendor: Inspect Element Ltd (13017981), trading as Echo.
- Affected Products: echo.ac AntiCheat scanner tool.
- Affected Versions: echo.ac - <5.2.1.0, echo_driver.sys - All shipped versions.
- Affected operating systems: 64Bit versions of Windows from; Windows 7 to Windows 11.
- Mitigation: Do not use the software, and add driver signatures to blacklist.

## CERTIFICATE REVOKED

Microsoft has added the Echo Driver to the Vulnurable Driver Blocklist and the certificate has been revoked (even after the Echo team insisted that the exploit wasn't real).

If you still wish to use the exploit, you must enable test signing and disable the Microsoft Vulnurable Driver blocklist.

# 💕 Credits

- Whanos: https://github.com/Whanos/ - Initial discovery, first contact with echo.ac, writing.
- Lemon (Wishes to stay anonymous) - Exploit development and assistance.
- kite03: https://github.com/kite03/ - Exploit development, and writing.

# Detailed Writeup Link
https://ioctl.fail/echo-ac-writeup/

# Driver Download
I have removed the binary from this repo for security.

You may read extra info and download the driver binary from the official loldrivers.io page: https://www.loldrivers.io/drivers/afb8bb46-1d13-407d-9866-1daa7c82ca63/

## Background
echo.ac is a commercial "screensharing tool", marketed and developed mostly for the Minecraft PvP community, but also used by some other game communities, such as Rust.
A "screensharing tool" is a program developed to "assist" server admins in identifying if someone's using cheats or similar banned external tools ingame - As such, these programs execute numerous intrusive scans on users computer, while being very vague of what they data collect and why.

Echo refused to acknowledge this bug when contact was made.

Thanks for your time 💜.
