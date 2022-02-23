# Minimig firmware - backported from MiST

I believe this firmware was backported to the original Minimig from the
MiST firmware, by the user "minimig_emu" on the Minimig.net forums.  Since 
the forum is long gone and the download doesn't appear to have been preserved
by the Wayback Machine at archive.org, I'm uploading it here to avoid it
being lost in the mists of time.

Compared with the stock firmware, it has the "new look" which is now common
between MiST, Turbo Chameleon 64 and MiSTer - and has partial support for
WinUAE-style HDFs without an RDB, as well as the ability for the Amiga to
access the SD card at the block level, allowing an SD card partition to be
used as an Amiga drive, and allowing the FAT partition to be accessed with
FAT95.  (Note: these features have all received upstream bugfixes in the
intervening years, which are not currently present here.)

Includes a couple of build fixes to make it compile cleanly with the
arm-none-eabi-gcc in Ubuntu's repos - but not tested.

