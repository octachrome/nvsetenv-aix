Nvsetenv on AIX
===============

The `nvsetenv` program is part of the `powerpc-utils` package for Linux. It allows you to modify variables in the NVRAM. I use it on an IBM eServer pSeries, to change the boot-device variable to point to the AIX disk so I can easily reboot from Linux into AIX without going through the Open Firmware boot menus:

    nvsetenv boot-device /pci@fef00000/scsi@c/sd@a,0

Unfortunately, the corresponding AIX tool to reboot back into Linux, `bootlist`, isn't up to the task. This:

    bootlist -m normal hdisk2

sets `boot-device` to `/pci@fef00000/scsi@c/sd@c:2`, which I think is the Open Firmware device name of the *second partition* of the Linux disk, which is not bootable. I couldn't find a tool to set `boot-device` explicitly (without using the `hdisk2` alias), so I ported `nvsetenv` to AIX.

I've tested it under AIX 4.3 and 5.1. Please tell me if this tool has helped you or if you have any improvements.
