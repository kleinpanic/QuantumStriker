# QuantumStriker

## Disclaimer 

> If you want to enjoy this game and submit your high score, please be aware that it may be lost due to future updates. If anyone besides myself is ever to play this game and contribute a score, I will try to develop a fully compatible mechanism so that past blockchain entries remain valid under future versions. I will save your score and do my best to ensure it is preserved for future iterations, but I cannot make that a guarantee.

## Top Scores & Cheaters 

<!-- TOP_SCORES_START -->
| Rank | Username        | Score | Timestamp           |
|------|-----------------|-------|---------------------|
| 1    | kleinpanicDevAI | 5267  | 2025-03-27 22:51:48 |
| 2    | kleinpanic      | 2585  | 2025-03-26 12:02:08 |
<!-- TOP_SCORES_END -->

<!-- CHEATERS_START -->
No cheaters found.
<!-- CHEATERS_END -->

QuantumStriker is a high-performance 2D space shooter written in C (using SDL2, SDL2_ttf, and SDL2_gfx). You pilot a futuristic spacecraft through a dynamically generated galactic environment, fending off waves of enemies that scale in difficulty as your score rises. When your ship is destroyed, your final score is recorded in a blockchain-based high score system that validates each entry via cryptographic proof-of-work and digital signatures. Invalid or tampered scores (failing verification) are flagged as **cheaters** and listed separately in the repository README.

## Features

- **Dynamic Galactic Environment:**  
  Explore a procedurally generated universe filled with stars, planets, and cosmic pickups. The background is populated randomly and scrolls as you fly, giving a sense of vast space.

- **Adaptive Difficulty & Advanced AI:**  
  As your score grows, enemy spawn rates accelerate and AI behaviors become more challenging. Early on, you face simple drones, but as you progress, advanced enemy types (e.g., *shooter*, *tank*, *fast*, *stealth*, *boss*) appear. Shooter enemies maintain a strategic distance and fire at you with slight aim randomness for realism. Boss enemies have higher health and unique movement patterns to test your skills.

- **Dynamic Ship Sizing:**  
  Change your ship’s size on the fly. Use **UP** to increase and **DOWN** to decrease your ship’s scale, affecting hitbox and firing spread. Press **Right Shift** to reset to the default size. This feature adds a tactical layer—larger ships can hit more targets but are easier to hit, while smaller ships are nimble but do less damage.

- **Blockchain-Backed High Scores:**  
  After each game over, input a username to submit your score. The game creates a new block in `highscore/blockchain.txt` containing your username, score, timestamp, proof-of-work hash, and a digital signature. Proof-of-work ensures the score is computationally earned (by finding a hash with a required number of leading zeros), and the RSA signature (using your unique key) ties the block to you. A Python script, `verify_scores.py`, recomputes each block’s hash and checks the proof-of-work and signature to validate authenticity. Verified scores show up in **Top Scores**, whereas any block failing verification is listed under **Cheaters**.

- **New Visual Effects (v0.1.9):**  
  - *Fullscreen & Resizable Window:* The game can run in fullscreen (`--fullscreen` flag) using your desktop’s resolution, or in a resizable window. The rendering system adapts to the current screen dimensions dynamically.  
  - *Camera Shake:* When your ship takes damage (enemy bullet hits or enemy collisions), the camera briefly shakes to emphasize the impact. This uses a short `shakeTimer` and `shakeMagnitude` to offset the camera’s position for a few frames, then settles.  
  - *Explosion Animations:* Destroying enemies with bullets triggers an explosion effect. A fiery orange blast expands and fades over 30 frames, adding visual feedback when you score a kill. These explosions do not occur for enemies destroyed by colliding with your ship (to distinguish kill types).

- **Automated Leaderboard Updates:**  
  A GitHub Actions workflow runs `scripts/update_highscores.py` every Monday at 00:00 UTC to update the **Top Scores** and **Cheaters** sections in this README. The script reads `highscore/blockchain.txt`, verifies each entry, and updates the markdown table between the `TOP_SCORES` markers with the highest valid scores. It similarly updates the `CHEATERS` section if any invalid entries are detected. This ensures the online leaderboard is always current and trustable, without manual intervention.

