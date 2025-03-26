# Changelog

This document chronicles all major changes and improvements made to QuantumStriker. Each version entry reflects significant milestones, feature enhancements, and fixes—told from the perspective of our journey in crafting a unique, blockchain‑backed space shooter.

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

## [0.0.2] - Modularization & Background Enhancement
- **Code Organization:**  
  - Broke the monolithic code into distinct modules (background, bullet, enemy, game, player, score) for improved maintainability.  
- **Background Improvements:**  
  - Enhanced the background rendering to include a Cartesian grid and more randomized star placement.  
- **Developer Notes:**  
  The focus was on cleaning up the codebase and laying a solid foundation for future enhancements.

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

## [0.0.4] - Player Movement Refinement & Inertia System
- **Movement Physics:**  
  - Introduced velocity and damping, simulating realistic inertia for the player’s ship.  
  - Capped the maximum speed to ensure smooth, controlled movement.  
- **Rotation Improvements:**  
  - Fine‑tuned rotation controls for better precision during gameplay.  
- **Developer Notes:**  
  This update made the ship’s movement feel fluid and responsive, closely mimicking real-world physics.

---

## [0.0.5] - Bullet Origin & Trajectory Fixes
- **Bullet Spawn Correction:**  
  - Fixed the calculation of the bullet’s starting point so that it now originates exactly from the ship’s tip.  
  - Adjusted trajectory calculations to match the ship’s facing direction accurately.  
- **Developer Notes:**  
  Bullets now behave as expected, significantly improving the shooting mechanics.

---

## [0.0.6] - Dynamic Bullet Pool Implementation
- **Unlimited Firing:**  
  - Replaced the fixed bullet array with a dynamically growing bullet pool.  
- **Performance Gains:**  
  - Enhanced memory management and ensured smooth gameplay even during rapid fire.  
- **Developer Notes:**  
  This improvement removed limitations on firing, allowing for more dynamic combat scenarios.

---

## [0.0.7] - Transition to SDL2 Graphics
- **Graphical Overhaul:**  
  - Migrated from ncurses to SDL2, introducing a modern, graphical interface.  
  - Integrated SDL2_ttf for better text rendering and SDL2_gfx for advanced shapes.  
- **Developer Notes:**  
  A major milestone that transitioned our game from a text-based prototype to a visually appealing experience.

---

## [0.0.8] - Enhanced Assets & Visual Refinements
- **Aesthetic Improvements:**  
  - Redesigned the player’s ship into a sleek five-point arrow.  
  - Upgraded enemy graphics with smooth, filled ellipses.  
  - Improved background visuals with additional cosmic elements and grid enhancements.  
- **Developer Notes:**  
  These changes significantly improved the overall look and feel of the game, making it much more engaging.

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

