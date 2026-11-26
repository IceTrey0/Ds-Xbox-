# melonxbox
melonDS-based Nintendo DS emulator for Xbox Dev Mode (UWP). GPL-3.0.

## Setup
    git init && git submodule add https://github.com/melonDS-emu/melonDS extern/melonDS

## Saves
- Battery saves: raw `.sav`. DraStic `.dsv` files are imported via
  `NormalizeSave()` (strips footers; rejects sizes that aren't valid).
- Save states: own format (`.mxs`, 10 slots/game). DraStic `.dss` states are
  NOT compatible (proprietary, different core).

## TODO
1. Wire StateBackend to melonDS `Savestate` (match pinned version's API)
2. UWP frontend (D3D11, Windows.Gaming.Input, XAudio2, file picker)
3. MSIX packaging step in the workflow