- **Command-Line Flags:**  
  QuantumStriker supports several launch options:
  - `--version`: Print the current game version.
  - `--help`: Show a help message describing usage.
  - `--debug`: Enable verbose debug logging (to console) for development or troubleshooting.
  - `--fullscreen`: Launch in fullscreen mode. In v0.1.9 this flag is fully supported, toggling `SDL_WINDOW_FULLSCREEN_DESKTOP` for an immersive experience.
  - `--highscores`: Display all recorded high scores in the terminal (reads from `blockchain.txt` and prints a formatted list).

- **Responsive Controls:**  
  - **Movement:** Use **WASD** to move relative to your ship’s facing direction (W = forward thrust, S = reverse, A = strafe left, D = strafe right). This scheme means forward (W) always moves the ship in the direction its nose is pointing.  
  - **Rotation:** Use the **←→** (left/right arrow keys) to rotate your ship. Rotation is smooth, and you can combine it with thrust for banking turns.  
  - **Speed Boost:** Hold **Left Ctrl** to double movement and rotation speed, useful for quick dodges or repositioning.  
  - **Shooting:** Press **Space** to fire bullets from your ship’s tip. Bullets have unlimited range until they despawn or hit a target, and a short cooldown prevents spamming.  
  - **Shield:** Press **E** to toggle an energy shield. The shield can absorb enemy bullets, but drains an energy meter while active. If energy depletes, the shield turns off until it recharges. A circular indicator appears around your ship when the shield is up. Shield management is crucial: use it to block damage in tight situations, but watch the energy bar.

- **Modular Architecture:**  
  The game’s code is divided into modules, each handling a specific aspect:
  - **Player Module:** Manages player state (position, velocity, health, shield, ship size) and drawing the ship.
  - **Enemy Module:** Manages enemy spawning, AI behaviors, movement, health, and drawing enemies.
  - **Bullet Module:** Manages creation and physics of bullets (both player and enemy bullets), including an `isEnemy` flag to differentiate origin.
  - **Background Module:** Renders a scrolling starfield and occasional planets/objects as you move.
  - **Score/Blockchain/Signature Modules:** Handle high score saving, blockchain mechanics, proof-of-work, RSA key generation, and digital signing for scores.
  - **Debug Module:** Provides `DEBUG_PRINT` macros with different verbosity and color-coded output for logs. This helps trace game events and is gated by flags so it doesn’t affect performance when not needed.
  - **Configuration and Versioning:** Global constants (like difficulty, max enemies, physical constants) are defined in `config.h` for easy tuning. `version.h` holds the game’s current version string.

For a deeper technical breakdown, see [`ARCHITECTURE.md`](ARCHITECTURE.md). For development guidelines, refer to [`CONTRIBUTING.md`](CONTRIBUTING.md). 

## Installation

### Dependencies

Ensure you have the following installed on your system:

- **GCC or Clang** (any C99-compliant compiler)
- **SDL2** – Core library for graphics, windowing, and input.
- **SDL2_ttf** – Extension for rendering text (high score and HUD).
- **SDL2_gfx** – Extension for drawing shapes (used for explosions, shapes, etc.).
- **OpenSSL** – Required for RSA key generation and SHA-256 (for signature and proof-of-work).
- **Make** – For building the project.

On Debian/Ubuntu, you can install these via apt:

```bash
sudo apt-get install build-essential libsdl2-dev libsdl2-ttf-dev libsdl2-gfx-dev libssl-dev
```

*(`libssl-dev` provides OpenSSL development libraries.)*

### Building

Clone the repository and navigate into it:

```bash
git clone https://github.com/yourusername/QuantumStriker.git
cd QuantumStriker
```

Compile the game using the provided Makefile:

```bash
make
```

This produces an executable named `QuantumStriker`. To include debug symbols and verbose logging, build in debug mode:

```bash
make debug
```

This defines debug flags and disables optimizations for easier debugging.

## Usage

Run the game from the terminal:

```bash
./QuantumStriker
```

Use the controls described above to play. Close the game window or press **Esc** to exit.

### Command-Line Flags

You can modify the game’s behavior with these optional flags:

- **`--version`**: Print the game version and exit.
- **`--help`**: Show a brief help text with usage and flags.
- **`--debug`**: Enable debug mode (console logging). By default, only critical errors log; with this flag, detailed logs appear (movement, collisions, AI decisions, etc.). Useful for development or if reporting a bug.
- **`--fullscreen`**: Start in fullscreen mode at desktop resolution. (You can still toggle windowed mode by quitting and restarting without the flag.)
- **`--highscores`**: Print all high scores recorded in the blockchain (valid and invalid) to the console in a formatted list, then exit.

