# anbernic_favorites_tool
Tool for shuffling or sorting the favorites list on Linux-based Anbernic consoles like the RG35XX-H.

Not affiliated with or endorsed by Anbernic.

> [!WARNING]  
> The favorites file is modified in-place; no backups are made.
> Use at your own risk!

## Install
Unzip [anbernic_favorites_tool.zip](https://github.com/atdt/anbernic_favorites_tool/releases/latest/download/anbernic_favorites_tool.zip) to the Roms/APPS folder on either SD card of your Anbernic console:

```sh
unzip anbernic_favorites_tool.zip -d /path/to/SD/Roms/APPS
```

Find 'Shuffle favorites' and 'Sort favorites' under **App Center** > **APPS**.

The scripts exit immediately without providing feedback.

## Build
You will need a cross-compiler for 32-bit Arm (armhf). On Debian systems, you can run:

```sh
sudo apt-get install g++-arm-linux-gnueabihf
```

Then, to build the tool, simply run `make`.

## Alternatives
* [FavSort](https://github.com/Skeeve/FavSort)

## Favorites file format
The location of the favorites file is `/mnt/data/misc/.favorite`.

### Format
- The first line must be `Version=1`.
- Subsequent lines are entries in the favorites list.
- The last line is a [JAMCRC](https://reveng.sourceforge.io/crc-catalogue/all.htm#crc.cat.jamcrc) checksum computed over the above content, including the trailing newline.

### Example Python code

```python
import struct
import zlib

data = b"Version=1\nApotris-v4.0.2GBA.zip:GBA:nul:3:1:0\n"
crc = 0xffffffff - zlib.crc32(data, 0xffffffff)
checksum = struct.pack('<I', crc)
with open('foo', 'wb') as f:
  f.write(data)
  f.write(checksum)
```
