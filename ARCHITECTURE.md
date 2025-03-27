Below is an updated version of your ARCHITECTURE.md document that reflects the new nature of Q‑Striker after the recent improvements. This version incorporates the dynamic ship sizing, refined enemy AI (including shooter enemies that keep a safe distance and fire with imperfect aim), enhanced collision detection between player and enemy (as well as enemy–bullet collisions), and the updated bullet module with ownership flags.

---

# QuantumStriker (Q‑Striker) Source Code Documentation

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

QuantumStriker (Q‑Striker) is a 2D space shooter built on minimalistic, efficient C code. The project emphasizes both fast performance and robust security: gameplay elements such as dynamic ship sizing and advanced enemy AI are integrated with a blockchain‑backed high score system secured via cryptographic proof‑of‑work and RSA digital signatures. This document details the internal logic and design of each module while highlighting recent improvements.

---

## 2. Overall Architecture

Q‑Striker’s core is a real-time game loop that handles:

- **Input Processing:** Capturing keyboard events to control the player’s ship—including dynamic resizing.
- **State Updates:** Updating the positions and states of the player, enemies, and bullets. Enemies now include a variety of types with distinct AI behaviors (for example, shooter enemies that maintain a safe distance and fire with imperfect aim).
- **Collision Detection:** Detecting and processing collisions between:
  - Player bullets and enemies,
  - Enemy bullets and the player (with shield blocking), and
  - Direct collisions between enemies and the player (inflicting damage).
- **Rendering:** Drawing the background, player ship (with dynamic sizing), enemies (with advanced AI behaviors), bullets (with ownership markers), and HUD.
- **Score Management & Security:** Calculating and securely recording high scores via a blockchain mechanism, ensuring each score submission is verifiable with cryptographic proof‑of‑work and digital signatures.

Each module is isolated by its header/API to facilitate independent development and future extensions.

---

## 3. Module-by-Module Analysis

### 3.1 Main Module

**Files:** `main.c`, `version.h`

- **Purpose:**  
  Acts as the entry point, parsing command‑line arguments and delegating execution to the game loop.
- **Key Features:**  
  - Processes flags such as `--version`, `--help`, `--debug`, `--fullscreen`, and `--highscores`.
  - Configures debug logging (via `g_debug_enabled` and `g_debug_level`).
  - Calls `game_loop()` to start the main game once options are processed.

### 3.2 Game Module

**Files:** `game.c`, `game.h`

- **Purpose:**  
  Contains the main game loop and orchestrates overall game logic.
- **Key Functions and Enhancements:**  
  - **Initialization:**  
    - Sets up SDL, TTF, and the main window/renderer.
    - Loads a TTF font for text rendering.
    - Handles user identification via `.username` and ensures RSA key pair availability.
    - Initializes all game objects (player, bullet pool, enemy array).
  - **Input Processing:**  
    - Processes player controls including rotation, thrust, strafing, and dynamic ship sizing (using UP/DOWN for size adjustments and Right Shift to reset to default).
    - Processes shooting (space key) and shield activation (key E).
  - **State Updates:**  
    - Updates player physics (integrating thrust, applying inertia, and handling damping).
    - Dynamically spawns enemies based on the current score (ensuring that advanced types such as shooter enemies are included).
    - Updates enemy AI:  
      - **Shooter Enemies:** Now try to maintain a distance of about 50 units from the player (backing off if too close) while firing with a random (±10°) offset when within 300 units.
    - Updates bullet positions and manages collisions.
  - **Collision Detection:**  
    - **Player Bullets vs. Enemies:** Reduces enemy health and deactivates both bullets and enemies on impact.
    - **Enemy Bullets vs. Player:** Damages the player if the shield is not active.
    - **Direct Enemy–Player Collisions:** If any enemy (of any type) comes within 20 units of the player, the player’s health is reduced and the enemy is deactivated.
  - **Score & Blockchain:**  
    - Calculates score based on survival time and enemies destroyed.
    - On game over, constructs and submits a new score block to the blockchain.
  - **Rendering:**  
    - Uses camera logic to center the view on the player.
    - Draws the background, HUD, bullets, enemies, and the dynamically sized player ship in proper order.
  - **Frame Timing:**  
    Maintains smooth gameplay using a fixed delay between frames.