## Game Mechanics & High Score System

- **Scoring:** Your score increases with time survived and enemies destroyed. The current implementation awards 10 points per enemy kill plus 1 point per second of survival. The HUD (top-left) continuously displays your health, shield energy, score, and coordinates for awareness.

- **Enemy AI:** Enemies use simple AI patterns:
  - Basic drones head straight towards you.
  - Shooters try to keep a distance (~150-300 units) and fire periodically with slight inaccuracy.
  - Fast enemies dart quickly, making them hard to hit.
  - Evasive enemies zigzag or circle to avoid fire.
  - Tank enemies move slowly but can absorb more hits.
  - Stealth enemies occasionally turn invisible (not implemented fully in v0.1.9, but groundwork is laid).
  - Bosses appear at high scores (2000+ points) with significant health and a mix of behaviors (e.g., Boss1 might combine shooting and fast movements).
  
  The spawn algorithm introduces these types gradually: e.g., before 100 points only basics spawn; by 500 points, shooters mix in; by 1000, tanks join; by 2000+, all types including bosses can spawn. Each enemy type has preset health and sometimes unique properties (shooters have a shoot cooldown timer, stealth might have a transparency toggle, etc.).

- **Game Loop:** The core loop runs at ~60 FPS (frame delay ~15ms) and handles:
  1. **Input** – reading keyboard state for movement and actions.
  2. **Updating Entities** – moving the player and enemies, applying physics (thrust, drag), checking bounds.
  3. **AI Decisions** – updating enemy behavior (moving towards/away from player, deciding to shoot).
  4. **Collisions** – checking bullet collisions and enemy-player overlaps:
     - Player bullets vs enemies: if within ~15px, the bullet deals damage and (unless in dev “piercing” mode) disappears. If the enemy’s health drops ≤0, it’s “killed”: an explosion is triggered, the enemy is deactivated, and your kill count (for score) increments.
     - Enemy bullets vs player: if an active enemy bullet comes within ~15px of the player, it either depletes health (if your shield is off) or is absorbed (if shield on). On a hit without shield, the game triggers a camera shake (setting `shakeTimer=20` frames and `shakeMagnitude=10`) to visually indicate damage. The bullet is then removed.
     - Enemy ships vs player: if any enemy gets within ~20px of the player, it’s a collision. If your shield is down, you take damage and camera shakes. The enemy is also destroyed by the impact (removed from play). Shielded collisions simply destroy the enemy without health loss (with appropriate debug log).
  5. **Rendering** – drawing everything in the correct order: background first (offset by camera, which follows the player), then particles/explosions, then enemies and bullets at their world positions offset by camera, then finally the player at screen center (if camera is following them), plus the HUD on top. If the camera is shaking, its offset is applied to all world elements (background, enemies, bullets) for that frame.
  6. **Delay & Loop** – enforce the frame delay, then repeat until the game ends.

- **Game Over & Score Submission:** When your health drops to 0, the game ends:
  - A “GAME OVER” message displays with your time survived, enemies killed, and final score.
  - The game then takes your username (loaded at start from a `.username` file or asked at runtime) and prepares a score block:
    - Gathers the last blockchain block for your username (if any) to chain properly.
    - Fills a new `ScoreBlock` struct with your username, score, timestamp, and the previous block’s hash.
    - Calls `add_score_block(&newBlock, prevBlock, DIFFICULTY)` to compute the proof-of-work hash by finding a nonce such that the SHA-256 hash starts with **DIFFICULTY=4** leading zeros. This typically means thousands of hash iterations (PoW).
    - Signs the block using your private RSA key (`sign_score`) to produce a signature string.
    - Appends the block as a JSON line in `highscore/blockchain.txt`. The JSON includes all relevant fields (`username, score, timestamp, proof_of_work, signature, prev_hash, nonce`).
  - The console (if debug enabled) will log success or any issues (e.g., file write failures or signature problems).
  - The on-screen text “Score submitted securely!” confirms the process completed.

