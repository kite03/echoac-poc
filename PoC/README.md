# Example PoCs

# Download Driver
I have removed the driver binary from this repo for security. 

You may download the driver binary from the official loldrivers.io page, as well as read extra information about the driver file itself:
https://www.loldrivers.io/drivers/afb8bb46-1d13-407d-9866-1daa7c82ca63/

Driver SHA256 Hash: `ea3c5569405ed02ec24298534a983bcb5de113c18bc3fd01a4dd0b5839cd17b9`

## PrivilegeEscalation/
This PoC demonstrates a Privilege Escalation exploit, by stealing the Kernel's KPROCESS/EPROCESS block and writing it to a newly spawned shell to elevate its privileges to the highest possible - nt authority\system.

It may be too technical for some to understand, but I have tried to comment it well.


Have fun! 💜
