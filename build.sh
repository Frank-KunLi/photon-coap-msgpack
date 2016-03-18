#!/bin/bash
CWD=$(pwd)
( cd /data/c018/work/particle/firmware/; make PLATFORM=photon APPDIR="$CWD" v=1 )