- **High Score Verification:** The provided script `verify_scores.py` (and the automated `update_highscores.py`) performs the reverse:
  - Reads each JSON block from the blockchain file.
  - Verifies the proof-of-work by recomputing the hash of the block’s data and checking it matches the stored `proof_of_work` and has the required leading zeros.
  - Verifies the RSA signature with the public key for the user (public keys are stored in `highscore/public_keys/<username>.pem` when generated).
  - Ensures that each block’s `prev_hash` matches the previous block’s `proof_of_work` to maintain chain integrity.
  - Any block that fails any check (bad PoW, bad signature, broken chain) is considered invalid. Such scores are excluded from the leaderboard and flagged as cheating attempts.
  - The highest valid score per user is tallied, and the top scores are sorted for display in this README.

**Note:** The blockchain file is append-only. Each run of the game potentially adds one block. The difficulty is fixed (currently 4 leading zeros), which balances speed (submissions take a second or two) and security (intentional fake scores would require significant computation to forge). The RSA keys ensure one user cannot submit scores under another user’s name without access to their private key.

## Project Structure

```plaintext
QuantumStriker/
├── src/
│   ├── main.c               # Program entry; handles CLI flags and launches game loop.
│   ├── game.c               # Main game loop, event handling, core gameplay logic.
│   ├── game.h               # Game loop function and common structures (e.g., Explosion).
│   ├── player.c / player.h  # Player spaceship: movement physics, rendering, shield.
│   ├── enemy.c / enemy.h    # Enemy entities: types, AI behaviors, spawning, rendering.
│   ├── bullet.c / bullet.h  # Bullet pool: spawning bullets for player and enemies, updating movement, collision helpers.
│   ├── background.c / background.h  # Starfield background and pickups, drawn relative to camera.
│   ├── score.c / score.h    # Username & high score file management (loading .username, personal best tracking).
│   ├── blockchain.c / blockchain.h  # Blockchain functions: add block, verify chain, PoW calculation.
│   ├── signature.c / signature.h    # RSA signature generation and verification (wrapping OpenSSL).
│   ├── encryption.c / encryption.h  # Crypto utilities: SHA-256 hashing, RSA key gen/load (uses OpenSSL).
│   ├── debug.c / debug.h    # Debug logging system (with levels and color-coded output).
│   └── version.h            # Defines the current version string (e.g., "0.1.9").
├── highscore/
│   ├── blockchain.txt       # Blockchain of score submissions in JSON lines.
│   └── public_keys/         # Public keys for each user (username as filename) to verify signatures.
├── scripts/
│   ├── update_highscores.py # Script to update README.md’s Top Scores and Cheaters sections based on blockchain.
│   └── verify_scores.py     # (Utility) Verifies blockchain integrity and prints results (used by update_highscores).
├── .github/workflows/
│   └── update-highscores.yml # CI workflow to run update_highscores.py weekly.
├── README.md
├── ARCHITECTURE.md
├── CHANGELOG.md
├── TODO.md
└── CONTRIBUTING.md
```

The code is organized to separate concerns and allow independent development of features. For example, you could replace the enemy module with more sophisticated AI without affecting the blockchain code, or adjust the blockchain difficulty without altering game rendering.

## High Score & Cheater System

- **Score Submission:**  
  At game over, you’re prompted for a username (if not already set). The game then appends a new block to `highscore/blockchain.txt` containing:
  - `username`: (string) your name or alias.
  - `score`: (integer) your final score.
  - `timestamp`: (integer) Unix time of submission.
  - `proof_of_work`: (64-char hex) a SHA-256 hash of the block’s core data meeting the difficulty criteria (e.g., starting with "0000").
  - `signature`: (hex string) RSA signature of the block’s data, using the private key associated with `username`.
  - `prev_hash`: (64-char hex) the previous block’s hash (or all zeros for the first block for that user).
  - `nonce`: (integer) the value that produced the `proof_of_work` when hashing.

  All this is done in-game automatically. The blockchain file grows over time, representing an immutable ledger of scores.

- **Score Verification:**  
  The verification script checks each block by:
  - **Hash Recalculation:** It recomputes the SHA-256 hash of the block’s data (username, score, timestamp, prev_hash, nonce) and ensures it matches the stored `proof_of_work`.
  - **Proof-of-Work Difficulty:** It checks that the `proof_of_work` hash has the required prefix of zeros (difficulty 4 ⇒ hash begins with "0000").
  - **Signature Validity:** It uses the stored public key for the username to verify the signature against the block’s data. If a public key is missing or the signature doesn’t match, the block is invalid.
  - **Chain Linkage:** For each block after the first, it ensures the `prev_hash` matches the *previous* block’s `proof_of_work`, maintaining an unbroken chain.
  
  Only if all checks pass is the block considered valid. This multi-layer security makes it extremely hard to forge a score: one would need the user’s private key and to compute a valid proof-of-work, *and* not break the chain sequence.

