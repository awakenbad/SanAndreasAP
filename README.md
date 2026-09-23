# GTA: San Andreas - Archipelago

<img width="1200" height="287" alt="sanandreasap short" src="https://github.com/user-attachments/assets/798c85bf-3415-4400-b931-c2777475073a" />

An [Archipelago](https://archipelago.gg) multiworld randomizer for Grand Theft
Auto: San Andreas (PC, v1.0). Story missions are gated behind Progressive
Mission items; collectibles, Ammu-Nation purchases, and side activities send
checks to the multiworld.

> [!NOTE]
> All regions and planned side-activities are randomized (Big Smoke → End of the Line), however the mod still remains unstable. 
> Feel free to bug report either through `Issues` or using [GTA SA Thread](https://discord.com/channels/1085716850370957462/1098055987648282717) in Archipelago After Dark

## Related repositories

The randomizer logic (locations, items, rules) is developed in my
[Archipelago fork](https://github.com/awakenbad/Archipelago) under
`worlds/gta_sa` — that's where `gta_sa.apworld` comes from. This repo only has
the C++ mod.

## Current scope (v0.9.0)

- **Goal:**
    - **The Green Sabre** (Los Santos only)
    - **Are You Going to San Fierro?** (Los Santos → Badlands)
    - **Yay Ka-Boom-Boom** (Los Santos → San Fierro)
    - **A Home in the Hills** (Los Santos → Las Venturas)
    - **End of the Line** (Los Santos → Return to Los Santos)
    - **100%** (All story missions and side-activities)
- 100 story missions across all regions
  gated by Progressive Mission items
- Choosing a starting point (Badlands, San Fierro, Las Venturas)
- Spray tags, snapshots, horseshoes, oysters and stunt jumps as individual locations (toggleable)
- Submissions sending checks per level (tunable):
    - Paramedic, Firefighter and Vigilante (12 levels each)
    - Taxi Driver (one per fare)
    - Burglary (one per $1000 stolen) etc.
- Ammu-Nation sells multiworld items.
- Taking over gang territories in RTLS sends checks per 5%
- Bike Challenges, Stadium Events, Courier missions, Street Races, Gun Challenges send checks.
- Traps:
    - Wanted Level
    - Fat CJ
    - Flat Tires
    - Car Fire etc.
- Filler items:
    - Weapons
    - Full Armor
    - Car Repair
    - Weapon Skill etc.
- Autosaving to slot 8 after every story mission.
- Universal Tracker tab in `GTA SA Client`
- Deathlink support
- Fast Travel markers

## Setup

### Prerequisites

- Grand Theft Auto: San Andreas **v1.0** — If you own Steam version: watch
  [this video](https://youtu.be/sAgqTbCScmQ?si=OrUAvIKqTjmyFgJl&t=158)
  (2:39–8:27) to downpatch
- An ASI Loader — any should work; tested with
  [Silent's ASI Loader](https://github.com/GTAmodding/ASI-Loader/releases/tag/v1.3.0)
- ~~[ThirteenAG's Widescreen Fix](https://gtaforums.com/topic/547841-gtanfsmpother-widescreen-fixes-pack/)~~ (not required anymore)
- [Archipelago](https://github.com/ArchipelagoMW/Archipelago/releases) 0.6.7+

### Install

1. Download the `SanAndreasAP-vX.X.X.zip` and `gta_sa.apworld` from the latest
   [release](https://github.com/awakenbad/SanAndreasAP/releases).
2. Install `gta_sa.apworld` by double-clicking it (a popup should appear confirming the
   install), then restart the Archipelago Launcher.
3. Extract contents of `SanAndreasAP-vX.X.X.zip` into the `scripts` folder inside your GTA San
   Andreas installation directory. Create the folder if it doesn't exist.
4. If you haven't already, create a `.yaml` file:
   - In the Archipelago Launcher, open **Options Creator**
   - Find **Grand Theft Auto: San Andreas**
   - Tweak the settings, enter your slot name at the top, and press
     **Export Options**
5. In the Archipelago Launcher, open **GTA SA Client** (it comes with the
   `.apworld` you installed in step 2), then type `/connect <address>` to connect
   it to your Archipelago server.
6. Launch the game.

**To verify:** the bottom-left of the main menu shows
`Archipelago: Connected` in green once the game is linked to the client.

### Recommended (optional) mods
Not required, but they improve the experience:
- [SilentPatch](https://gtaforums.com/topic/669045-silentpatch/) - a large collection of bug
  fixes, including mouse input not being detected.
- [FramerateVigilante](https://www.moddb.com/downloads/iiivcsa-framerate-vigilante) - fixes the
  framerate-dependent bugs that show up with the frame limiter off, so the game works correctly at 60 FPS.
- [WindowedMode](https://github.com/ThirteenAG/III.VC.SA.WindowedMode) - windowed and borderless
  modes, which makes alt-tabbing to the Archipelago client much less painful.

> [!WARNING]
> **CLEO 4** and older completely break the mod.
>
> If you already run CLEO for other mods, it has to be [CLEO 5](https://cleo.li/) (tested with 5.4.0)

### Using Starting point save files
Starting point saves now live inside `scripts/Archipelago`, leave them there. On a non Los Santos start they will appear in the load list.

## How does this mod work?

- Story missions require Progressive Mission items. When you run out, every
  mission start marker is greyed out and blocked until you unlock more.
- Collectibles, Ammu-Nation purchases, and side-activity levels all send
  checks. The Ammu-Nation menu shows what each slot contains.
- Collectible locations appear on the radar and map. Press **F8** in the pause menu to toggle them off
- Client commands `/tag <1-100>`, `/snapshot <1-50>` etc. highlight a specific collectible on the radar so you
  can find it from anywhere.
- A respawning spray can sits outside CJ's house.
- A respawning camera sits outside Doherty Garage.
- A respawning jetpack sits outside Four Dragons Casino.
- The game autosaves to **slot 8** ("Autosave: [mission]") after every story
  mission and side-activity completion.
- On-screen notifications show items you receive and checks you send to other
  players.

## Known issues

- Autosave titles longer than ~22 characters are truncated.
- The Caligula's Casino Progressive Mission counter is misaligned.
- CLEO 4 or older breaking most features of the mod.

## Troubleshooting

- **"Archipelago: Disconnected" in the menu** — the **GTA SA Client** isn't
  running. Open it from the Archipelago Launcher; the game reconnects
  automatically within a few seconds.

## Notes

- Save files and archipelago progress are stored in `GTA San Andreas User Files/archipelago`.
- The client must be running before checks can be sent; anything you complete
  while it is disconnected is sent automatically once it reconnects.

## Support
If you ever feel like supporting the project, there's a Sponsor button at the top of the
repo. It's completely optional and not required for the mod's development.

## Roadmap
- All missions startable from `In The Beginning`

## Credits

- [Archipelago](https://archipelago.gg) project and contributors
- [plugin-sdk](https://github.com/DK22Pac/plugin-sdk) — the modding SDK this
  mod is built on
- [Rainbomizer](https://github.com/Parik27/SA.Rainbomizer) — mission starting positions and autosave approach
- [gta-reversed](https://github.com/gta-reversed/gta-reversed) and the
  [GTAMods wiki](https://gtamods.com) — save format and memory research
- [Sanny Builder's map tool](https://gtag.sannybuilder.com/maps/gtasa/) —
  spray tag coordinates
- Wafflejunkie — starting point save files
- Lordmau5 — help with various parts of the project
