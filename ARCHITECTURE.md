# QuantumStriker (Q-Striker) Source Code Documentation

*Version: NEBULA_NEXUS_VERSION (see version.h)*

*Date: 2025-03-26*

---

## Table of Contents

1. [Introduction](#introduction)
2. [Overall Architecture](#overall-architecture)
3. [Module-by-Module Analysis](#module-by-module-analysis)  
   3.1. [Main Module](#main-module)  
   3.2. [Game Module](#game-module)  
   3.3. [Player Module](#player-module)  
   3.4. [Enemy Module](#enemy-module)  
   3.5. [Bullet Module](#bullet-module)  
   3.6. [Background Module](#background-module)  
   3.7. [Score Module](#score-module)  
   3.8. [Blockchain Module](#blockchain-module)  
   3.9. [Encryption Module](#encryption-module)  
   3.10. [Signature Module](#signature-module)  
   3.11. [Debug Module](#debug-module)  
   3.12. [Versioning](#versioning)
4. [Inter-Module Interactions and Data Flow](#inter-module-interactions-and-data-flow)
5. [Algorithms and Key Concepts](#algorithms-and-key-concepts)
6. [Conclusion and Future Directions](#conclusion-and-future-directions)

---

## 1. Introduction

QuantumStriker (Q-Striker) is a 2D space shooter designed around the principles of minimalism and efficiency. Its codebase, written in C, is split into a collection of modules under the src directory. This documentation paper explains in detail the internal logic of each component, the algorithms used for movement, collision detection, dynamic object management, and the innovative blockchain-based high score system that secures user scores with cryptographic proof-of-work and digital signatures.

---

## 2. Overall Architecture

At its core, Q-Striker is structured around a real-time game loop that handles:
- **Input Processing:** Reading keyboard events for player control.
- **State Updates:** Updating the player, enemy, and bullet states; adjusting enemy spawns dynamically.
- **Collision Detection:** Handling interactions between bullets and enemies, and enemy-player collisions.
- **Rendering:** Drawing the background, player ship, enemies, bullets, and HUD elements.
- **Score Management:** Calculating and securely storing high scores using a blockchain mechanism.

Each functional area is implemented in its own module, which simplifies maintenance and future enhancements. Inter-module communication is done through well-defined APIs in the header files.

---

## 3. Module-by-Module Analysis

### 3.1 Main Module

**Files:** `main.c`, `version.h`

- **Purpose:**  
  The main module serves as the entry point to the application. It parses command-line arguments and invokes the game loop.
  
- **Key Features and Logic:**  
  - **Command-Line Argument Processing:**  
    Iterates through `argv` to check for flags such as `--version`, `--help`, `--debug`, `--fullscreen`, and `--highscores`.  
    For example, if `--version` is provided, it prints the version from `version.h` and exits.
  - **Debug Configuration:**  
    The `--debug` flag sets global variables (`g_debug_enabled` and `g_debug_level`) used by the debug module for logging.
  - **Delegation:**  
    When no special flags are detected, `main()` calls `game_loop()` from the game module to start the game.

### 3.2 Game Module

**Files:** `game.c`, `game.h`

- **Purpose:**  
  This module contains the main game loop and orchestrates overall game logic. It integrates input processing, state updates, collision detection, score submission, and rendering.

- **Key Functions:**  
  - **Initialization:**  
    - **SDL & TTF Setup:** Initializes SDL, SDL_ttf, and creates the window and renderer.  
    - **Font Loading:** Loads a TTF font (Arial.ttf) for HUD and text rendering.
    - **User Authentication:**  
      Uses functions from the score module to load a username from `.username`; if not found, it calls `prompt_username()` (rendered in the game window) and saves the new username.
    - **Key Pair Setup:**  
      Ensures that a valid RSA key pair exists (via encryption module) before gameplay begins.
    - **Game Objects Initialization:**  
      Initializes player (with default position, velocity, health, energy, and shield status), bullet pool, and enemy array.
  - **Input Processing:**  
    Polls SDL events and checks the keyboard state to update:
    - **Rotation:** Left/Right arrow keys rotate the ship.
    - **Movement:** WASD (with optional speed multiplier via Control keys) apply thrust, reverse thrust, and strafing.
    - **Shooting:** Space key fires a bullet from the ship’s tip.
    - **Shield Activation:** Key E activates the shield, which is also managed by energy depletion and refilling.
  - **Game Updates:**  
    Each frame, the game:
    - Updates player position (using physics with inertia and damping).
    - Updates shield energy based on whether the shield is active.
    - Updates bullet positions (with dynamic pool management).
    - Dynamically spawns enemies based on a timer and a spawn rate that increases with the score.
    - Updates enemy positions using a simple chase algorithm (see Enemy Module).
  - **Collision Detection:**  
    - **Bullets vs. Enemies:**  
      Computes Euclidean distance between each active bullet and enemy. If within collision threshold, enemy health is reduced, and the bullet is deactivated. Enemies are deactivated when their health reaches zero.
    - **Enemies vs. Player:**  
      Checks if any active enemy is within a threshold distance of the player. If so, and if the shield is not active, the player’s health is reduced.
  - **Score Calculation & Blockchain Integration:**  
    - The score is based on survival time plus a bonus for each enemy destroyed.
    - Upon game over (player health ≤ 0), the module:
      - Retrieves the last block for the user from the blockchain file.
      - Constructs a new `ScoreBlock` (with fields: username, score, timestamp, nonce, previous block’s hash).
      - Computes proof-of-work by iterating the nonce until the SHA‑256 hash meets a difficulty (leading zeros).
      - Signs the block using RSA digital signatures.
      - Appends the new block to `highscore/blockchain.txt` in JSON format.
  - **Rendering:**  
    - **Camera Logic:**  
      Computes a camera offset so that the player is centered.
    - **Drawing Order:**  
      1. Background (grid and starfield)  
      2. HUD (health, energy, score, position, angle)  
      3. Bullets  
      4. Enemies  
      5. Player ship  
    - Uses SDL and SDL_ttf for drawing text and SDL2_gfx for primitives.
  - **Frame Timing:**  
    Uses a fixed delay (`FRAME_DELAY`) to regulate frame rate and smooth gameplay.

### 3.3 Player Module

**Files:** `player.c`, `player.h`

- **Purpose:**  
  Implements the behavior and physics of the player’s ship.

- **Data Structures:**  
  - **Player Structure:**  
    Contains world coordinates (`x, y`), velocity components (`vx, vy`), a rotation angle (in degrees), health, energy, and a shield activation flag.

- **Key Functions and Algorithms:**  
  - **Initialization (`init_player()`):**  
    Sets starting values (position at origin, angle 90° indicating “up”, health, energy set to maximum, and shield deactivated).
  - **Rotation (`rotate_player()`):**  
    Adjusts the player's angle, wrapping around to remain within 0–360°.
  - **Thrust & Movement:**  
    - A common helper, `apply_thrust()`, calculates acceleration by converting an angle offset (relative to the ship’s current facing direction) into a velocity increment using sine and cosine functions.  
    - Specific functions (`thrust_player()`, `reverse_thrust()`, `strafe_left()`, `strafe_right()`) call `apply_thrust()` with appropriate offsets (0°, 180°, -90°, 90°).
  - **Update (`update_player()`):**  
    - Integrates velocity into position.
    - Caps speed by calculating the Euclidean norm of the velocity vector; if it exceeds `MAX_SPEED`, scales it down.
    - Applies a damping factor (`DAMPING`) to simulate friction/inertia.
  - **Shield Energy Management (`update_shield_energy()`):**  
    - When the shield is active, energy is depleted at a fixed rate (`DEPLETION_RATE`).  
    - When inactive, energy refills up to a maximum (`MAX_ENERGY`) at a slower rate (`REFILL_RATE`).
  - **Rendering (`draw_player()`):**  
    - Uses a fixed ship model defined in local coordinates (a five-point polygon resembling an arrow).
    - Rotates the polygon according to the ship’s angle (using standard 2D rotation transformation).
    - Draws a border (using anti-aliased polygon drawing) and, if the shield is active, an additional rectangle is rendered as a shield indicator.
  - **Utility (`get_ship_tip()`):**  
    - Calculates the tip of the ship (critical for bullet spawn location) by rotating the model’s tip coordinate and adding the player’s current world position.

### 3.4 Enemy Module

**Files:** `enemy.c`, `enemy.h`

- **Purpose:**  
  Manages enemy entities including their initialization, basic AI movement, spawning, and rendering.

- **Data Structures:**  
  - **Enemy Structure:**  
    Holds the enemy’s position (`x, y`), health, a type identifier (for future AI extensions), and an `active` flag indicating whether the enemy is in play.

- **Key Functions:**  
  - **Initialization (`init_enemies()`):**  
    Iterates through a fixed-size array (size defined by `MAX_ENEMIES`) and sets the `active` flag to 0.
  - **Spawning (`spawn_enemy()`):**  
    - Scans the array for an inactive enemy.
    - Uses random angle (converted to radians) and a random distance (between 150 and 300 units) from the player's current position to calculate spawn coordinates.
    - Sets enemy health scaled by a spawn rate modifier and marks the enemy as active.
  - **Movement Update (`update_enemies()`):**  
    - For each active enemy, calculates the difference between the enemy’s position and the player’s position.
    - Applies a fixed movement increment (0.5f for the x-axis and 0.3f for the y-axis) multiplied by a difficulty factor, nudging the enemy toward the player.
    - Incorporates threshold checks to avoid jittery movements when already very close.
  - **Rendering (`draw_enemies()`):**  
    - Uses SDL2_gfx’s `filledEllipseRGBA()` to draw enemies as red ellipses.
    - The rendering is performed relative to the camera position.

### 3.5 Bullet Module

**Files:** `bullet.c`, `bullet.h`

- **Purpose:**  
  Manages the creation, updating, and rendering of bullets fired by the player.

- **Data Structures:**  
  - **Bullet Structure:**  
    Contains position (`x, y`), velocity (`dx, dy`), and an `active` flag.
  - **BulletPool Structure:**  
    Manages a dynamic array of bullets along with a capacity count.

- **Key Functions:**  
  - **Initialization (`init_bullet_pool()`):**  
    Allocates an initial pool of bullets (default capacity of 100) and marks them inactive.
  - **Dynamic Addition (`add_bullet()`):**  
    Searches for an inactive bullet slot to reuse; if none is found, doubles the pool capacity using `realloc` and initializes the new slots.
  - **Shooting (`shoot_bullet()`):**  
    - Computes the bullet’s velocity by converting the player’s firing angle to radians.  
    - Adjusts for the fact that the ship’s angle is defined relative to “up” by subtracting 90°.  
    - Calls `add_bullet()` to insert the new bullet into the pool.
  - **Update (`update_bullets()`):**  
    Iterates over active bullets, updates their positions by adding the velocity components, and deactivates them if they exit a defined game boundary (e.g., beyond ±5000 units).
  - **Rendering (`draw_bullets()`):**  
    Renders each active bullet as a small yellow rectangle centered on its current position (adjusted by camera offset).

### 3.6 Background Module

**Files:** `background.c`, `background.h`

- **Purpose:**  
  Provides a dynamic, star-filled background that gives the impression of deep space.

- **Data Structures:**  
  - **BGObject Structure:**  
    Defines a background object with type (star, planet, or health pickup), position, size, and color.
  - **Static Array:**  
    Maintains a fixed number (`NUM_BG_OBJECTS`) of background objects which are randomly initialized once per game session.

- **Key Functions:**  
  - **Initialization (`init_background_objects()`):**  
    Seeds the random generator, and iterates through the array, assigning:
    - **Type:**  
      - Stars (70% chance) – drawn as small pixels with bright color values.
      - Planets (20% chance) – larger circles with random colors.
      - Health objects (10% chance) – fixed-size objects with a distinct green color.
    - **Position:**  
      Distributed across a large world coordinate range.
  - **Rendering (`draw_background()`):**  
    - Clears the screen with a deep-space color.
    - Draws a Cartesian grid using a fixed spacing (`GRID_SPACING`) to provide a sense of scale.
    - Renders each background object if it falls within an extended camera view (with an extra buffer of ±100 units).
    - Uses simple SDL primitives: rectangles for stars/health and a custom `draw_filled_circle()` for planets.

### 3.7 Score Module

**Files:** `score.c`, `score.h`

- **Purpose:**  
  Manages persistence for high scores and user identification.

- **Key Functions:**  
  - **Username Persistence:**  
    - `load_username()`: Reads the username from the file `.username`, stripping newline characters.
    - `save_username()`: Writes the username to the `.username` file.
  - **High Score Handling:**  
    - `load_highscore_for_username()`: Constructs a file path (`highscore/<username>_highscore.txt`), reads an integer score from it.
    - `save_highscore_for_username()`: Writes a score into the corresponding high score file.
  - **Blockchain Linkage:**  
    - `get_last_block_for_user()`: Opens the blockchain file and uses `sscanf` to parse JSON-formatted blocks. It finds the most recent block for the given username (based on timestamp) to facilitate proper chaining when submitting a new score.

### 3.8 Blockchain Module

**Files:** `blockchain.c`, `blockchain.h`

- **Purpose:**  
  Secures the high score system by recording scores in a blockchain structure, ensuring tamper resistance through proof‑of‑work and cryptographic signatures.

- **Key Data Structure:**  
  - **ScoreBlock Structure:**  
    Defined in `blockchain.h`, it includes:  
    - `username`: User who submitted the score.  
    - `score`: The numerical score.  
    - `timestamp`: Time of submission.  
    - `proof_of_work`: The SHA‑256 hash meeting difficulty requirements.  
    - `signature`: Digital signature (RSA) over the block’s key fields.  
    - `prev_hash`: The hash from the previous block in the chain.  
    - `nonce`: A number that is incremented until a valid proof-of‑work is found.

- **Key Functions:**  
  - **Hash Computation (`compute_block_hash()`):**  
    - Constructs a buffer string by concatenating the username, score, timestamp, previous hash, and nonce (formatted appropriately).
    - Calls `hash_score()` (from the encryption module) to compute the SHA‑256 digest.
  - **Proof-of-Work (`compute_proof_of_work()`):**  
    - Iterates, starting with nonce zero, recalculates the block hash each time until the hash has a specified number of leading zeros (difficulty requirement).
  - **Block Addition (`add_score_block()`):**  
    - Sets the `prev_hash` to the previous block’s proof-of-work or to a genesis value (all zeros) if no previous block exists.
    - Computes the valid proof-of-work and updates the block’s nonce and proof.
  - **Blockchain Verification (`verify_blockchain()`):**  
    - Iterates through an array of blocks and verifies that:
      - The stored proof-of-work matches the recomputed hash.
      - The hash meets the difficulty requirement.
      - Each block’s `prev_hash` correctly matches the proof-of-work of the previous block.

### 3.9 Encryption Module

**Files:** `encryption.c`, `encryption.h`

- **Purpose:**  
  Provides cryptographic primitives and RSA key management using OpenSSL.

- **Key Functions:**  
  - **SHA‑256 Hashing (`hash_score()`):**  
    - Computes a SHA‑256 hash on input data and formats it as a hex string.
  - **RSA Key Pair Generation (`generate_keypair()`):**  
    - Creates an RSA key pair (2048 bits) using OpenSSL’s EVP APIs.
    - Saves the private key in the `.username` file (appended after the username line) and the public key in the `highscore/public_keys/` directory.
  - **Key Loading:**  
    - `load_private_key()`: Reads the `.username` file, extracts the PEM block, and uses OpenSSL to load the private key.
    - `load_public_key()`: Loads the public key from the corresponding PEM file.
  - **Key Verification (`ensure_keypair()`):**  
    - Checks if the private key exists and is valid; if not, it regenerates the key pair.

### 3.10 Signature Module

**Files:** `signature.c`, `signature.h`

- **Purpose:**  
  Responsible for generating and verifying digital signatures on score blocks to ensure authenticity.

- **Key Functions:**  
  - **Data Preparation:**  
    - `get_block_data_string()`: Assembles the block’s immutable fields (username, score, timestamp, previous hash, nonce) into a single string that represents the data to be signed.
  - **Signing (`sign_score()`):**  
    - Loads the private key for the user.
    - Initializes an OpenSSL digest context for SHA‑256.
    - Signs the assembled block data and converts the resulting binary signature to a hex string.
  - **Verification (`verify_score_signature()`):**  
    - Loads the public key.
    - Converts the provided hex signature back into binary.
    - Verifies the signature against the block data using the public key.

### 3.11 Debug Module

**Files:** `debug.c`, `debug.h`

- **Purpose:**  
  Implements a macro-based logging system that prints messages with varying levels of detail and color-coded severity.

- **Key Features:**  
  - **Global Debug Flags:**  
    - `g_debug_enabled`: Enables or disables debug output.
    - `g_debug_level`: Controls the granularity of debug messages.
  - **DEBUG_PRINT Macro:**  
    - Accepts parameters for detail, severity, and a printf‑style format string.
    - Uses ANSI color codes to format output (red for errors, yellow for warnings, blue for debug, green for success).
    - Prints messages conditionally based on the current debug level and severity.

### 3.12 Versioning

**File:** `version.h`

- **Purpose:**  
  Centralizes the version information of the game (e.g., `"0.1.7"`).
- **Usage:**  
  Accessed by the main module (and optionally displayed via the `--version` flag) to report the current release version.

---

## 4. Inter-Module Interactions and Data Flow

The modules interconnect in the following manner:

- **Main Module → Game Module:**  
  After processing command-line options, `main.c` calls `game_loop()`, transferring control to the game module.

- **Game Module ↔ Player/Bullet/Enemy/Background Modules:**  
  - **Input Processing:**  
    The game loop retrieves keyboard states and calls functions from the player module (e.g., `rotate_player()`, `thrust_player()`).
  - **Dynamic Updates:**  
    The game loop calls `update_player()`, `update_bullets()`, and `update_enemies()`, integrating physics and AI.
  - **Rendering:**  
    The game loop draws the background, bullets, enemies, and player by calling `draw_background()`, `draw_bullets()`, `draw_enemies()`, and `draw_player()` in sequence.
  - **Collision Detection:**  
    Checks are performed in the game module, with bullet-enemy and enemy-player interactions updating health and enemy states.

- **Game Module → Score/Blockchain/Signature/Encryption Modules:**  
  - **Score Submission:**  
    Upon game over, the game module constructs a new `ScoreBlock`. It calls `get_last_block_for_user()` from the score module to maintain blockchain linkage.
  - **Blockchain Integrity:**  
    The game module calls `add_score_block()` to compute the proof‑of‑work and update the block, then calls `sign_score()` from the signature module to digitally sign the block.
  - **Encryption & Key Management:**  
    Before gameplay, the game module ensures a key pair exists via `ensure_keypair()` from the encryption module.

- **Debug Module Usage Across All Modules:**  
  Every module calls `DEBUG_PRINT` with various detail and severity levels to log internal state, function entry/exit, and key variable values. This is critical for tracing the game’s execution flow and troubleshooting.

---

## 5. Algorithms and Key Concepts

### Proof-of-Work Algorithm

- **Concept:**  
  To secure score submissions, each block must have a hash that starts with a specified number of zeros.
- **Implementation:**  
  The nonce is incremented repeatedly in `compute_proof_of_work()`, and for each iteration, `compute_block_hash()` is called.  
  The helper function `hash_meets_difficulty()` checks whether the hash’s leading characters meet the difficulty level.
  
### RSA Digital Signatures

- **Purpose:**  
  Ensure that score blocks are authenticated and tamper-resistant.
- **Flow:**  
  1. The block’s immutable data is concatenated into a string.
  2. `sign_score()` uses the private key (loaded via OpenSSL) to create a signature.
  3. The signature is stored as a hex string alongside the block.
  4. Verification (`verify_score_signature()`) uses the public key to confirm the signature.

### Dynamic Memory Management for Bullets

- **Bullet Pool Resizing:**  
  The bullet pool starts with a fixed size. As bullets are fired and slots fill up, the pool doubles in size using `realloc()`, ensuring efficient memory usage without a hard limit on bullets.

### Physics and Movement

- **Inertia Simulation:**  
  Player movement uses simple physics: acceleration is added to velocity based on thrust input; velocity is then applied to update position; damping is applied to simulate friction.
- **Angular Rotation:**  
  Rotation is implemented using 2D rotation matrices to transform ship coordinates, ensuring that the ship’s tip (used for bullet spawning) is accurately calculated.

---

## 6. Conclusion and Future Directions

This documentation has provided an exhaustive review of Q-Striker’s source code logic, detailing every module within the src directory. The design prioritizes minimalism, efficiency, and security:
- The game loop integrates physics, AI, collision detection, and rendering seamlessly.
- The innovative blockchain high score system leverages cryptographic proof-of-work and RSA signatures to secure player achievements.
- Modular organization facilitates maintenance and future enhancements, as outlined in the accompanying TODO documentation.

Future enhancements may include expanding enemy AI, a more robust decentralized high score mechanism, enhanced UI/UX elements, and cross-platform improvements. This document serves as the definitive reference for developers seeking to understand or extend QuantumStriker’s core logic.

---