### 3.3 Player Module

**Files:** `player.c`, `player.h`

- **Purpose:**  
  Implements player physics and rendering.
- **Enhancements:**  
  - **Dynamic Ship Sizing:**  
    The player can increase, decrease, and reset the ship size using UP, DOWN, and Right Shift keys.  
  - **Physics:**  
    Applies thrust based on input, simulates inertia with damping, and caps the maximum speed.
  - **Shield Management:**  
    Controls shield energy (depletion when active, refilling when inactive) and renders a shield indicator around the ship.
  - **Rendering:**  
    Draws the ship as a five-point polygon that rotates according to the current angle.

### 3.4 Enemy Module

**Files:** `enemy.c`, `enemy.h`

- **Purpose:**  
  Manages enemy behavior, including spawning, movement, and advanced AI.
- **Enhancements:**  
  - **Enemy Type Diversity:**  
    Supports up to 10 enemy types (7 regular and 3 bosses) with varying health, speed, and behavior.
  - **Shooter Enemy AI:**  
    Shooter enemies now attempt to maintain a minimum distance of 50 units from the player. If they get too close, they move away; otherwise, they approach and fire bullets at the player with a random offset.
  - **Collision Behavior:**  
    Enemies will damage the player on collision (direct contact within 20 units).
  - **Spawning Logic:**  
    The enemy spawn function now uses the current score to decide which type of enemy to spawn, ensuring a gradual introduction of advanced enemy types.

### 3.5 Bullet Module

**Files:** `bullet.c`, `bullet.h`

- **Purpose:**  
  Manages bullet creation, updating, and rendering.
- **Enhancements:**  
  - **Ownership Flag:**  
    Each bullet now carries an `isEnemy` flag to distinguish between player-fired and enemy-fired bullets.
  - **Dynamic Pool:**  
    Bullets are managed in a dynamically resizing pool.
  - **Trajectory Calculation:**  
    Converts the firing angle (adjusted for “up” orientation) into velocity components.
  
### 3.6 Background Module

**Files:** `background.c`, `background.h`

- **Purpose:**  
  Renders a dynamic, star-filled background with a Cartesian grid.
- **Key Logic:**  
  - Randomly initializes background objects (stars, planets, and health pickups) with different probabilities.
  - Draws these objects relative to the camera’s position.

### 3.7 Score Module

**Files:** `score.c`, `score.h`

- **Purpose:**  
  Handles high score persistence and username management.
- **Key Functions:**  
  - Loads and saves the username.
  - Reads and writes per-user high score files.
  - Retrieves the most recent blockchain block for proper score chaining.

### 3.8 Blockchain Module

**Files:** `blockchain.c`, `blockchain.h`

- **Purpose:**  
  Secures the high score system using a blockchain structure.
- **Key Concepts:**  
  - Each score block includes the username, score, timestamp, proof‑of‑work, digital signature, previous block’s hash, and nonce.
  - Proof‑of‑work is computed by iterating the nonce until the SHA‑256 hash meets a difficulty requirement.
  - Blocks are chained together and signed using RSA digital signatures.

### 3.9 Encryption Module

**Files:** `encryption.c`, `encryption.h`

- **Purpose:**  
  Provides cryptographic primitives (SHA‑256 hashing, RSA key generation) using OpenSSL.
- **Key Functions:**  
  - Generates RSA key pairs, saves the private key in `.username`, and the public key in `highscore/public_keys/`.
  - Loads and verifies keys as needed for signing and verifying score blocks.

### 3.10 Signature Module

**Files:** `signature.c`, `signature.h`

- **Purpose:**  
  Generates and verifies digital signatures for score blocks.
- **Key Functions:**  
  - Constructs a string from the block’s immutable fields.
  - Uses RSA and SHA‑256 to sign this string.
  - Verifies signatures using the public key.

### 3.11 Debug Module

