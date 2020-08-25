# Super Mario 64 Port

- This repo contains a full decompilation of Super Mario 64 (J), (U), and (E) with minor exceptions in the audio subsystem.
- Naming and documentation of the source code and data structures are in progress.
- Efforts to decompile the Shindou ROM steadily advance toward a matching build.
- Beyond Nintendo 64, it can also target the PS Vita.

This repo does not include all assets necessary for compiling the game.
A prior copy of the game is required to extract the assets.

## Building for the Vita
Savedata is stored at `ux0:data/sm64_save_file.bin`. `cont.m64` can be put either into the same folder, or into `ux0:app/SM6400001/` for TAS input.

### Prerequisites
For running this on the PS Vita you will first need to extract the runtime shader compiler. Follow this [guide](https://samilops2.gitbook.io/vita-troubleshooting-guide/shader-compiler/extract-libshacccg.suprx) for help with that.

- Ubuntu: `sudo apt install -y git build-essential pkg-config`.
- Windows (x86_64): `pacman -S git make python3 mingw-w64-x86_64-gcc`

### Build Instructions
1. Install [VitaSDK](https://vitasdk.org)
2. Clone the repo: `git clone https://github.com/bythos14/sm64-vita.git`, which will create a directory `sm64-vita` and then **enter** it `cd sm64-vita`.
3. Place a Super Mario 64 ROM called `baserom.<VERSION>.z64` into the repository's root directory for asset extraction, where `VERSION` can be `us`, `jp`, or `eu`.
4. Building
    1. Run `./build_deps.sh` to build and install dependencies. This only has to be done once.
    2. Run `make TARGET_VITA=1 vpk` to build the game. Add `-j4` to improve build time.
5. The installable vpk will be located at `build/us_vita/sm64.<VERSION>.f3dex2e.vpk`

## Known Issues

1. At the start of the game with Princess Peach, when the camera zooms out the skybox seems to disappear. It is only temporary.
2. The fade transition from the star select menu seems to have a minor bug, the white background disappears as it fades into white.
3. TAS Input stops working if the app is suspended or the Vita is put to sleep.

## Contributing

If you want to make it better, fork it and make it better. Thanks. 

## Credits

* WOLFwang for making the livearea assets.
* fgsfds for the initial effort towards porting to the vita.
* The original sm64decomp team for this impressive decompilation.
* The sm64-port team for the work towards porting the decomp to other platforms.
* Rinnegatamante for making the vitaGL library, it made this port alot easier.
