# anbernic_favorites_tool
Tool for shuffling or sorting the favorites list on Anbernic consoles.
Not affiliated with or endorsed by Anbernic.

## Install
Run `make`. Unzip `anbernic_favorites_tool.zip` to your console's app folder.

## Favorites file format
The location of the favorites file is `/mnt/data/misc/.favorites`.

### Format
- The first line must be Version=1.
- Subsequent lines are entries in the favorites list.
- The last line is a JAMCRC checksum computed over the above content.

## License
Copyright 2024 Ori Livneh <ori.livneh@gmail.com>
MIT license.
