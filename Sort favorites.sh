#!/bin/bash
SHDIR=$(dirname "$0")
cd "$SHDIR/favorites_tool"
./anbernic_favorites_tool --sort 2>&1 | tee -a log.txt
