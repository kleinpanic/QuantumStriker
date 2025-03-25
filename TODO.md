# TODO

## High Score Authentication & Leaderboards

- [ ] Replace plaintext high score files with PGP-signed or otherwise tamper-resistant storage to ensure scores are achieved authentically.
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

## Input and Control Enhancements

- [X] Ensure WASD and arrow keys always move and rotate the ship relative to its tip as intended.
- [ ] Implement customizable key bindings via a configuration file.
- [ ] Improve control responsiveness and refine physics.

## Cross-Platform Enhancements

- [ ] Ensure smooth compilation and execution on Windows, Linux, and macOS.
- [ ] Create platform-specific build scripts if needed.

## Code Quality and Testing

- [ ] Add unit tests for core game components.
- [ ] Integrate continuous integration (CI) via GitHub Actions.
- [ ] Improve error handling and logging throughout the codebase.

## Additional Features

- [ ] Implement sound effects and background music.
- [ ] Explore multiplayer or cooperative modes.
- [ ] Add a `--debug` flag that, when enabled, displays additional debug information (to be implemented).
- [ ] Add a `--fullscreen` flag to launch the game in fullscreen mode (to be implemented).
- [ ] Add a `--highscores` flag to display a table of all high scores.
- [ ] Support command-line flags such as `--version` and `--help` for better usability.



## Documentation

- [ ] Expand in-code documentation and comments.
- [ ] Maintain and update README.md, CONTRIBUTING.md, and CHANGELOG.md with project progress and new features.