- **Leaderboard Update:**  
  The `update_highscores.py` script (run by CI or manually) parses the blockchain:
  - It aggregates the highest **valid** score per username.
  - Sorts these scores and takes the top N (usually top 10).
  - Overwrites the **Top Scores** table in this README (between `<!-- TOP_SCORES_START -->` and `<!-- TOP_SCORES_END -->`) with the latest rankings.
  - If any invalid blocks were found, it lists the corresponding usernames and scores under the **Cheaters** section (between markers). Each cheater entry typically indicates an attempted forged score or a corrupted entry.
  - The script then commits the changes. On GitHub, the Action will push the updated README so that the public view is updated.
  
  For transparency, any changes or issues with the leaderboard are managed via this auditable script rather than manual edits.

## How It Works (Technical Highlights)

- **Game Loop & Difficulty Scaling:**  
  QuantumStriker employs a straightforward game loop (in `game.c`) that repeats continuously:
  1. Poll input events (keyboard).
  2. Update player movement based on input (applying thrust or rotation).
  3. Spawn new enemies over time, with probability influenced by current score (higher score → more frequent spawns, and more advanced enemy types).
  4. Update all enemies (move them; some may shoot bullets towards the player).
  5. Update all bullets (move forward; deactivate if out of range).
  6. Check collisions (bullets hitting targets, etc. as detailed above).
  7. Render everything to the screen.
  8. Loop delay for consistent frame timing (ensuring ~60 FPS).

  Difficulty increases implicitly by spawning tougher enemies as your score increases. There are no "levels"—it’s continuous survival with increasing challenge.

- **Player Controls & Physics:**  
  The player’s ship uses a simple physics model:
  - **Orientation:** A variable `player.angle` (degrees) determines facing direction. Arrow keys adjust this angle. The rendering of the ship rotates accordingly, and the bullet firing direction is aligned with this angle.
  - **Thrust & Movement:** Pressing W adds a forward velocity component in the direction of `player.angle`. S adds a reverse thrust (slower than forward). A/D strafe by adding lateral velocity perpendicular to the angle. Velocity gradually decays (a damping factor of 0.99 each frame) to simulate friction in space. A cap on max speed (`MAX_SPEED`) prevents infinite acceleration.
  - **Ship Resizing:** Changing size not only scales the drawn ship and collision radius but also shifts the bullet spawn point (since the tip of the ship moves with size). This is handled by referencing `player.size` in bullet creation.
  - **Shield:** Toggling the shield sets `player.shieldActive`. If active, an energy meter (`player.energy`) drains at a constant rate. If inactive, energy slowly regenerates. The shield’s state is used during collision checks to nullify damage but still count the collision for explosion or enemy removal.

- **Bullets & Collision:**  
  Bullets are stored in a pool structure for efficiency (reusing inactive slots to avoid constant malloc/free). Each bullet has `x, y` position, a velocity vector, and an `isEnemy` flag:
  - When the player shoots, a bullet is spawned with `isEnemy=0`, positioned at the ship’s tip, and velocity calculated from the ship’s angle.
  - When an enemy shoots (only certain types, like shooters or bosses), a bullet spawns with `isEnemy=1`, originating at the enemy’s position and velocity directed at the player’s current location (with some inaccuracy applied in AI, e.g., ±5° random offset in v0.1.9).
  - Bullet travel: each frame, bullet positions update by their velocity. If a bullet goes further than a set distance from the player or leaves the world bounds, it’s deactivated to free the slot.
  - Collisions are distance-based checks: if a bullet is within 15 pixels of a target (enemy or player), we consider it a hit. This approach is a simplification (treats both as circles), but given bullet speeds and sizes, it works well.
  - Damage: player bullets carry a damage value (default 1, can be modified for special bullets), enemy bullets usually deal 1 damage. Enemies and players have health values, which decrease accordingly.
  - Effects: On enemy death by bullet, we create an explosion entry (as described). On player hit by bullet or enemy, we trigger camera shake and possibly a ship destruction sequence if health ≤ 0.

