# HexAlter

Hexalter v0.1
-------------

This is a quick (but hopefully not dirty) implementation of a primitive hex alterer.  Usage
is very simple:

hexalter file_to_edit address1=byte1,..,byten ... addressn=byte1,...,byten

For example:

hexalter foo.txt 0x4=1,2,3 0x20=0xa0 0xff=4

Changes:

address | new value
-------------------
4       | 1
5       | 2
6       | 3
32      | 160
255     | 4

If addresses overlap, the earlier changes will overwrite the later changes.  You probably
shouldn't rely upon this feature, though.

If address is out of range or byte values are too high/low or address or byte values aren't
entered in decimal or hexademical, hexalter should refuse to patch.

If during the actual apply phase some are happens, only some changes might be applied.
WW