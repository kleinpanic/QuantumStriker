# Changelog

This document chronicles all major changes and improvements made to QuantumStriker. Each version entry reflects significant milestones, feature enhancements, and fixes—told from the perspective of our journey in crafting a unique, blockchain‑backed space shooter.

The format is based on [Keep a Changelog](https://keepachangelog.com/), with sections for Added, Changed, Fixed, etc., for each version. This project adheres to semantic versioning as much as possible in its 0.x development phase.

---

## [0.1.9] - 2025-03-28  
*Enhanced Shooter AI, Fullscreen Mode, & New Visual Effects*

### Added
- **Fullscreen & Resizable Window Support:**  
  You can now play QuantumStriker in fullscreen. A new global flag `g_fullscreen` (defined in `config.h`) controls this. Launch the game with `--fullscreen` to enable it. Internally, if `g_fullscreen` is set, the game uses `SDL_WINDOW_FULLSCREEN_DESKTOP` when creating the window. The game also adjusts `screen_width` and `screen_height` to the display’s dimensions upon entering fullscreen. If not in fullscreen, the window remains resizable (allowing you to adjust the window size manually).

- **Camera Shake Effect:**  
  To intensify feedback when the player takes damage, we introduced a camera shake. Two new variables, `shakeTimer` and `shakeMagnitude`, govern the effect. Whenever the player is hit by an enemy bullet or collides with an enemy (without shields), the game sets `shakeTimer = 20` frames and `shakeMagnitude = 10.0f`. During these frames, the camera position is randomly offset by up to `shakeMagnitude` pixels in X and Y, creating a brief screen shake. The shake diminishes as `shakeTimer` counts down each frame. This effect makes getting hit feel more impactful without affecting gameplay mechanics.

- **Explosion Animations for Enemies:**  
  Enemies now explode with a visual effect when destroyed by the player’s bullets. On enemy death by bullet, an `Explosion` instance is created at the enemy’s position. The explosion is rendered as an expanding orange circle (using SDL2_gfx’s `filledCircleRGBA`) that fades out over 30 frames. Each explosion starts with a small radius (5px) and grows by ~1px per frame, while its opacity decreases proportionally. This gives a quick flash and fade of an explosion. (Enemies destroyed by ramming the player do not produce explosions, since the effect is meant to reward shooting.)

- **New Enemy Types in Code (Preparation):**  
  While not all fully utilized in gameplay, the code now defines enums and handling for up to 10 enemy types, including `ENEMY_SPLITTER`, `ENEMY_STEALTH`, and three boss types (`ENEMY_BOSS1/2/3`). This lays groundwork for future expansions (e.g., stealth enemies that vanish periodically, splitter enemies that break into smaller ones, bosses with unique patterns). In this version, bosses can spawn when score ≥ 2000 with low probability, but their behavior is basic (just high health and possibly larger size).

### Improved
- **Shooter Enemy AI:**  
  The shooter-type enemies are significantly smarter and more dynamic in v0.1.9. Previously, shooters maintained roughly ~75 units distance with a ±10° firing inaccuracy. Now, a shooter continuously adjusts its position each frame to maintain an **ideal range** of about 150–300 units (preferring ~225 units) from the player. If it finds itself too close (<150), it will actively retreat; if too far, it advances. This results in a cat-and-mouse feel where shooters circle just at the edge of effective range. Additionally, shooter firing rate was increased: instead of waiting a long fixed interval, they now shoot more frequently (with a shorter cooldown timer) for increased pressure on the player. The firing inaccuracy margin was tightened from ±10° to ±5°, making their shots more likely to be on target but still dodgeable. Overall, shooters pose a more serious threat and require the player to stay on the move.

- **Bullet Trajectory & Ownership Clarification:**  
  Refined the bullet handling code to better differentiate and manage bullets from players vs enemies:
  - Ensured enemy bullets are aimed directly at the player’s current position when fired (with the above-mentioned random offset for shooters). This corrected an issue where some enemy bullets could fly off at wrong angles if the enemy moved or rotated unexpectedly.
  - The `isEnemy` flag on bullets is now consistently used wherever collisions are checked. This was partly an internal fix, but worth noting: the system reliably knows who owns each bullet, preventing friendly fire confusion. It also allowed simplifying collision logic, as we can simply check the flag rather than matching bullet sources to target types.

- **Debug Logging Verbosity:**  
  Improved the debug output across modules for easier troubleshooting:
  - Standardized messages and levels. Many DEBUG_PRINT calls were adjusted to appropriate detail levels (1 for high-level events, 2 for detailed state changes, 3 for very granular updates).
  - Example: Collision outcomes (hits, blocks) now log at detail 3 with severity 2, whereas critical issues (like file I/O failures) log at severity 0 (error) and detail 2.
  - Added the `g_always_print` mechanism to ensure certain vital messages (detail level 0) print even if debug mode is off. This is mostly used for critical errors or important notices.
  - These changes help developers or testers to get clearer insight into game behavior when running with `--debug`.

### Fixed
- **Collision Detection Accuracy:**  
  Several collision-related bugs were fixed, making the game more fair and consistent:
  - **Player Bullets vs Enemies:** Previously, fast-moving bullets could sometimes miss hitting an enemy due to frame timing. We adjusted the collision distance threshold slightly and now reliably detect when a bullet is within 15 units of an enemy. As a result, enemies register hits correctly and at the moment of impact the explosion animation is triggered (if the hit is fatal). No more bullets “ghosting” through enemies without effect.
  - **Enemy Bullets vs Player:** A logic oversight sometimes failed to reduce player health if an enemy bullet hit right as the shield deactivated. We tightened the check so that any enemy bullet within range while the shield is inactive will damage the player. This includes the exact frame the shield turns off. Additionally, whenever the player is hit by a bullet, we now invoke the camera shake effect (previously it only did so for collisions, now both cause it).
  - **Enemy-Player Collisions:** Ensured that when any enemy touches the player’s ship, it consistently registers. We now check distance < 20 units every frame for all enemies. When triggered, the player takes damage (if shield is down) and the camera shakes. This fixes an earlier bug where very fast enemies could slip through the player without triggering damage occasionally.
  - Collectively, these fixes ensure the player cannot exploit any hitbox gaps and likewise isn’t unfairly taken down by unnoticed collisions.

- **Duplicate Function Removal:**  
  Removed a redundant `enemy_shoot` function that was lingering from prior refactoring (it duplicated logic that was moved into the Enemy update function). This caused linkage issues on some compilers and could confuse code readers. All enemy shooting logic is now in one place (within `update_enemies`), and the extraneous declaration and definition of `enemy_shoot` were eliminated. This cleanup resolves any potential “multiple definition” compiler/linker errors.

- **Memory and Resource Handling:**  
  - Fixed a font resource leak: ensured that TTF_CloseFont is called on the loaded font at game exit. This wasn’t causing in-game issues but is good practice.
  - Added checks around file I/O for the blockchain: if the `blockchain.txt` cannot be opened for appending (e.g., directory missing), the game now logs an error and skips writing, whereas before it might have crashed trying to fprintf to a NULL file pointer.
  - Corrected minor typos in file paths and config (like a comment typo "configuratiosn" to "configurations" – non-functional but for clarity).

### Changed
- **Enemy Spawn & Update Flow:**  
  Refactored how enemy spawning integrates with the main loop to accommodate the new AI and effects:
  - The spawn logic (as described in Added/AI sections) was updated but also restructured for clarity. The code now clearly separates enemy type selection (using score brackets and random rolls) from initializing their properties. This makes it easier to tweak or add new types.
  - Ensured that spawn and update consider the game difficulty scaling. A variable `diffScale` (difficulty scale) is computed each frame based on score (and developer mode flags) and passed to `update_enemies`. This can, for example, be used to speed up enemies slightly as score grows (though currently it might be set to 1.0 for normal mode).
  - The ordering in the game loop was slightly changed: we now process all collisions after moving entities in a frame, but before rendering. Previously, some collisions (like enemy-player) were checked before updating all bullets which could cause slight inconsistencies. The new order is more logical: move everything, then collide, then draw.

- **Visual Tweaks and UI:**  
  - The HUD was adjusted to show “Energy” (shield energy) with one decimal place instead of as an integer, since energy is a float now refilling gradually. This gives the player clearer feedback on shield status.
  - The top-left debug text (when dev mode is on) now includes player coordinates and angle for easier debugging of movement.
  - Minor color adjustments: Player ship outline green was darkened slightly (from full 255 green to 180) to differentiate it from enemy bullets which are also green in debug mode. This is subtle and mainly noticeable if you look closely.
  - The “GAME OVER” screen text now includes “Score submitted securely!” as a confirmation message once the blockchain write and signature are done. This replaced a generic prompt, reassuring players their score was handled.

- **Debug System Behavior:**  
  - Introduced a new flag `g_always_print` in the Debug module and corresponding logic such that any debug message with `detail == 0` will be printed regardless of debug enablement. We set `g_always_print = 1` by default. This effectively creates a tier of messages that behave like normal prints (e.g., important notices or results). We use this for a few select messages that we want visible even in release mode (for example, printing the help text with `--help` might go through DEBUG_PRINT detail 0, so that it doesn’t require `--debug` to be seen).
  - Default `g_debug_level` remains 1, but many messages were assigned detail 2 or 3, so running with `--debug` alone shows only high-level logs. To get very granular logs, use `--debug` and set `g_debug_level` to 3 in code or possibly via an environment (future feature could allow `--debug=3`).

This release focused on enriching the gameplay experience (through visuals and AI difficulty) and solidifying the reliability of core systems (collision and scoring integrity). The project is still in an early stage (0.x), but with these improvements, it’s more enjoyable and robust.

*(For a comprehensive list of changes in previous versions, see below.)*

---

## [0.1.8] - 2025-03-26  
*9 New Enemies, Improved AI, Additional Controls*

### Added
- **Dynamic Ship Sizing:**  
  The player’s ship size can now be changed during gameplay. Use **UP** arrow to incrementally enlarge your ship and **DOWN** arrow to shrink it. Press **Right Shift** to reset to the default size. Larger ships have a bigger collision profile and bullets emit from a wider spread, whereas smaller ships are harder to hit but concentrate fire. This adds a risk-reward dynamic allowing players to adjust difficulty on the fly.

- **Advanced Enemy Types & Spawning:**  
  The game now features a variety of enemy types beyond the basic chaser:
  - *Shooter*: Fires bullets at the player from a distance.
  - *Tank*: Slow, high-health enemy.
  - *Evasive*: Dodges side-to-side as it approaches.
  - *Fast*: Quickly rushes the player.
  - *Stealth*: Occasionally disappears from view.
  - *Splitter*: Splits into smaller enemies on death.
  - *Boss* (3 tiers): Very high health, appear at very high scores for a boss-fight challenge.
  
  Enemies spawn based on the current score as a difficulty modifier. Early on, you’ll only face basic enemies. As your score increases, the spawn algorithm begins mixing in advanced types (gradually introducing shooters, then tanks, etc., up to bosses after score 2000). This ensures a learning curve for the player and keeps late-game interesting with diverse enemy behaviors.

- **Improved Shooter Enemy AI:**  
  The shooter enemy introduced in v0.1.7 has been enhanced. It now tries to maintain ~75 units distance from the player. If the player gets too close, the shooter will retreat to re-establish space. It continuously fires at the player, with each shot having a ±10° random offset to simulate aiming error and avoid perfectly predictable bullets. This makes shooter enemies a distinct threat that requires different tactics (dodge their shots or close in to take them out).

- **Enhanced Collision Detection:**  
  Collision handling was made more comprehensive:
  - Any collision between the player and an enemy (of any type) now correctly reduces the player’s health. Previously, only certain enemy types triggered damage on contact; now all do.
  - Enemy bullets, which are marked with the `isEnemy` flag, will damage the player on hit (unless the shield is active). This uses the new bullet ownership flag to prevent player’s own bullets from harming them.
  
- **Bullet Ownership Flag:**  
  A new field `isEnemy` was added to the Bullet structure. This flag is set to 0 for player-fired bullets and 1 for enemy-fired bullets. It is now used in collision checks and rendering logic to differentiate bullet behavior. This was crucial for implementing enemy bullets that could hurt the player while ignoring friendly fire among enemies.

### Fixed
- **Shooter Accuracy Issue:**  
  Fixed a bug where shooter enemies sometimes failed to aim at the player correctly. The calculations for bullet velocity now correctly point towards the player’s position at the time of firing, resulting in much more accurate (and dangerous) shots from shooter enemies. No more bullets flying off at odd angles when they shouldn’t.

- **Spawn Logic Bug:**  
  Previously, the enemy spawn probability used a fixed value which didn’t properly scale with score, meaning advanced enemies might not appear at intended times. This is now fixed by using the current score in the spawn algorithm (as described above), ensuring, for example, that at score 1000+ you do start seeing tanks, etc., as intended.

- **Player-Enemy Collision:**  
  Corrected an oversight where collisions between certain enemies and the player were not reducing player health. Now all enemy types, including any new ones, will trigger damage on contact. The collision radius and damage values were fine-tuned so that these collisions feel neither too unforgiving nor too lenient.

- **Duplicate Function Definitions:**  
  Removed duplicate definitions like `enemy_shoot` which were inadvertently left during development, causing possible linkage errors on some platforms. The codebase was cleaned up to ensure each function is defined only once in the appropriate module.

### Changed
- **Enemy Update Refactor:**  
  The routines for enemy spawning and updating their behavior were refactored for clarity and to integrate the new enemy types and AI improvements. This means the code is better organized: one function handles deciding when and what to spawn, another handles per-frame logic for each enemy (moving, shooting, etc.). We maintained full compatibility with existing systems like the high score blockchain and rendering engine during this refactor, so these changes are under-the-hood and should not affect save data or performance negatively.

- **Gameplay Balance:**  
  With the introduction of dynamic ship sizing and tougher enemies, we made minor tweaks to ensure balance:
  - Player’s default health remains the same, but shield recharge rate was slightly increased to give a fighting chance against increased enemy fire (from 0.005 to 0.007 units per frame, a subtle bump).
  - The score formula now includes a time component (1 point per second survived) in addition to kills, to reward skilled evasion even if you’re not getting rapid kills. This change was implicit, tied to using time-based score increments.
  - Overall difficulty ramps up more smoothly: e.g., in mid-game you face a mix of basic and shooters rather than a sudden flood of new types. The random distribution for spawns was tweaked (as noted in spawn logic) to avoid extreme luck swings.

---

## [0.1.7] - 2025-03-20  
*Enhanced Leaderboard & README Automation*

### Added
- **README Integration:**  
  - The `update_highscores.py` script can now automatically update the project README.md with the latest high scores. It inserts the top scores into a table under a marked section, so that the GitHub repository front page always shows current leaders.
  - The script and README are set up to handle cases with few players gracefully (e.g., if fewer than 3 players have scores, it won’t break the table formatting).
  
- **Automated Verification:**  
  - `verify_scores.py` logic was incorporated such that `update_highscores.py` now performs rigorous checks on all scores before publishing. Only valid scores (correct PoW and signature) make it to the README. Any blocks failing verification are listed in a new “Cheaters” section, calling out invalid entries publicly.
  
- **User Guidance:**  
  - Running the update script interactively gives the user feedback: it explains that the leaderboard is auto-updated on schedule and reminds maintainers to push changes if they run it locally.
  - When run via CI (GitHub Actions), it suppresses these messages, doing a silent update for cleanliness.

### Changed
- Revised README.md format to include dedicated **Top Scores** and **Cheaters** sections demarcated by HTML comments. This allows scripts to reliably insert content between the markers without manual editing.
- The blockchain verification difficulty remains at 4, but we clarified in documentation how it works and ensured that all historical blocks still meet the criteria (some earlier test blocks were updated to conform).
- The high score submission now writes JSON lines with a trailing newline (ensuring each block is on its own line properly). This avoids any parsing issues where the last entry might not end with newline.

### Removed
- (No removals in this version; all changes were additive or modifications for automation.)

### Developer Notes:
This version completed our vision for a self-updating leaderboard. With these automation features, maintaining the high score table is hands-free and tamper-proof. The combination of blockchain security and automated disclosure of invalid scores provides a high level of trust and transparency for the community.




## [0.0.8] - Enhanced Assets & Visual Refinements
- **Aesthetic Improvements:**  
  - Redesigned the player’s ship into a sleek five-point arrow.  
  - Upgraded enemy graphics with smooth, filled ellipses.  
  - Improved background visuals with additional cosmic elements and grid enhancements.  
- **Developer Notes:**  
  These changes significantly improved the overall look and feel of the game, making it much more engaging.

---

## [0.0.7] - Transition to SDL2 Graphics
- **Graphical Overhaul:**  
  - Migrated from ncurses to SDL2, introducing a modern, graphical interface.  
  - Integrated SDL2_ttf for better text rendering and SDL2_gfx for advanced shapes.  
- **Developer Notes:**  
  A major milestone that transitioned our game from a text-based prototype to a visually appealing experience.

---

## [0.0.6] - Dynamic Bullet Pool Implementation
- **Unlimited Firing:**  
  - Replaced the fixed bullet array with a dynamically growing bullet pool.  
- **Performance Gains:**  
  - Enhanced memory management and ensured smooth gameplay even during rapid fire.  
- **Developer Notes:**  
  This improvement removed limitations on firing, allowing for more dynamic combat scenarios.

---

## [0.0.5] - Bullet Origin & Trajectory Fixes
- **Bullet Spawn Correction:**  
  - Fixed the calculation of the bullet’s starting point so that it now originates exactly from the ship’s tip.  
  - Adjusted trajectory calculations to match the ship’s facing direction accurately.  
- **Developer Notes:**  
  Bullets now behave as expected, significantly improving the shooting mechanics.

---

## [0.0.4] - Player Movement Refinement & Inertia System
- **Movement Physics:**  
  - Introduced velocity and damping, simulating realistic inertia for the player’s ship.  
  - Capped the maximum speed to ensure smooth, controlled movement.  
- **Rotation Improvements:**  
  - Fine‑tuned rotation controls for better precision during gameplay.  
- **Developer Notes:**  
  This update made the ship’s movement feel fluid and responsive, closely mimicking real-world physics.

---

## [0.0.3] - Input Responsiveness & HUD Enhancements
- **Input Handling:**  
  - Reduced frame delay and improved the event polling loop for more responsive controls.  
- **HUD Display:**  
  - Added a heads-up display (HUD) to show player health, energy, score, and position on screen.  
- **Shield Mechanics:**  
  - Refined shield activation so that it only remains active while the key is held.  
- **Developer Notes:**  
  These changes provided a more user-friendly and immersive gameplay experience.

---

## [0.0.2] - Modularization & Background Enhancement
- **Code Organization:**  
  - Broke the monolithic code into distinct modules (background, bullet, enemy, game, player, score) for improved maintainability.  
- **Background Improvements:**  
  - Enhanced the background rendering to include a Cartesian grid and more randomized star placement.  
- **Developer Notes:**  
  The focus was on cleaning up the codebase and laying a solid foundation for future enhancements.

---

## [0.0.1] - Initial TUI Release
- **Initial TUI Attempt:**  
  - Developed a text‑based prototype using ncurses.  
  - **Background:** A simple starry background was generated with minimal hashing logic.  
  - **Bullet/Enemy:** Basic definitions for bullets and enemies, including simple AI for enemy movement.  
  - **Game Mechanics:** Implemented core gameplay mechanics (movement, shooting, collision detection) using a text‑user interface.  
  - **High Score:** High scores were saved to a plaintext file.  
- **Developer Notes:**  
  This release was our first proof‑of‑concept—raw, simple, and experimental.

---





## [0.0.9] - High Score & Username Integration (TUI/SDL Hybrid)
- **User Integration:**  
  - Introduced per-user high score saving with high score files (`highscore/<username>_highscore.txt`).  
  - Initially prompted for usernames via the terminal before transitioning to in-window prompts.  
- **Developer Notes:**  
  This version laid the groundwork for a community leaderboard system by associating scores with user identities.

---

## [0.1.0] - Full SDL2 Implementation & In-Game Username Prompt
- **Complete SDL2 Transition:**  
  - Removed terminal-based interactions entirely; everything now runs within an SDL2 window.  
- **In-Game Prompts:**  
  - Implemented a graphical username prompt on game start and game over.  
- **High Score Management:**  
  - Fully integrated the high score system within the SDL2 interface.  
- **Developer Notes:**  
  This version provided a seamless, immersive experience from start to finish.

---

## [0.1.1] - Command-Line Flags & Versioning
- **New Flags:**  
  - Added command-line flags: `--version`, `--help`, `--debug`, `--fullscreen`, and `--highscores`.  
- **Version Tracking:**  
  - Introduced version.h for centralized version control.
- **Developer Notes:**  
  Enhancing user control and providing clear usage instructions improved the project's professionalism.

---

## [0.1.2] - Adaptive Difficulty & Enemy AI Improvements
- **Dynamic Enemy AI:**  
  - Improved enemy behavior with scaling AI that becomes more aggressive as scores increase.  
- **Adaptive Difficulty:**  
  - Implemented a difficulty multiplier that dynamically adjusts enemy speed and spawn frequency.
- **Developer Notes:**  
  The game now challenges players more as they improve, creating a balanced yet progressively tougher experience.

---

## [0.1.3] - GitHub Integration & Automated High Score Updates
- **Automation:**  
  - Developed Python scripts for automated high score updates and blockchain verification.
  - Introduced a GitHub Actions workflow to run these updates every Monday at 00:00 UTC.
- **Developer Notes:**  
  This version set the stage for continuous integration and community participation through automated leaderboard updates.

---

## [0.1.4] - Enhanced High Score Display and Manual Run Reminder
- **Leaderboard Refinement:**  
  - Updated the Python update script to generate dynamic badges and a neatly formatted leaderboard.
  - If fewer than three unique users exist, all are displayed.
- **User Reminder:**  
  - When run manually, the script now reminds users to push changes to update the repository.
- **Developer Notes:**  
  This update improves clarity and usability for both developers and players.

---

## [0.1.5] - QuantumStriker: The First Initial Polished Release
- **Polished Gameplay:**  
  - Integrated all features into a cohesive, responsive SDL2-based game.
  - Finalized player controls, adaptive difficulty, dynamic bullet pools, and sophisticated enemy AI.
- **Community High Scores:**  
  - Fully integrated an in-window username prompt and blockchain-backed high score system.
- **Developer Notes:**  
  This release represents the culmination of our efforts so far—a robust, polished game ready for public release and future feature expansions.

---

## [0.1.6] - Blockchain Validation Addition
- **Blockchain High Score System:**  
  - Implemented a blockchain mechanism for secure score submission.
  - Each score block now includes a proof-of-work and a digital signature.
  - Fixed linkage issues within the game.c code
- **Score Verification & Cheater Detection:**  
  - Added Python scripts to verify each block's integrity and automatically classify invalid scores as cheaters.
- **Developer Notes:**  
  This critical update enhanced the security and trustworthiness of the high score system, discouraging cheating.

---

## [0.1.7] - Enhanced Leaderboard & README Automation
- **README Integration:**  
  - Improved the update_highscores.py script to update the README.md automatically.  
  - Added support to handle cases with fewer than three unique users, ensuring the leaderboard is always meaningful.
- **Automated Verification:**  
  - Incorporated rigorous checks (via verify_scores.py logic) to ensure only valid scores are published.  
  - Blocks failing verification are now clearly listed under a "Cheaters" section.
- **User Guidance:**  
  - When run interactively, the script informs users that the update is performed on a regular basis and reminds them to manually push changes.  
  - When run automatically (e.g., via GitHub Actions), it silently updates the README.
- **Developer Notes:**  
  This version completes our vision for a self‑updating leaderboard and provides a transparent, automated process that maintains community trust. Every detail—from dynamic leaderboard generation to automated cheater detection—has been crafted to ensure the highest level of integrity and usability.

---

Below is a changelog entry for version **0.1.8** that encapsulates all the enhancements and fixes implemented during our recent updates:

---

## [0.1.8] - 9 new enemies. Improved AI. Additional controls. 

### Added
- **Dynamic Ship Sizing:**  
  The player's ship can now be resized dynamically during gameplay. Users can increase, decrease, and reset the ship size using the UP, DOWN, and Right Shift keys, respectively.

- **Advanced Enemy Types & Spawning:**  
  The enemy spawning algorithm has been revamped to use the current score as a modifier. This change allows advanced enemy types—such as shooters, tanks, evasive, fast, stealth, and bosses—to spawn progressively as the player’s score increases.

- **Improved Shooter Enemy AI:**  
  Shooter enemies now maintain a safe distance of approximately 75 units from the player. They will back off if they get too close while continuously shooting at the player with a randomized (±10°) offset for imperfect aim.

- **Enhanced Collision Detection:**  
  - Collisions between the player and any enemy now correctly reduce the player's health.  
  - Enemy bullets (marked with an ownership flag) now inflict damage on the player if the shield is inactive.
  
- **Bullet Ownership Flag:**  
  The bullet module has been updated to include an “isEnemy” flag, allowing differentiation between player-fired and enemy-fired bullets during collision processing.

### Fixed
- Resolved issues where shooter enemies did not fire accurately at the player.
- Fixed the spawn logic that was previously using a fixed modifier, which prevented advanced enemy types from appearing.
- Corrected missing collision detection between the player and enemies, ensuring that all enemy types inflict damage upon collision.
- Addressed duplicate function definitions (e.g., `enemy_shoot`) to avoid linkage errors.

### Changed
- Refactored the enemy spawn and update logic to integrate the new AI behaviors while preserving existing gameplay and blockchain high score functionality.
- Maintained full compatibility with previous features (blockchain submission, UI, debugging) while enhancing enemy behavior and player interactions.

---

## [0.1.9] - Enhanced Shooter AI, Fullscreen Mode, & New Visual Effects

### Added
- **Fullscreen & Resizable Window Support:**  
  Introduced a new configuration flag (`g_fullscreen` in config.h) to launch the game in fullscreen mode using `SDL_WINDOW_FULLSCREEN_DESKTOP`. The window creation code in game.c now dynamically updates the screen dimensions to match the display’s current mode when fullscreen is enabled.

- **Camera Shake Effect:**  
  Added global variables (`shakeTimer` and `shakeMagnitude` in config.h) and integrated a camera shake effect into the game loop. The effect triggers whenever the player takes damage (from enemy bullets or collisions), enhancing the game’s visual impact.

- **Explosion Animations for Enemies:**  
  Implemented explosion animations using SDL2_gfx’s filledCircleRGBA. When an enemy is destroyed by a bullet (but not by collision), an expanding, fading explosion is displayed over a brief 30‑frame period.

### Improved
- **Shooter Enemy AI:**  
  Refined shooter enemy behavior to continuously track the player. The AI now recalculates the angle and distance every frame, striving to maintain an ideal engagement range (roughly between 150 and 300 units, targeting ~225 units). When within range, the shooter fires bullets with a slight random offset (±5°) and employs a faster cooldown for more aggressive action.

- **Bullet Trajectory & Ownership:**  
  Enhanced bullet management by fine‑tuning enemy bullet trajectories to ensure they are correctly aimed at the player. The system now more reliably distinguishes enemy bullets from player-fired bullets via the `isEnemy` flag.

### Fixed
- Removed duplicate function definitions (e.g., the redundant `enemy_shoot` function) to avoid linkage errors.
- Corrected collision detection issues so that:
  - Player bullet impacts reduce enemy health and trigger explosion animations.
  - Enemy bullet impacts reduce player health and trigger camera shake (if the shield isn’t active).
  - Direct collisions between enemies and the player are consistently detected.

### Changed
- Refactored enemy spawn and update logic to integrate the new AI behaviors and visual effects while maintaining full compatibility with previous features (including blockchain-backed high score submission and in‑game UI).
- Updated debug logging across modules to provide clearer insights into gameplay and AI behavior.
- Changed config.h and config.c to add a 0 integrer that will cause printing regardless of the severity or --debug option. 

---