**Files:** `debug.c`, `debug.h`

- **Purpose:**  
  Implements a macro-based logging system with varying detail levels and color-coded output.
- **Key Features:**  
  - Global flags for enabling/disabling debug output and setting verbosity.
  - DEBUG_PRINT macro used throughout the code to trace execution and variable states.

### 3.12 Versioning

**File:** `version.h`

- **Purpose:**  
  Centralizes version information (e.g., “0.1.8”) used throughout the application and displayed via command‑line flags.

---

## 4. Inter-Module Interactions and Data Flow

- **Main Module to Game Module:**  
  `main.c` handles startup and command‑line arguments, then calls `game_loop()`.

- **Game Module and Core Gameplay:**  
  The game loop in `game.c` manages input processing, state updates, and rendering. It interacts with:
  - **Player Module:** For movement, dynamic sizing, shield management, and rendering.
  - **Enemy Module:** For spawning (based on the current score), updating enemy positions and behavior, and invoking enemy shooting.
  - **Bullet Module:** For creating, updating, and drawing bullets—with ownership flags for collision detection.
  - **Background Module:** For drawing the starfield and grid.
  - **Score/Blockchain/Signature/Encryption Modules:** For score calculation, blockchain submission, and cryptographic operations.

- **Collision Detection:**  
  - **Player Bullets vs. Enemies:**  
    When a bullet (owned by the player) hits an enemy, enemy health is reduced; if it reaches zero, the enemy is deactivated.
  - **Enemy Bullets vs. Player:**  
    When an enemy bullet collides with the player (and the shield is off), the player's health is reduced.
  - **Direct Enemy–Player Collision:**  
    When any enemy collides with the player (distance below a threshold), the player's health is reduced and the enemy is deactivated.

- **Debugging:**  
  All modules use the DEBUG_PRINT macro to log events based on the current debug level.

---

## 5. Algorithms and Key Concepts

### Dynamic Ship Sizing

- **Mechanism:**  
  The player’s ship size can be increased or decreased on-the-fly using the UP and DOWN keys. Right Shift resets the size to a default value. This is implemented in the player module and affects both rendering and the calculation of the bullet spawn point.

### Advanced Enemy AI

- **Shooter Enemy Behavior:**  
  Shooter enemies now attempt to maintain a minimum distance (≈50 units) from the player. If they get too close, they reverse their movement to avoid collision while continuing to fire at the player with an imperfect aim (random offset ±10°).
- **Spawning Algorithm:**  
  The enemy spawn function now uses the current score to probabilistically determine enemy types. As the score increases, advanced enemy types such as shooter, tank, evasive, fast, splitter, stealth, and bosses appear.
  
### Collision Detection

- **Multi-Level Collisions:**  
  - Bullets vs. Enemies: Checks if distance < 15 units.
  - Enemy Bullets vs. Player: Uses the `isEnemy` flag to reduce health if the shield is inactive.
  - Direct Enemy–Player: Uses a collision threshold (e.g., 20 units) to reduce health and deactivate the enemy.
  
### Cryptographic Score Security

- **Blockchain High Score System:**  
  Combines proof-of‑work and RSA digital signatures to ensure that each score block is tamper‑resistant. Score blocks are chained via previous hashes and stored in JSON format.

---

## 6. Conclusion and Future Directions

This document outlines Q‑Striker’s current architecture—a highly modular design that separates core gameplay, physics, enemy AI, rendering, and security into discrete, maintainable modules. The latest updates introduce dynamic ship sizing, sophisticated enemy behavior (especially for shooter enemies), and robust collision detection that ensures all forms of contact (bullet, enemy, and direct collision) correctly affect gameplay.

Future enhancements may include:
- Further refinements to enemy AI (such as coordinated attacks or more nuanced evasive maneuvers).
- Additional gameplay modes or customizable controls.
- More advanced cryptographic features or a decentralized leaderboard system.
- Cross‑platform optimizations and richer UI/UX enhancements.

This architecture is designed to be both flexible and scalable, paving the way for continued innovation and feature expansion in Q‑Striker.

--- 

