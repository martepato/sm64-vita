# Super Mario 64 Port

- This repo contains a full decompilation of Super Mario 64 (J), (U), and (E) with minor exceptions in the audio subsystem.
- Naming and documentation of the source code and data structures are in progress.
- Efforts to decompile the Shindou ROM steadily advance toward a matching build.
- Beyond Nintendo 64, it can also target the PS Vita.

This repo does not include all assets necessary for compiling the game.
A prior copy of the game is required to extract the assets.

## Building for the Vita

For running this on the PS Vita you will first need to extract the runtime shader compiler. Follow this [guide](https://samilops2.gitbook.io/vita-troubleshooting-guide/shader-compiler/extract-libshacccg.suprx) for help with that.

Savedata is stored at `ux0:data/sm64_save_file.bin`. `cont.m64` can be put into the same folder for TAS input.

1. Install prerequisites (Ubuntu): `sudo apt install -y git build-essential pkg-config`.
2. Install [VitaSDK](https://vitasdk.org)
3. Clone the repo: `git clone https://github.com/bythos14/sm64-vita.git`, which will create a directory `sm64-vita` and then **enter** it `cd sm64-vita`.
4. Place a Super Mario 64 ROM called `baserom.<VERSION>.z64` into the repository's root directory for asset extraction, where `VERSION` can be `us`, `jp`, or `eu`.
5. Run `./build_vita.sh` to build and install dependencies and build the game.
6. The installable vpk will be located at `build/us_vita/sm64.<VERSION>.f3dex2e.vpk`

## Issues

1. Sound is not great, crackling sometimes occurs.
2. At the start of the game with Princess Peach, when the camera zooms out the skybox seems to disappear. It is only temporary.

## Contributing

If you want to make it better, fork it and make it better. Thanks. 

## Credits

* WOLFwang for making the livearea assets.
* fgsfds for the initial effort towards porting to the vita.
* The original sm64decomp team for this impressive decompilation.
* The sm64-port team for the work towards porting the decomp to other platforms.
* Rinnegatamante for making the vitaGL library, it made this port alot easier.