- **Cryptographic Verification:**  
  The integration of cryptography ensures high scores are legitimate:
  - **RSA Key Generation:** On first run, the game ensures a private key for the user exists (either generated in `.username` hidden file or provided). If not, it uses OpenSSL to generate a 2048-bit RSA key pair. The private key stays on the user’s machine, and the public key is saved in `highscore/public_keys/username.pem`.
  - **Signing Scores:** The function `sign_score(block, username, signature_out)` uses OpenSSL’s EVP interface to create a SHA-256 digest of the block’s data (username|score|timestamp|prev_hash|nonce) and then signs it with the private key. The signature is converted to a hex string and stored in the block. If signing fails (shouldn’t in normal conditions), the block is still added but marked with an empty signature (which will fail verification later).
  - **Verifying Signatures:** The `verify_score_signature(block, username, signature)` does the inverse: loads the public key for `username` and checks the signature against the block’s data digest. This is used both in the `verify_scores.py` script and within the game when loading existing blockchain data (to avoid counting a score that somehow has a bad signature).
  - **Proof-of-Work:** Difficulty is set to 4 leading zeros (can be adjusted in `game.c`). When adding a block, `compute_proof_of_work()` increments the nonce until the SHA-256 hash of the block data has the required zeros. This typically takes on the order of millions of hashes in worst case, but average is lower; with modern CPUs this is fast (a second or two). This mechanism prevents someone from simply editing the blockchain file to a higher score, as they’d need to recompute a valid hash which the verify script would check.
  - **Blockchain Structure:** Each block links to the previous by including the prev block’s hash. This means the entire chain for a user is tamper-evident. If any historical block changed, the chain link would break and verification would fail. In our system, we treat each user’s score history as an independent chain (so `prev_hash` is the hash of that user’s last submitted block). This simplifies multi-user handling: effectively we have many small blockchains rather than one linear chain of all scores. The verify script still scans the whole file but checks continuity per user.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on coding style, issue reporting, and how to submit pull requests. All code submissions are expected to be well-tested; changes to critical systems (like the high score verification or game physics) should be accompanied by appropriate testing or evidence of reliability.

We particularly welcome contributions in the following areas:
- New enemy types or smarter AI behaviors.
- Performance optimizations (especially in collision detection or rendering).
- Cross-platform build support or packaging.
- Enhanced visual/audio effects to make the game more engaging.
- Improved documentation or tutorials for new players.

When contributing, keep the modular structure in mind and aim to make minimal changes to interfaces between modules. This makes it easier to review and integrate changes.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for a version-by-version history of project changes. The highlights for the latest version (v0.1.9) are:

- **Added:** Fullscreen mode support, camera shake on damage, explosion animations on enemy death.
- **Improved:** Shooter enemy AI now maintains optimal distance (~225 units) and fires more aggressively with slight aim variance.
- **Fixed:** Collision detection fully covers all cases (no more “missed” collisions), and duplicate code was removed to fix build issues.
- **Changed:** Internal refactoring for enemy spawn logic and unified debug logging across modules.

Please refer to the changelog for detailed entries, including previous versions (v0.1.8, v0.1.7, etc.), each documenting the evolution of QuantumStriker’s features and fixes in depth.

## TODO

Planned improvements and features under consideration (see [TODO.md](TODO.md) for more):

- [ ] **Audio Effects:** Add sound effects for shooting, explosions, and an adrenaline-pumping soundtrack.
- [ ] **Visual Polish:** Better graphics for ships and enemies (potentially replace primitive shapes with sprites), and particle effects for thrust or shield activation.
- [ ] **Difficulty Modes:** Easy/Normal/Hard settings that adjust enemy spawn rates and AI aggressiveness.
- [ ] **Pause Functionality:** Ability to pause the game, especially needed for longer play sessions.
- [ ] **Online Features:** Perhaps a web scoreboard or P2P verification of scores for a decentralized approach.
- [ ] **Code Refactoring:** Modularize AI behaviors into scriptable components, making it easier to tweak or add new behaviors without touching core game logic.
- [ ] **Unit Tests:** Increase coverage of critical systems (like blockchain verification, collision math).

We aim to keep the game lean and fast, so each feature is weighed against performance and complexity. Community feedback is invaluable—feel free to suggest ideas or vote on priorities in the issue tracker.

Enjoy piloting your ship through the galaxy, and strive for that high score! Good luck, and watch out for those bosses!

---
