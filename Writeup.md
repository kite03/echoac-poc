# A PoC and writeup on vulnerabilties discovered in echo.ac's software.

## Background
echo.ac is a commercial "screensharing tool", marketed and developed mostly for the Minecraft PvP community, but also used by some other game communities, such as Rust.
A "screensharing tool" is a program developed to "assist" server admins in identifying if someone's using cheats or similar banned assistance tools ingame - As such, these programs execute numerous intrusive scans on users computer, while being very vague of what they data collect and why.

We will go into more detail about the data scanned by echo.ac and what it's seemingly doing with the data later.

