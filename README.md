# anbernic_favorites_tool
Tool for shuffling or sorting the favorites list on Anbernic consoles like the RG35XX-H.

Not affiliated with or endorsed by Anbernic.

> [!WARNING]  
> The favorites file is modified in-place; no backups are made.
> Use at your own risk!

## Build
You will need a cross-compiler for armhf. On Debian systems, you can run:

```sh
sudo apt-get install g++-arm-linux-gnueabihf
```

Then, to build the tool, simply run `make`.

## Install
Unzip `anbernic_favorites_tool.zip` to the Roms/APPS folder on either SD card of your Anbernic console:

```sh
unzip anbernic_favorites_tool.zip -d /path/to/SD/Roms/APPS
```

Find 'Shuffle favorites' and 'Sort favorites' in the Apps menu.
The scripts exit immediately and don't provide feedback.

## Favorites file format
The location of the favorites file is `/mnt/data/misc/.favorite`.

### Format
- The first line must be `Version=1`.
- Subsequent lines are entries in the favorites list.
- The last line is a [JAMCRC](https://reveng.sourceforge.io/crc-catalogue/all.htm#crc.cat.jamcrc) checksum computed over the above content, including the trailing newline.
