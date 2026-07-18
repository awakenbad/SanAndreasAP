# Setup Guide
## Prerequisites:
* Grand Theft Auto: San Andreas V1.0
* ASI Loader
* Widescreen Fix (Some text will not scale correctly without it currently)
* Archipelago Launcher

### GTA SA V1.0
If you have Steam version of the game, watch [this](https://youtu.be/sAgqTbCScmQ?si=OrUAvIKqTjmyFgJl&t=158) (2:39-8:27) video on how to downpatch to 1.0

### ASI Loader
Any should work, I used [Silent's ASI Loader](https://github.com/GTAmodding/ASI-Loader/releases/tag/v1.3.0)

### Widescreen fix
Here's the link for [ThirteenAG's Widescreen Fix](https://gtaforums.com/topic/547841-gtanfsmpother-widescreen-fixes-pack/)

## Next steps

1. Head to the latest [release](https://github.com/awakenbad/SanAndreasAP/releases) and download the `.zip` file
2. Install the `gta_sa.apworld` file by double-clicking on it (a pop up should appear saying that it was installed)
3. Put `Archipelago.SA.asi` into `[Your game folder path]/Grand Theft Auto: San Andreas/scripts.` If this folder doesn't exist, create it.
4. If you haven't already, create a `.yaml` file
    * In Archipelago Launcher, look for `Options Creator`
    * Find `Grand Theft Auto: San Andreas`
    * Tweak the settings to your liking
    * Enter slot name at the top (you will need it to connect to the game) and press `Export Options`
5. Run `GTASAClient.exe` and connect to your Archipelago server
6. Launch the game. Bottom-left corner should say `Archipelago: Connected`, once the game is linked to the client.

## How does this mod work?
* Story missions require Progressive Mission items. Once you are out of Progressive Missions every mission marker becomes blocked until you unlock more.
* Spray tags, Ammu-nation purchases, submissions and more send checks.
* Tags are shown on the map and radar and can be toggled off using F8
* Client command /tag <1-100> highlights a specific tag on the radar
* A respawning spray can with plenty of paint sits outside CJ's house.
* There are also traps (i.e. increase wanted level) and filler items (guns, money, etc.)

## Currently implemented features
* Every Los Santos Mission (Big Smoke -> The Green Sabre)
* All tags are locations.
* Submissions randomized:
    * Paramedic
    * Vigilante
    * Firefighter
    * Taxi Driver
    * Burglary
    * LS Gym Boxing
* Ammu-nation sells checks
* Traps:
    * Increase Wanted Level
    * CJ becomes fat (Timer)
    * All vehicles CJ sits in pop their tires (Timer)
    * Next car CJ sits in will catch fire
* Fillers:
    * Every weapon
    * Every throwable
    * Repair current car
    * Full armor
* Deathlink

## Notes
- Archipelago progress is stored next to your GTA save files as `<savename>_ap.dat` -
  if you move or back up saves, keep those files together.
- The client must be running before checks can be sent; anything you complete while
  it is disconnected is sent automatically once it reconnects (within the same game
  session).
