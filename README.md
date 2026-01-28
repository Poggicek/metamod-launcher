# Metamod Launcher

Alternative way to load metamod without having to modify game files. If you get annoyed by the need to constantly change gameinfo to switch between metamod and normal gameplay, this fixes that.

## Usage

### Windows

- extract the contents of the [zip file](https://github.com/Poggicek/metamod-launcher/releases/latest/download/launcher.zip) to the `game/bin/win64` folder where `cs2.exe` is located
- if you just launch `metamod-launcher.exe`, a dialog window will open prompting you to run the dedicated server or the client with insecure, if you launch the app with command line arguments, it will pass them to `cs2.exe` directly and not open a dialog window (must include -insecure or -dedicated)