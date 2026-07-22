# GTA San Andreas Archipelago

An [Archipelago](https://archipelago.gg) multiworld randomizer for Grand Theft
Auto: San Andreas (PC, v1.0). Story missions are gated behind Progressive
Mission items; spray tags, Ammu-Nation purchases, and side activities send
checks to the multiworld.

> [!NOTE]
> Los Santos and Badlands are randomized so far (Big Smoke → Are You Going
> to San Fierro?). The rest of the game will come in future updates. Feel free to bug report either through `Issues` or using [GTA SA Thread](https://discord.com/channels/1085716850370957462/1098055987648282717) in Archipelago After Dark

## Related repositories

The randomizer logic (locations, items, rules) is developed in my
[Archipelago fork](https://github.com/awakenbad/ArchipelagoSA) under
`worlds/gta_sa` — that's where `gta_sa.apworld` comes from. This repo only has
the C++ mod.

## Current scope (v0.2.0)

- **Goal:** **The Green Sabre** (Los Santos only) or
  **Are You Going to San Fierro?** (Los Santos + Badlands).
- 27 Los Santos story missions, plus 9 Badlands missions,
  gated by Progressive Mission items
- 100 spray tags as individual locations (toggleable)
- Side activities, paying out **per level** rather than once at the end:
  Paramedic, Firefighter and Vigilante (12 levels each), Taxi Driver (one per
  5 fares), Burglary (one per $1000 stolen), Trucking (8 levels, Badlands),
  and LS Gym Boxing
- 8 Ammu-Nation shop slots that sell checks (the menu shows what each
  purchase contains)
- Traps (wanted level, fat CJ, flat tires, car fire) and filler items
  (21 weapons, money, full armor, car repair)
- DeathLink
- Autosave to slot 8 after every story mission and side-activity completion

## Setup

### Prerequisites

- Grand Theft Auto: San Andreas **v1.0** — If you own Steam version: watch
  [this video](https://youtu.be/sAgqTbCScmQ?si=OrUAvIKqTjmyFgJl&t=158)
  (2:39–8:27) to downpatch
- An ASI Loader — any should work; tested with
  [Silent's ASI Loader](https://github.com/GTAmodding/ASI-Loader/releases/tag/v1.3.0)
- [ThirteenAG's Widescreen Fix](https://gtaforums.com/topic/547841-gtanfsmpother-widescreen-fixes-pack/)
  (some text does not scale correctly without it)
- [Archipelago](https://github.com/ArchipelagoMW/Archipelago/releases) 0.6.7+

### Install

1. Download the `.zip` from the latest
   [release](https://github.com/awakenbad/SanAndreasAP/releases).
2. Install `gta_sa.apworld` by double-clicking it (a popup should appear confirming the
   install), then restart the Archipelago Launcher.
3. Copy `Archipelago.SA.asi` into the `scripts` folder inside your GTA San
   Andreas installation directory. Create the folder if it doesn't exist.
4. If you haven't already, create a `.yaml` file:
   - In the Archipelago Launcher, open **Options Creator**
   - Find **Grand Theft Auto: San Andreas**
   - Tweak the settings, enter your slot name at the top, and press
     **Export Options**
5. Run `GTASAClient.exe`, then type `/connect <address>` to connect it to your
   Archipelago server.
6. Launch the game.

**To verify:** the bottom-left of the main menu shows
`Archipelago: Connected` in green once the game is linked to the client.

## How does this mod work?

- Story missions require Progressive Mission items. When you run out, every
  mission start marker is physically blocked until you unlock more.
- Spray tags, Ammu-Nation purchases, and side-activity levels all send
  checks. The Ammu-Nation menu shows what each slot contains; already-checked
  slots revert to selling their normal weapons.
- Tag locations appear on the radar and map (nearest ones only, to keep the
  radar readable). Press **F8** in the pause menu to toggle them off
- Client command `/tag <1-100>` highlights a specific tag on the radar so you
  can find it from anywhere.
- A respawning spray can with plenty of paint sits outside CJ's house.
- The game autosaves to **slot 8** ("Autosave: [mission]") after every story
  mission and side-activity completion.
- On-screen notifications show items you receive and checks you send to other
  players.

## Known issues

- Some text may not scale correctly without Widescreen Fix installed.
- Autosave titles longer than ~22 characters are truncated.

## Troubleshooting

- **"Archipelago: Disconnected" in the menu** — `GTASAClient.exe` isn't
  running. Start it; the game reconnects automatically within a few seconds.

## Notes

- Archipelago progress is stored next to your GTA save files as
  `<savename>_ap.dat`. If you move or back up saves, keep those files
  together.
- Because that progress lives with the save, loading an older save correctly
  re-grants the items it never received, and restarting the client does not
  hand you everything a second time.
- The client must be running before checks can be sent; anything you complete
  while it is disconnected is sent automatically once it reconnects.

## Roadmap

- San Fierro and Las Venturas regions with their missions and side activities
- All checks in every region
    - Collectibles
    - Schools
    - Submissions (Train, Quarry)
    - Sidemissions (Zero, Heist)
- More options for customizing seeds
- Native support with Rainbomizer

## Credits

- The [Archipelago](https://archipelago.gg) project and contributors
- [plugin-sdk](https://github.com/DK22Pac/plugin-sdk) — the modding SDK this
  mod is built on
- [Rainbomizer](https://github.com/Parik27/SA.Rainbomizer) — mission ID
  numbering and the autosave approach
- [gta-reversed](https://github.com/gta-reversed/gta-reversed) and the
  [GTAMods wiki](https://gtamods.com) — save format and memory research
- [Sanny Builder's map tool](https://gtag.sannybuilder.com/maps/gtasa/) —
  spray tag coordinates
