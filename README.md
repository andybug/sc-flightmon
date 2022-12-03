# SimConnect Flight Monitor

This is a simple utility to output the position, speed, and direction of a flight using the SimConnect library.
This works with Microsoft Flight Simulator 2020.

The SimConnect libraries and header file were downloaded from MSFS.

## Running in Proton

```bash
export WINEPREFIX=~/.steam/steam/steamapps/compatdata/1250410/pfx
cd lib
WINEFSYNC=1 ~/.steam/steam/steamapps/common/Proton\ -\ Experimental/files/bin/wine64 ../bin/flightmon.exe
```

## Why C?

Because cross compiling in Rust was running in to a lot of problems.
