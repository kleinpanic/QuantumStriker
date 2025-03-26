# TODO

## High Score Authentication & Leaderboards

- [X] Replace plaintext high score files with PGP-signed or otherwise tamper-resistant storage to ensure scores are achieved authentically.
- [X] Add encryption algorithms and block-chain technology as well as pgp digital signatures to ensure authenticity.
- [ ] Encorporate game settings as part of the encryption, and block-chain technology to ensure that game settings can be reproduced if the user manually changes them.
- [ ] Detere more advanced cheating by trying to create a distributed blockchain, and network-wide consensus to prevent tampering, enhance the PoW slightly (but not to an extent to use too much CPU resources). Current implementation deteres causal score manipulation (requires correct signature and valid PoW, but not immune to file modifications.) 
    - Truly, 100% prevention of cheating is impossible, and there are limitations with open-sourc, and cheating-prevention.  
- [ ] Explore implementing online leaderboards for global competition.

## Start Menu & Additional UI Features

- **Main Menu Screen:**
  - Implement a visually appealing start menu displayed on launch.
  - Include buttons for:
    - **Start Game:** Begin the main gameplay loop.
    - **Help:** Show instructions and controls.
    - **Quit:** Cleanly exit the application.
  - Animate menu transitions and background effects to enhance user experience.

- **Game Modes:**
  - **Classic Mode:** Standard gameplay with adaptive difficulty.
  - **Arcade Mode:** Timed challenges or endless waves with score multipliers.
  - **Practice Mode:** A relaxed mode to practice controls without enemy pressure.
  - Allow selection of game modes via the start menu.

- **Additional UI Enhancements:**
  - Include settings for volume, key bindings, and difficulty adjustments.
  - Create polished overlays for in-game pause, help, and game over screens.
  - Consider animated transitions and sound effects to create a more professional, app-like experience.

- **Future Improvements:**
  - Integrate a settings/configuration file for user customization.
  - Integrate a settings button to interact with this file?
  - Add a credits screen for contributors.
  - Implement achievements and unlockables tied to game performance.


## Enhanced Enemy AI

- [ ] Implement multiple enemy types with unique behaviors (e.g., enemies that shoot back, faster enemies).
- [ ] Scale enemy difficulty further as the score increases (including health, speed, and spawn frequency).
- [ ] Introduce advanced enemy strategies such as coordinated attacks and evasive maneuvers.

## User Interface Improvements

- [ ] Refine the dynamic background with additional galactic elements (e.g., more stars, varied planet types, animated cosmic effects).
- [ ] Improve HUD visuals and animations (e.g., smoother transitions, real-time FPS display in debug mode).
- [X] Implement a graphical username prompt within the game window (instead of terminal input).
- [ ] Implement a shake feature when user takes damage
- [ ] Implement explosion animation for enemies and user on death
- [ ] implement a settings screen.
- [ ] Implemenet a screen to load a game or a screen option after the start option.
- [ ] Make a prettier Ui for the username entry section

## Input and Control Enhancements

- [X] Ensure WASD and arrow keys always move and rotate the ship relative to its tip as intended.
- [ ] Implement customizable key bindings via a configuration file.
- [ ] Improve control responsiveness and refine physics.
- [ ] Implement a mechanism for dynamic size changes of the ship with the up and down arrows [add restrictions]? 

## Cross-Platform Enhancements

- [ ] Ensure smooth compilation and execution on Windows, Linux, and macOS.
- [ ] Create platform-specific build scripts if needed.

## Code Quality and Testing

- [ ] Add unit tests for core game components.
- [ ] Integrate continuous integration (CI) via GitHub Actions.
- [ ] Improve error handling and logging throughout the codebase.

## Additional Features

- [ ] Implement sound effects and background music.
- [ ] Explore implementation of saving and reloading saved game files.
- [X] Replace current kill operation for q and esc with one that saves the data. 
- [ ] Explore multiplayer or cooperative modes.
- [X] Add a `--debug` flag that, when enabled, displays additional debug information (to be implemented).
- [ ] Add a `--log` flag that, when enabled, logs all of the debug info instead of shows it
    - Integrate a mechanism for logging. --debug level 3 should auto activate logging [future improvement]. 
    - the log flag should be used in conjunction with the --debug flag to define how indepth these logs should be. By default, if its not used with --debug, its at level 2. --log can also be used to specify the path of the log file but by default it will be in /tmp/. 
- [ ] Improve `--debug` flag by adding a mechanism for level 3 that would add more info about like which file the error is occuring in [either defined by an internal variable or a hasmap in debug.h that needs to be maintained]. Additionally every debug should have 3 debugs for each level. Improve debugging.
- [ ] Add a `--fullscreen` flag to launch the game in fullscreen mode (to be implemented).
- [ ] Add a `--testing` flag and a `--development` flag for testing the program and stress testing it and edge casing it.
    - Development flag would allow for a "cheated" mode that contributes to a fake blockchain that isn't pushed but is used to test the full features of the program and make sure its production ready. So for instance, I wouldn't die, i could set the score to whatever i wanted, i can spawn enemies, I can teleport, I want an option to have the game be played by an AI for me to just stress test the game and its resource intensity over time, and for other reasons, maybe also seperate this into its on --ai flag.  
    - --testing flag would be for stress testing in other aspects. Not sure how this would differ from --development flag, but maybe this would used for more edge cases, running unit tests, seeing if they all pass, allowing an automated issues creation option maybe?
- [X] Add a `--highscores` flag to display a table of all high scores.
- [ ] Add a `--config` flag to perminently change configurations from the command line
    - CLi equivalent of the settings button i wanna add to with the start menu. also need to add config file and configuration reading, and maybe use these configs in the blockchain for that standardization / inspection i was talking about [future improvement could be a dynamic weighted algorithm that determines top users based on the configurations like enemy speed, user speed, infinite bullets, maybe if there is a bullet cooldown, etc] 
- [X] Support command-line flags such as `--version` and `--help` for better usability.


## Core logic additions 

- [ ] Add a way to save game state, and load game state, and have it work with the blockchain.
- [ ] Incorporate key game settings and configurations from the config file [like health, speed, enemy speed, enemy difficulty, etc] into the blockchain and add that dynamic algorithm mentioned elsewhere.
- [ ] Make a python file that maintains the blockchain, or figure out a way to do this, the point of the program is to future proof the block chain so that past user data isn't lost with blockchain logic additions as it might take me a while to implement the blockchain configuration additions.
- [ ] Make a python file that makes sure the change log and the version match, maybe use a small AI, or some online resource, that is able to automate changelog additions [this is kinda bloated and i don't think its a good addition for such a minute aspect of the project]
- [ ] Mimic decentralization with backup / various repositioes using different services like gitea, gitlabs, and github, or various remotes on one platform. Have a mechanism to regularly validate the blockchain against eachother, always ensuring decrenctralization, and ensuring validity and authenticity of the blockchain, and prevents some asshole from deleting the blockchain and pushing the changes. Can use my dell server as a central origin node w bb. Offer users to become part of the network on startup of the program or have it be a config option. 



## Documentation

- [ ] Expand in-code documentation and comments.
- [X] Maintain and update README.md, CONTRIBUTING.md, and CHANGELOG.md with project progress and new features.

