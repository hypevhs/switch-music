# switch-music
Music player for the Nintendo Switch using libmikmod

## Build

1. Install [devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman)
1. Install `switch-libmikmod`
1. `make`
1. Copy and run the NRO file using hbmenu

## Game-Music-Emu

See the `gme` branch for a game-music-emu demo (requires a switch port of that lib. I have a WIP one at the repo `libjared/game-music-emu`)

## Deprecated stuff

libnx got rid of the `gfx*` functions, so this doesn't compile on recent versions of libnx, you have to install an older version
