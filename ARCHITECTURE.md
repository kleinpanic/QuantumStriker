# QuantumStriker Architecture

**Version:** 0.1.9 (QunatumStriker)  
**Date:** 2025-03-28

This document provides a deep technical breakdown of the QuantumStriker system architecture at version 0.1.9. It covers each module’s purpose, how modules interact, and important algorithms. Special focus is given to complex parts of the code: AI logic, blockchain implementation, cryptographic verification, and the main game loop structure.

## Table of Contents

1. [Introduction](#1-introduction)  
2. [Overall Architecture](#2-overall-architecture)  
3. [Module-by-Module Analysis](#3-module-by-module-analysis)  
   3.1. [Main Module](#31-main-module)  
   3.2. [Game Module](#32-game-module)  
   3.3. [Player Module](#33-player-module)  
   3.4. [Enemy Module](#34-enemy-module)  
   3.5. [Bullet Module](#35-bullet-module)  
   3.6. [Background Module](#36-background-module)  
   3.7. [Score Module](#37-score-module)  
   3.8. [Blockchain Module](#38-blockchain-module)  
   3.9. [Encryption Module](#39-encryption-module)  
   3.10. [Signature Module](#310-signature-module)  
   3.11. [Debug Module](#311-debug-module)  
   3.12. [Versioning](#312-versioning)  
4. [Inter-Module Interactions and Data Flow](#4-inter-module-interactions-and-data-flow)  
5. [Algorithms and Key Concepts](#5-algorithms-and-key-concepts)  
   - 5.1 [Dynamic Ship Sizing](#51-dynamic-ship-sizing)  
   - 5.2 [Advanced Enemy AI](#52-advanced-enemy-ai)  
   - 5.3 [Collision Detection System](#53-collision-detection-system)  
   - 5.4 [Cryptographic Score Security](#54-cryptographic-score-security)  
6. [Conclusion and Future Directions](#6-conclusion-and-future-directions)

---

## 1. Introduction

QuantumStriker (often abbreviated **Q-Striker**) is a 2D space shooter written in C with a focus on both **high performance** and **robust security**. The gameplay features—such as dynamic ship resizing, advanced enemy AI, and procedurally generated environments—are tightly integrated with a blockchain-backed high score system that uses cryptographic proof-of-work and RSA digital signatures for validation. This architecture document details the internal design of each module and explains how they collaborate to create the final game. It also highlights recent improvements made up to version 0.1.9.

The codebase is intentionally modular. Each module has a well-defined interface (via its header file) and encapsulates specific functionality. This modularity allows focused development (e.g., improving AI logic won’t affect rendering code) and simplifies future extensions or refactoring.

---

## 2. Overall Architecture

At the heart of Q-Striker is a **real-time game loop** that drives everything. Each iteration of the loop handles:

- **Input Processing:** Reading keyboard input to control the player’s ship. This includes movement (thrust, rotation, strafing) and actions like shooting, activating the shield, and resizing the ship (a unique feature where UP/DOWN keys adjust ship size on the fly).

- **State Updates:** Progressing all game entities and systems:
  - Moving the player based on physics (current velocity, friction).
  - Spawning new enemies and updating existing enemies’ positions and AI state.
  - Updating all active bullets’ positions.
  - Checking if any explosions (enemy death animations) need updating (radius expansion, fade-out).
  - Tracking score and other HUD elements.
  - Scaling difficulty: as score increases, the game dynamically increases enemy spawn rate and introduces tougher enemy types.
  
- **Collision Detection:** Checking and handling collisions between:
  - **Player bullets vs Enemies:** If a bullet fired by the player intersects an enemy’s hit radius, deal damage and possibly destroy the enemy.
  - **Enemy bullets vs Player:** If an enemy bullet hits the player and the shield is not active, reduce player health (and trigger camera shake effect). If the shield is active, the bullet is absorbed with no health loss.
  - **Enemies vs Player (physical collision):** If an enemy’s position overlaps the player (within a threshold distance), treat it as a collision—damage the player (with shake) and typically destroy the enemy.
  
- **Rendering:** Drawing the current game state to the screen:
  - Draw the scrolling background (stars, etc.) offset by the camera (which follows the player).
  - Draw all game objects: the player’s ship (scaled appropriately), active enemies (each rendered according to type), bullets (with visual distinction if needed, though currently bullets are simple shapes), and the HUD (health, energy, score text) overlaid on top.
  - Apply visual effects: if the camera shake is active, adjust the rendering coordinates for a jittery camera view. If explosions are active, draw expanding circles with transparency for each explosion.
  
- **Score Management & Security:** If the game ends (player dies), initiate the high score submission process. This involves packaging the score into a blockchain “block”, computing a proof-of-work hash, signing it, and saving it to persistent storage. Throughout gameplay, interim score is simply tracked as an integer (based on time + kills).

Each function of the loop corresponds to calls into different modules (player, enemy, bullet, etc.). The loop ensures smooth gameplay by capping frame rate (a small delay each frame to target ~60 FPS) and handling any needed cleanup when exiting (like freeing memory or closing files).

**Isolation:** Each module interacts with the main loop and possibly a few other modules through defined APIs. Global variables are minimized (some exist in config for toggles like `g_fullscreen` or `g_exit_requested`). This separation means, for example, the blockchain code runs largely independently of rendering; it’s invoked only at game over to add a block, but otherwise doesn’t interfere with frame updates.

---

## 3. Module-by-Module Analysis

In this section, we examine each module in detail, focusing on its purpose, internal structure, key functions, and how it interacts with other modules.

### 3.1 Main Module

**Files:** `src/main.c`, `src/version.h`

- **Purpose:**  
  The main module is the application entry point. It parses command-line arguments and sets up global configurations before transferring control to the game loop.

- **Key Responsibilities:**  
  - Parse flags such as:
    - `--version` (print version and exit),
    - `--help` (print usage info),
    - `--debug` (enable debug logs; may also parse a level like `--debug=3` to set verbosity),
    - `--fullscreen` (set `g_fullscreen` flag true to request fullscreen mode),
    - `--highscores` (run high score display routine and exit).
  - Configure debug settings: e.g., set `g_debug_enabled=1` if `--debug` was provided, and possibly set `g_debug_level` accordingly. The debug module uses these to filter log output.
  - Invoke initialization of essential systems if needed (though most init happens in game loop).
  - Finally, call `game_loop()` (from the Game module) to start gameplay if not exiting due to a flag.
  
- **Interactions:**  
  Main primarily interacts with the Game module by calling it, and with the Debug/Version modules for printing info. It doesn’t do heavy game logic itself. After launching the game loop, control returns to main only after the game is closed or an exit condition occurs.

### 3.2 Game Module

**Files:** `src/game.c`, `src/game.h`

- **Purpose:**  
  Encapsulates the **main game loop** and top-level game state management. It’s the orchestrator that ties together players, enemies, bullets, input, and rendering each frame.

- **Key Functions and Structures:**  
  - `game_loop()`: Implements the continuous loop that runs until the game ends. Inside:
    - **Window and Renderer Setup:** Initializes SDL video and TTF, creates a window (`SDL_CreateWindow`) with flags based on `g_fullscreen` (fullscreen desktop if set, or resizable window otherwise). Creates an SDL_Renderer for drawing. Also sets initial `screen_width` and `screen_height` (defaults to 800x600, updated if fullscreen to current display mode).
    - **Resource Initialization:** Opens the game font (`TTF_OpenFont` for HUD text). Allocates arrays for enemies and bullets, and initializes them (setting `active = 0` for all enemies initially, etc.). Also, sets up an array of `Explosion` structures (from game.h) and initializes `lifetime=0` for each to mark them inactive.
    - **User Profile Load:** Loads the username from disk (via Score module, which reads a `.username` file or asks the player). If no RSA key exists for this username, it may trigger key generation (Encryption module).
    - **Game State Variables:** Initializes player position (usually center of world or (0,0)), health, etc., sets starting score to 0, and starts a game timer (`startTime = time(NULL)`).
    - **Main Loop (while running):** Each frame:
      1. Poll SDL events (keyboard). Handle SDL_QUIT (window closed) by setting `running=0`. For keydown events, update player’s desired actions: e.g., set thrust on if W is pressed, rotate left if left arrow pressed, fire bullet if space pressed (with debouncing logic perhaps), toggle shield if E pressed, adjust size if UP/DOWN, etc. Also handle the Right Shift to reset ship size.
      2. Compute delta time or use fixed frame timing (in this case, we use a fixed delay, so each loop is roughly constant time).
      3. Update Player:
         - Calculate new velocity based on thrust keys and current angle.
         - Apply rotation from arrow keys.
         - Update position (x, y) by velocity.
         - Apply damping to velocity (multiply by e.g. 0.99 for friction).
         - Ensure player remains within world bounds (or wrap around if that’s intended, though currently world is large so bounds rarely reached).
         - Update shield energy (if shield is on, drain energy; if off, recharge a bit).
      4. Spawn Enemies:
         - Possibly on a timer or based on random chance influenced by `score`. The spawn function picks an inactive slot, sets it active, positions it at a random angle around the player at some distance (ensuring they spawn off-screen), and chooses type based on current score brackets. Each type has default health and any special timers (e.g., shooters have a shoot cooldown).
      5. Update Enemies:
         - Loop through all active enemies. For each:
           - Compute vector toward player (dx, dy).
           - Different behavior by type:
             - Basic: move towards player.
             - Shooter: maintain distance. If too far, move closer; if too close (< ~150 units), move away. Also, decrement its shoot timer; if timer <= 0 and within a certain range, fire an enemy bullet (reset timer).
             - Fast: move towards player but at higher speed.
             - Evasive: perhaps move in a strafe pattern around player (not too advanced yet).
             - Stealth: maybe toggle visibility (not fully implemented, but reserved).
             - Bosses: could combine shooting and other moves; e.g., Boss1 might spawn additional enemies or shoot bursts.
           - In all cases, update enemy (x, y) based on its velocity or intended movement for this frame.
           - If an enemy goes out of certain bounds or is inactive (health <= 0), skip further processing.
      6. Enemy Shooting:
         - As part of enemy update, certain enemies fire bullets. This uses the Bullet module to spawn a new bullet with `isEnemy=1`, starting at the enemy’s location. Velocity is usually aimed at the player: 
           ```c
           angleToPlayer = atan2(player.y - enemy.y, player.x - enemy.x);
           bullet.vx = cos(angleToPlayer) * speed;
           bullet.vy = sin(angleToPlayer) * speed;
           ```
           A slight random angle offset can be added (as v0.1.9 does ±5°). Enemy bullets might have a different color or simply be tracked via `isEnemy`.
      7. Update Bullets:
         - Loop through active bullets. For each:
           - Add velocity to position.
           - If bullet goes beyond `BULLET_DESPAWN_DISTANCE` (~1000 units from firing point or from player, etc.), mark it inactive to free resources.
           - (If any special bullet behavior like piercing or splitting exists, handle it here. In dev AI mode, `AI_PIERCING_SHOT` allows player bullets to continue after hitting an enemy).
      8. Collision Detection:
         - **Player bullets vs enemies:** Double loop through active bullets and active enemies. If bullet is from player (`!isEnemy`) and distance between bullet and enemy < threshold (15 px):
           - Subtract bullet damage from enemy health.
           - If not a piercing bullet, deactivate the bullet (it “hit” something).
           - If enemy health <= 0: 
             - Increase `enemiesKilled` counter (for scoring).
             - Mark enemy inactive.
             - Trigger explosion: find an `Explosion` slot and initialize it (set x,y to enemy’s position, radius to some initial small value like 5, lifetime to 30 frames).
         - **Enemy bullets vs player:** Loop bullets again; if bullet is enemy (`isEnemy`) and distance to player < threshold (15 px):
           - If player’s `shieldActive` is false:
             - Decrease player health by bullet damage (usually 1).
             - Set `shakeTimer = 20` and `shakeMagnitude = 10` for camera shake.
           - Else (shield on): just absorb (no health loss, maybe play shield hit effect if implemented).
           - Deactivate the bullet.
         - **Enemies vs player:** Loop enemies; if distance from player < threshold (20 px):
           - If shield off:
             - player.health-- (take damage).
             - Trigger camera shake (same as bullet hit).
           - If shield on:
             - (Player doesn’t take damage; you might still apply shake for feedback, but current code just logs shield blocked).
           - In both cases, you likely remove or bounce the enemy. Current code deactivates the enemy (removing it as it “collided and was destroyed”).
      9. Rendering:
         - Compute camera position = player position minus half screen (to center player). If shaking, offset camera by a small random amount in X and Y each frame of shake.
         - Clear screen, draw background via `draw_background(renderer, cam_x, cam_y, width, height)`. The background module uses `cam_x, cam_y` to draw stars relative to camera.
         - Prepare HUD text (health, energy, score, etc.) into a string and render via `render_text` at a fixed screen location.
         - Draw all bullets: each bullet’s on-screen position = world position - camera offset. Likely drawn as small filled circles or rectangles (with different color if enemy bullet).
         - Draw all enemies similarly (often as colored shapes via SDL2_gfx, e.g., filledEllipse for different enemy types). Each enemy type uses a different color/size for identification:
           - Basic: small red ellipse.
           - Shooter: maybe a different color (from code: shooter might still be red with same shape, perhaps we didn’t differentiate color in v0.1.8; could improve).
           - Tanks: larger size.
           - Fast: smaller?
           - Stealth: grey or flickering.
           - Bosses: much larger ellipses with outline (e.g., Boss3 draws a large 32x22 ellipse with a white circle outline to stand out).
         - Draw player: The player is drawn at screen center (since camera is centered on them) by `draw_player(&player, renderer, screen_width/2, screen_height/2)`. This function draws a rotated polygon (5-point spaceship shape) based on `player.angle` and `player.size`. It also draws a rectangle around the ship as a shield indicator if `shieldActive` is true (a simple visual cue).
         - Draw explosions: Loop through `explosions` array; for each active (lifetime > 0), draw a filled circle with radius and alpha corresponding to remaining lifetime. Color chosen is orange (255,165,0) with decreasing opacity. Increment radius and decrement lifetime each frame.
         - Present renderer (SDL_RenderPresent) to display the frame.
      10. Frame delay: SDL_Delay(FRAME_DELAY) to throttle speed (FRAME_DELAY defined as 15 ms => ~66 FPS cap).
      11. Check exit conditions: If `g_exit_requested` was set (e.g., by a signal or by pressing a quit key), break the loop gracefully. Also break if player.health <= 0 (game over handled below).
    - When loop ends (either quit or player died):
      - If player died (health <= 0), run the Game Over sequence:
        - Likely handled inside the loop when health hits 0: display "Game Over" and score summary on screen.
        - Perform the high score submission (detailed in Score/Blockchain modules, but initiated here).
      - Clean up: free bullet pool memory, destroy renderer and window, quit SDL_ttf and SDL, free any allocated strings (username).
  
  - `get_user_top_score(username, &topBlock)`: A helper to retrieve the highest score (and associated block) for the given username from the blockchain file. It opens `BLOCKCHAIN_FILE`, parses each line (using `fscanf` or similar) and verifies signatures to ensure authenticity. It returns the top score and optionally the block data via `topBlock`. This is used to know what the previous score block for the user is, so new blocks can chain correctly and avoid duplicates.
  
- **Interactions:**  
  The Game module heavily interacts with almost all other modules:
  - **Player** (for movement, drawing, shield handling).
  - **Enemy** (for spawning and updating enemies).
  - **Bullet** (for creating and updating bullets).
  - **Background** (for rendering background each frame).
  - **Score** (for username and previous high score retrieval).
  - **Blockchain/Signature/Encryption** (at game over: to add a new score block with proof-of-work and signature).
  - **Debug** (using `DEBUG_PRINT` throughout for logging state changes and critical events).
  
  It acts as the central hub where all these modules come together.

### 3.3 Player Module

**Files:** `src/player.c`, `src/player.h`

- **Purpose:**  
  Manages the player’s state, including movement physics, health, shield, and rendering the player’s ship.

- **Data Structures:**  
  `Player` struct (defined likely in player.h or game.h) with fields such as:
  - Position (`x, y` floats).
  - Velocity (`vx, vy` floats).
  - Angle (float, degrees or radians).
  - Health (int).
  - Shield status (`shieldActive` bool and `energy` float).
  - Size (float, default maybe 10.0 representing ship radius).
  - Possibly additional fields like score (though score is often tracked outside), etc.

- **Key Functions:**  
  - `update_player(Player *p, InputState *input)`: (Pseudo, might not exist as one function, but conceptually) Apply input to player:
    - If input.thrust forward, increase velocity in direction of `p->angle`.
    - If thrust backward, small opposite velocity.
    - If strafe left/right, adjust velocity perpendicular to angle.
    - If rotation left/right, adjust `p->angle` by some rotation speed.
    - Enforce `MAX_SPEED` limit by capping sqrt(vx^2+vy^2).
    - If resizing input, change `p->size` up or down within some min/max bounds. On reset, set `p->size` to default.
  - `update_shield_energy(Player *p)`: Gradually refills or depletes shield energy. Called each frame by game loop after updating shield status.
  - `activate_shield(Player *p, int active)`: Turn shield on or off based on input E key and current energy.
  - `draw_player(Player *p, SDL_Renderer *r, int screen_x, int screen_y)`: Render the ship as a rotated polygon representing a spaceship. Implementation:
    - Define the ship shape in model coordinates (unrotated, centered at (0,0)). For example, a simple arrow: a point at (0, -size) for the tip, and other points forming a triangular or five-point shape for wings and tail.
    - Compute `angleRad = (angle - 90) * (π/180)` so that 0 degrees corresponds to facing up on screen (since we define model with tip facing up).
    - Rotate each model point by `angleRad`, then translate by (screen_x, screen_y) which is typically the center of screen for player.
    - Use SDL2_gfx to draw a filled polygon (filledPolygonRGBA) and an anti-aliased outline (aapolygonRGBA) in green color.
    - If shieldActive, draw a rectangle or circle around the ship to indicate shield. The code draws a rectangle slightly larger than the ship’s bounding box as a quick indicator.
  - Possibly `init_player(Player *p)`: sets starting values (health = 3 or some value, energy = MAX_ENERGY, size = default, position = (0,0), angle = 90 (facing up)).

- **Special Mechanics:**  
  - **Dynamic Ship Sizing:** The Player module implements logic to handle the ship’s size. Pressing UP increments `player.size` (up to some max like 20), DOWN decrements (to a min like 5), and Right Shift sets it back to default (say 10). This affects:
    - The player’s collision radius (bigger ship is easier for enemies to hit).
    - The rendering scale (drawn polygon uses this size).
    - Possibly bullet starting position (bullet appears at tip, which is `size` units from center).
    - The inertia might be slightly affected (mass simulation), but likely not in current simple model.
  
  - **Shield:** Player has a shield with energy. If active, it drains energy at `DEPLETION_RATE` per frame. If energy hits 0, shield auto-deactivates. When off, energy refills at `REFILL_RATE` until max, unless in dev mode where values differ.
  
  - **Health:** Health might start at 3 (3 hits before death). Collisions with enemies or bullets typically subtract 1 health each time. If health <= 0, game over triggers.

- **Interactions:**  
  Primarily with Game (which calls update and draw functions) and Bullet (when shooting, game will call Bullet’s create function with player’s angle and position). Also interacts with Debug (prints events like shield on/off, etc.). There’s minimal direct interaction with Enemy, except indirectly through collisions (which are handled in Game or possibly partially in Player if one wanted to put collision logic there, but currently collisions are in Game loop).

### 3.4 Enemy Module

**Files:** `src/enemy.c`, `src/enemy.h`

- **Purpose:**  
  Defines enemy behavior, including how enemies spawn, move (AI logic), shoot, take damage, and are rendered.

- **Data Structures:**  
  `Enemy` struct likely with fields:
  - Position (`x, y`).
  - Velocity (`vx, vy` or maybe just an angle/direction and speed).
  - Type (enum `EnemyType` such as ENEMY_BASIC, ENEMY_SHOOTER, ENEMY_TANK, ENEMY_FAST, ENEMY_EVASIVE, ENEMY_SPLITTER, ENEMY_STEALTH, ENEMY_BOSS1, ENEMY_BOSS2, ENEMY_BOSS3).
  - Health (int).
  - Active flag (bool).
  - Possibly a `shootTimer` or other AI-specific timers.
  - Possibly `angle` for direction facing, though many just face movement direction (except shooter might not need orientation, they can shoot regardless).
  
  Constants:
  - `MAX_ENEMIES` = 50 (max simultaneous enemies).
  - Properties per type (could be in code or defined as arrays): e.g., health per type (basic=3, shooter=3, tank=9, etc.), speed per type, bullet cooldown per type.

- **Key Functions:**  
  - `spawn_enemy(Enemy enemies[], float player_x, float player_y, int score)`: Finds an available slot in the enemy array, initializes a new enemy there:
    - Chooses a random angle around the player (0-359°) and a random distance (e.g., 150 to 300 units) to position the enemy relative to player. This ensures enemies spawn within a ring around the player but not too close.
    - Determines enemy type based on the current score:
      - If score < 100: only ENEMY_BASIC.
      - score < 500: 80% basic, 20% shooter.
      - score < 1000: 50% basic, 25% shooter, 25% tank.
      - score < 2000: mix in fast and evasive and splitter up to certain percentages.
      - >= 2000: allow all types with certain probabilities (stealth and bosses come in at the tail end: e.g., 5% chance of Boss1, 5% Boss2, 5% Boss3, etc.).
    - Set initial health accordingly.
    - For shooters, possibly set a `shootTimer` (like 120 frames initial cooldown).
    - Mark as active, initialize any other fields (maybe set velocity to some initial value or angle).
  
  - `update_enemies(Enemy enemies[], float player_x, float player_y, float diffScale, BulletPool *bulletPool)`: (Speculative name) Goes through each active enemy and updates behavior:
    - Common: compute distance to player and angle to player for decision-making.
    - By type:
      - **Basic:** Possibly just sets velocity towards player (normalized vector * baseSpeed).
      - **Shooter:** Compute distance `d`. If `d < MIN_DIST (e.g., 150)`, set velocity *away* from player (to increase distance). If `d > MAX_DIST (maybe 300)`, set velocity toward player (to close gap). If in a sweet spot (~150-300 range), maybe stop or circle. Always face/player in terms of shooting direction if needed. Decrement shootTimer; if <= 0 and within some range, fire bullet:
        - Use Bullet module to create an enemy bullet aimed at player with a slight random angle offset of ±5° for imperfect aim.
        - Reset shootTimer to some value (shooters in v0.1.9 have faster firing, maybe shoot every 60 frames).
      - **Tank:** Slow movement towards player. Possibly ignore minor knockback. Just high health.
      - **Fast:** Higher base speed, direct line to player.
      - **Evasive:** Might move towards player but occasionally strafe or change direction unpredictably (could use a sine wave or random offset in movement).
      - **Splitter:** Possibly when killed, spawn two smaller enemies (not sure if implemented yet or placeholder).
      - **Stealth:** Perhaps toggles `active` off (or an invisible flag) every few seconds making it “vanish” from screen temporarily.
      - **Bosses:** 
        - Boss1: high health, behaves like a beefed-up shooter (fires bullets, moderate speed).
        - Boss2: perhaps like a tank (slow but maybe fires in spread).
        - Boss3: highest health, might combine multiple behaviors (like shooting + spawning minions).
    - This function likely also handles enemy firing by calling `enemy_shoot(enemy, bulletPool, player_x, player_y)` if conditions meet.
    - Additionally, ensure enemies stay within a world boundary or handle if they fly off-screen (maybe wrap around or just keep chasing).
  
  - `draw_enemies(Enemy enemies[], SDL_Renderer *renderer, float cam_x, float cam_y)`: Loops active enemies and draws each:
    - For each type, choose a shape/color:
      - Basic: `filledEllipseRGBA(renderer, cx, cy, 15, 10, 255,0,0,255)` (red oval) perhaps.
      - Shooter: maybe the same or different color (in code snippet we saw, shooter wasn’t explicitly different).
      - Tank: maybe bigger oval or different color.
      - Evasive/Fast: possibly smaller or different color (fast might be drawn smaller to indicate physically smaller target).
      - Stealth: drawn in grey or partially transparent.
      - Bosses: larger ellipses with distinct colors and outlines (boss1: purple, boss2: cyan, boss3: dark red with a white outline circle).
    - We saw in code: Boss1 (purple), Boss2 (turquoise), Boss3 (dark red + outline). Stealth was grey.
    - Use SDL2_gfx primitives like `filledEllipseRGBA` and `aacircleRGBA` for outlines.
  
  - `enemy_shoot(Enemy *e, BulletPool *bp, float player_x, float player_y)`: If implemented, this would encapsulate bullet firing. But from changelog, it seems a duplicate definition existed and was removed, possibly meaning the logic was consolidated elsewhere.

- **Enhancements in v0.1.9:**  
  - **Shooter AI Refinement:** Now recalculates target angle every frame, striving for ~225 units distance, firing more frequently and with less randomness (±5° offset). This makes shooters more formidable as they now actively maintain an optimal range and fire faster.
  - **Collision Behavior:** Enemies colliding with player definitely cause damage and die (previous versions had some issues missing these collisions, now fixed).
  - **Spawn Logic:** Further tuned probabilities for each score range to smoothly introduce new enemy types without overwhelming the player immediately.
  - **Explosion Triggering:** Only bullets cause explosions. If an enemy dies from a bullet, we add an explosion effect; if it dies from hitting the player, we currently do not, to differentiate cause (this avoids explosion on player’s ship location, which could be confusing).

- **Interactions:**  
  Enemies interact with:
  - **Player:** by chasing and attacking (via game updating them towards player’s coordinates). They read player’s position for AI, and affect player’s state via collisions and shooting.
  - **Bullet:** spawn bullets (calls bullet module functions to create new bullets flagged as enemy bullets).
  - **Game:** game calls spawn and update functions, passes in references to bullet pool and player coordinates.
  - **Debug:** a lot of debug prints, e.g., logging when a shooter fires, when an enemy dies, etc., with severity levels.

### 3.5 Bullet Module

**Files:** `src/bullet.c`, `src/bullet.h`

- **Purpose:**  
  Manages bullets for both player and enemies. It handles bullet creation, updates their movement each frame, checks world bounds, and renders bullets.

- **Data Structures:**  
  Likely a `Bullet` struct with:
  - Position (`x, y` floats).
  - Velocity (`vx, vy` floats).
  - Active flag (bool).
  - isEnemy flag (int or bool).
  - Damage (int or float, default 1 for normal bullets, could vary).
  
  And a `BulletPool` struct:
  - An array of Bullet (dynamically allocated or fixed size with `INITIAL_BULLET_CAPACITY`).
  - An integer for current count or capacity.
  - Perhaps `activeCount` or similar.

  Constants:
  - `BULLET_SPEED` ~5.0f (for player bullets); enemy bullets might reuse same speed or have their own logic.
  - `BULLET_DESPAWN_DISTANCE` ~1000.0f (distance from origin or player beyond which bullet is removed).

- **Key Functions:**  
  - `init_bullet_pool(BulletPool *bp)`: Allocate memory for bullets (maybe start with 100 capacity), set all as inactive.
  - `spawn_bullet(BulletPool *bp, float x, float y, float angle, int isEnemy)`: Create a new bullet:
    - If pool is full and no inactive bullet available, could resize the pool (hence dynamic pool).
    - Find an inactive bullet (or use the next index if count < capacity).
    - Set bullet.x = given x, bullet.y = given y.
    - Compute velocity from angle: `vx = cos(angle) * BULLET_SPEED`, `vy = sin(angle) * BULLET_SPEED` (note: if angle is in degrees, convert to rad; also angle might need adjusting if 0 degrees is up vs standard).
    - If isEnemy, maybe adjust damage (some enemies could have stronger bullets, but currently all 1 except in dev mode where AI bullet damage is configurable).
    - Set bullet.isEnemy = flag.
    - Mark bullet.active = 1.
    - Increase count if using count of active, or just mark active and leave count as capacity size.
  
  - `update_bullets(BulletPool *bp)`: Loop through bullets:
    - If active, do `bullet.x += bullet.vx; bullet.y += bullet.vy`.
    - Optionally, check distance from player or from spawn:
      - If using player as reference: if `sqrt((x - player.x)^2 + (y - player.y)^2) > BULLET_DESPAWN_DISTANCE` then deactivate.
      - Or if using some absolute world bounds (like WORLD_BORDER).
    - Possibly handle bullet lifetime if needed.
  
  - `draw_bullets(BulletPool *bp, SDL_Renderer *r, float cam_x, float cam_y)`: Draw each active bullet:
    - Compute on-screen coordinates = bullet.x - cam_x, bullet.y - cam_y.
    - Choose color: perhaps player bullets are drawn green or white, enemy bullets red or another distinct color. The code doesn’t explicitly mention bullet color, but one might distinguish via `isEnemy`. For now, assume all bullets drawn as small white rectangles or pixels, or maybe differently colored for debugging.
    - Draw shape: could be `SDL_RenderDrawPoint` or a small filled circle (2px radius) via SDL2_gfx `filledCircleRGBA`.
    - No complex shape needed since bullets are small.
  
  - `free_bullet_pool(BulletPool *bp)`: Free allocated memory (if dynamically allocated bullets).
  
- **Notable Implementations:**  
  - In v0.1.8, an `isEnemy` flag was added to Bullet to allow collision logic to distinguish bullet ownership. This is fully utilized in collision detection now (we check bullet.isEnemy == 0 for hitting enemies, == 1 for hitting player).
  - The bullet physics are straightforward linear. No acceleration or gravity in space.
  - The bullet “up” orientation: It appears the player’s angle is measured such that 0 or 90 might correspond to “up”. The code does an `angleRad = (player.angle - 90) * deg2rad` before computing cos/sin for drawing player. Similarly, bullet spawning likely subtracts 90 to align with visual orientation. That detail is handled within `spawn_bullet` or by game when calling it (if they pass `player.angle - 90` for the bullet direction).

- **Interactions:**  
  - **Player**: When player shoots, game calls Bullet module to spawn bullet at player’s tip position. The tip position can be calculated as:
    ```c
    float tipAngle = (player.angle) * deg2rad;
    float bx = player.x + cos(tipAngle) * player.size;
    float by = player.y + sin(tipAngle) * player.size;
    spawn_bullet(pool, bx, by, player.angle, 0);
    ```
    The bullet module doesn’t know about player except through such parameters.
  - **Enemy**: When enemy shoots (in enemy logic), they call spawn_bullet with their coords and aiming angle, isEnemy=1.
  - **Game**: It orchestrates calling update and draw on bullet pool each frame.
  - **Debug**: May log events like bullet creation or if a bullet goes out of bounds, etc.
  
  The bullet module itself is fairly isolated; it mainly provides utility to other modules.

### 3.6 Background Module

**Files:** `src/background.c`, `src/background.h`

- **Purpose:**  
  Generates and renders the static (or semi-static) background elements like stars, and maybe moving elements like pickups or far planets, to create a sense of a space environment.

- **Data and Configuration:**  
  - Uses constants from config: `WORLD_BORDER` (e.g., 20000 for a 10000x10000 world area), `GALACTIC_OBJECT_DENSITY` to determine how many stars to generate, and `ENABLE_GRID` to toggle drawing a coordinate grid.
  - Likely has an array of background objects (e.g., star positions, maybe types like 0=star, 1=planet, 2=health pickup).
  - `NUM_BG_OBJECTS` computed from density and world size (in provided config, ~20000*20000*5/1000000 = 20000*5/100? Actually that seems huge, but anyway).
  - Each background object: could be a struct with x, y, type, maybe color or size.

- **Key Functions:**  
  - `init_background(Background *bg)`: Randomly generate the positions of background objects:
    - For each index up to `NUM_BG_OBJECTS`, pick a random (x, y) within WORLD_BORDER square.
    - Decide type: e.g., 90% chance star, 5% chance planet, 5% chance pickup (if pickups are implemented).
    - If planet, maybe assign a radius or color.
    - If star, maybe brightness or size (small dot).
    - If pickup, assign type of pickup (like health, but health pickups might not be fully implemented beyond concept).
  - `draw_background(SDL_Renderer *r, float cam_x, float cam_y, int screen_w, int screen_h)`: 
    - Determine which background objects fall within the current view (cam_x to cam_x+screen_w and cam_y to cam_y+screen_h).
    - Draw each:
      - Stars: as small white points or very small rectangles.
      - Planets: as larger colored circles (SDL2_gfx filledCircle perhaps).
      - Grid (if ENABLE_GRID): draw vertical and horizontal lines every some units (like every 1000 units draw a faint line).
      - Possibly draw world border or some boundary if needed.
    - The background should scroll smoothly as the player moves. Since everything drawn is offset by cam_x, cam_y, the starfield will shift opposite to player movement, giving an illusion of movement through space.
    - If an object is at (obj.x=19000, obj.y=500) and camera is at (18000, 0), then on screen it appears at (1000, 500).
    - The background does not update positions (stars don’t move themselves), so it’s static relative to world coords.
  
  - (If pickups present) `check_pickup(Player *p)`: See if player is near a pickup and apply effect (like restore health). But likely not implemented fully at 0.1.9 beyond just visuals.

- **Interactions:**  
  - Only with Game for drawing (and possibly with player if pickups are active).
  - Doesn’t depend on other modules except config for constants.
  - Very independent: one could remove or replace background module without affecting game logic (just aesthetic).

### 3.7 Score Module

**Files:** `src/score.c`, `src/score.h`

- **Purpose:**  
  Handles high score persistence at a basic level and user identification (username management).

- **Functionality:**  
  - **Username Management:** 
    - On startup, the game may call `load_username()`: this looks for a file like `~/.qstriker_username` or a local `.username` file. If found, read the username (string) and return it. If not, possibly prompt the user (if running in interactive mode) to enter a name (or just default to "player").
    - It also triggers RSA key availability: e.g., if no `.username` file, generate keys for the new username. If username exists but no `public_keys/username.pem` exists, maybe generate a new key pair. However, one challenge: if user changes their username, ideally generate a new key to avoid identity confusion.
  - **High Score Files:** 
    - The game might store per-user best score in a separate file aside from blockchain for convenience (like `highscore/<username>.txt`). The Score module could handle reading/writing that, but with blockchain approach it might be redundant. Possibly earlier versions used separate high score files but now blockchain replaced it. The architecture doc suggests it still “reads and writes per-user high score files”, so perhaps each user has a plaintext file storing their max score for quick display in `--highscores` flag.
    - `save_high_score(username, score)`: writes to `highscore/<username>.txt` the score (or updates if higher than existing).
    - `get_last_block_for_user(username, ScoreBlock *block)`: reads through `blockchain.txt` for that user’s latest entry. In `game.c` snippet, `get_last_block_for_user(username, &lastBlock)` returns an `exists` flag indicating if a block was found. It likely parses the blockchain file from the end backwards for that username or keeps track of file offset to last entry.
  
  - **Highscore Listing (for --highscores flag):** 
    - Possibly `print_all_highscores()`: opens `blockchain.txt`, parses all entries, verifies them, and prints a sorted list to console. Or simpler, if they maintain a summary file or individual files, just read those.
    - Given the complexity of verifying on the fly, maybe for the flag they call `verify_scores.py` behind the scenes or require running it prior. But since the question suggests it exists, likely a simplified method: trust the blockchain entries and just read top ones (but that could print cheaters too if not careful).
  
- **Interactions:**  
  - **Encryption Module:** to generate RSA keys for a new user (calls something like `generate_key(username)` which writes the keys to files).
  - **Blockchain Module:** to get the last block for chain linking, and perhaps to assist in verifying or retrieving top scores.
  - **Signature Module:** to verify existing blocks’ signatures if needed when retrieving scores.
  - **Game:** primarily, Game calls Score functions when needed: at start (load username), during gameplay to fetch previous block, and perhaps at end to save a separate “personal best” or so.

### 3.8 Blockchain Module

**Files:** `src/blockchain.c`, `src/blockchain.h`

- **Purpose:**  
  Implements the blockchain-based high score storage mechanism. This includes creating new blocks with proof-of-work, verifying the integrity of the chain, and basic I/O for the blockchain file.

- **Data Structures:**  
  `ScoreBlock` struct (defined in blockchain.h) with fields:
  - username (char[USERNAME_MAX], e.g., 50 bytes).
  - score (int).
  - timestamp (time_t or long).
  - proof_of_work (char[HASH_STR_LEN], 65 bytes for 64 hex chars + null).
  - signature (char[SIG_STR_LEN], perhaps 513 bytes for up to 256-byte sig as hex).
  - prev_hash (char[HASH_STR_LEN]).
  - nonce (unsigned int or similar).
  
  Constants:
  - `HASH_STR_LEN = 65`.
  - `SIG_STR_LEN = 513`.
  - `DIFFICULTY` (defined in game.c as 4) (not in blockchain.h but used in logic).

- **Key Functions:**  
  - `static int hash_meets_difficulty(const char *hash, int difficulty)`: Checks if the given hash string has `difficulty` number of '0' characters at the start.
  - `void compute_block_hash(const ScoreBlock *block, char *output_hash)`: Calculates the SHA-256 hash of the block’s core data (username, score, timestamp, prev_hash, nonce) and outputs it as a hex string. It uses `encryption.h` for `hash_score` presumably. It excludes proof_of_work and signature fields when computing the hash (important, as those are derived).
  - `static void compute_proof_of_work(ScoreBlock *block, int difficulty)`: Brute-force increments `block->nonce` until `compute_block_hash` yields a hash with required leading zeros. It then copies that hash into `block->proof_of_work`.
    - It starts with nonce=0 and goes up by 1 each loop. This can be time-consuming if difficulty is high, but at 4 it’s okay.
    - It prints debug output of each nonce’s hash if debug level is high (to track progress).
    - When found, it prints a success message with the nonce found.
  - `void add_score_block(ScoreBlock *newBlock, const ScoreBlock *prev, int difficulty)`: Prepares a new block for writing:
    - If `prev` is not NULL: copy `prev->proof_of_work` into `newBlock->prev_hash`.
    - If `prev` is NULL (no prior block, i.e., new user): set `newBlock->prev_hash` to all '0's (64 zeros).
    - Set timestamp if not already set (often it’s set when game over occurs, but ensure it’s set).
    - Debug print “Before PoW” with block details (without PoW).
    - Call `compute_proof_of_work(newBlock, difficulty)` to fill in nonce and proof_of_work.
    - Debug print “After PoW” with resulting PoW and nonce.
    - (It does not handle signature; signing is done by Signature module after this.)
    - Note: It doesn’t write to file; it just populates the struct. Writing is handled outside (in game.c).
  - `int verify_blockchain(const ScoreBlock *chain, int count, int difficulty)`: Verify an array of blocks (probably read from file into memory):
    - For each block i in 0..count-1:
      - Recompute hash of block data (using compute_block_hash) to get `recomputed_hash`.
      - Compare `block->proof_of_work` with recomputed_hash; if mismatch, return 0 (PoW stored doesn’t match actual data -> tampering or corruption).
      - Check `hash_meets_difficulty(block->proof_of_work, difficulty)`; if false, return 0 (the PoW hash doesn’t actually meet difficulty, meaning someone may have lowered difficulty in code or faked a hash).
      - If i > 0, also ensure `block->prev_hash == chain[i-1].proof_of_work` (match previous link).
    - If all checks pass, return 1 (valid chain).
    - *Note:* This verifies internal consistency but not digital signatures. Signature verification is done in Signature module.
  - Potentially `int load_blockchain(ScoreBlock **chain, const char *filename)`: to read the JSON lines into an array of ScoreBlock. However, given simplicity, they might use `fscanf` line by line rather than full JSON parsing (which can be brittle but works if format is known exactly).
    - We saw usage of `fscanf` in game.c snippet reading the blockchain for top score. They used a format string with `%49[^\"]` etc. to parse JSON manually, which is workable since format is fixed.
    - That code reads one block at a time and checks signature immediately. So, the Score module or Game is partially doing that reading as needed.

- **File Format (blockchain.txt):**  
  Each line is a JSON object like:
  ```json
  {"username":"Alice", "score":1234, "timestamp":1670000000, "proof_of_work":"0000abcd1234...","signature":"abcdef1234...", "prev_hash":"0000prevhash...", "nonce":99999}
  ```
  The newline separates blocks. The system does not enforce a global chain order; it's essentially an append-only log.

- **Interactions:**  
  - **Encryption Module:** for hashing (calls `hash_score()` internally) and perhaps reading keys.
  - **Signature Module:** not strictly required for PoW, but signature generation happens right after PoW in game.c `sign_score(&newBlock, username, newBlock.signature)`. The Blockchain module doesn’t know about RSA, it just ensures data integrity for PoW and linking.
  - **Score Module:** Possibly uses blockchain functions to get last block for a user.
  - **Game:** uses `add_score_block` when creating a new score block on game over.
  - **Scripts:** The Python verification script essentially reimplements `verify_blockchain` and signature checks in Python. The C code’s verify function might not be used in game itself except maybe if one wanted to double-check after writing (but likely not done to save time).

### 3.9 Encryption Module

**Files:** `src/encryption.c`, `src/encryption.h`

- **Purpose:**  
  Provides low-level cryptographic operations: hashing and RSA key management. Essentially, it acts as a thin wrapper around OpenSSL for our specific needs.

- **Key Functions:**  
  - `int generate_rsa_key(const char *username)`: Generate a new RSA key pair using OpenSSL:
    - Create a keypair (2048 or 4096 bits, likely 2048 for speed).
    - Save private key to a file, possibly named `.username` in the project directory (or `highscore/privkeys/username.pem`).
    - Save public key to `highscore/public_keys/username.pem`.
    - Might also write the username file to store the username (so we know which private key to load next time).
    - Return 1 on success.
    - This might be called if no existing key is found for a username.
  - `EVP_PKEY* load_private_key(const char *username)`: Load the private key for given username from file (for signing).
  - `EVP_PKEY* load_public_key(const char *username)`: Load public key (for verifying).
  - `void hash_score(const char *input, char *output_hash)`: Compute SHA-256 of input string and put hex string in output (likely uses OpenSSL EVP or SHA256_Update functions).
    - Called in blockchain’s `compute_block_hash`.
  - Possibly some utility to ensure OpenSSL is initialized (though modern usage might not need explicit init).
  
- **Interactions:**  
  - **Blockchain:** uses `hash_score` to compute hashes for PoW.
  - **Signature:** uses `load_private_key` to get key for signing, `load_public_key` for verifying.
  - **Score Module:** uses `generate_rsa_key` when a new user is created.
  - **Game:** not directly, except via Score (for key gen) or indirectly via above.

- **Notes:**  
  OpenSSL usage can be complex. Likely, encryption.c has minimal code and defers heavy lifting to OpenSSL library. Also, keys are not stored in memory permanently—private key is loaded, used, and freed just-in-time for signing.

### 3.10 Signature Module

**Files:** `src/signature.c`, `src/signature.h`

- **Purpose:**  
  Handles creating and verifying digital signatures for score blocks using RSA keys.

- **Key Functions:**  
  - `int sign_score(const ScoreBlock *block, const char *username, char *signature_hex)`: Sign the given block’s data:
    - Use `encryption.load_private_key(username)` to get `EVP_PKEY` private key.
    - Prepare the data to sign: must be exactly the data included in the block’s hash, otherwise signature would be pointless. They likely use a helper `get_block_data_string` or similar to format `"%s|%d|%ld|%s|%u"` (same as in compute_block_hash).
    - Use OpenSSL EVP APIs to create a SHA256 digest and sign it:
      - `EVP_DigestSignInit` with SHA256.
      - `EVP_DigestSignUpdate` with the data bytes.
      - `EVP_DigestSignFinal` twice (first to get sig length, second to actually get signature bytes).
    - Convert signature bytes to hex string (two hex digits per byte).
    - Free OpenSSL contexts and key.
    - Return 1 if successful, 0 if any failure.
    - If fails, they might DEBUG_PRINT an error.
  
  - `int verify_score_signature(const ScoreBlock *block, const char *username, const char *signature_hex)`: Verify that the signature_hex is a valid signature of block’s data by user’s public key:
    - Load public key for username.
    - Convert hex signature to binary bytes (need to do this to use OpenSSL verify, likely they do similar but in reverse of sign; maybe they wrote a small helper not shown in snippet).
    - Use `EVP_DigestVerifyInit` with SHA256 and public key.
    - `EVP_DigestVerifyUpdate` with data bytes.
    - `EVP_DigestVerifyFinal` with signature bytes. It returns 1 if valid, 0 otherwise.
    - Free contexts and key.
    - Return 1 if valid, 0 if not.
    - DEBUG_PRINT errors if any step fails (like failing to load key or verify).
  
- **Interactions:**  
  - **Game/Blockchain:** game calls `sign_score` after computing PoW to finalize the block. It provides `newBlock` (with PoW and nonce set) and username.
  - **Verification (scripts or game):** when reading blockchain entries to validate, use `verify_score_signature` for each block and its stored signature.
  - **Encryption:** uses it for key loading and hashing context (since signature code also forms the data string similarly to blockchain hash).
  - **Debug:** logs signature failures or mismatches.

### 3.11 Debug Module

**Files:** `src/debug.c`, `src/debug.h`

- **Purpose:**  
  Provides a unified logging mechanism across the project with severity levels and color output, and respects global debug flags.

- **Design:**  
  - Global variables:
    - `g_debug_enabled` (0 or 1).
    - `g_debug_level` (1-3 for different verbosity levels, default 1).
    - `g_always_print` (an integer flag introduced in v0.1.9 for detail level 0).
  - The macro `DEBUG_PRINT(detail, severity, fmt, ...)` is defined to check conditions and print to stderr:
    - If severity == 0 (error), always print (in red).
    - Else if detail == 0 and `g_always_print` is true, print regardless of g_debug_enabled (this is the new change: so developers can mark crucial info with detail=0 that should show even if debugging isn’t fully on, as long as `g_always_print`=1).
    - Else if `g_debug_enabled` is true and `g_debug_level == detail`, then print. This means you set a debug level (1, 2, or 3) and only messages matching that detail number print (besides errors or always_print ones).
    - Color is chosen by severity:
      - 0 = red (error)
      - 1 = yellow (warning)
      - 2 = blue (info/debug)
      - 3 = green (success)
    - After printing the message, resets color.
    - This macro is used throughout code, giving context like `DEBUG_PRINT(3,2,"Enemy bullet blocked by shield.")` meaning detail 3 (maybe high-level game events) and severity 2 (just an info), which will print only if debug enabled and level 3 is set, and in blue text.
  
- **Functions:**  
  - In `debug.c`, probably minimal because macro does most work. They define the globals there (with default values).
  - Possibly a function to set debug level or parse environment, but likely not needed.

- **Changes in 0.1.9:**  
  They added `g_always_print` and logic for detail=0 to always print if that flag is set (the changelog mentions “Changed config.h and config.c to add a 0 integer that will cause printing regardless of severity or --debug option”, that refers to this always_print mechanism).
  
- **Interactions:**  
  - All modules include debug.h and use DEBUG_PRINT for logging events, errors, etc. It's a cross-cutting concern but in a controlled way.
  - `main.c` likely sets `g_debug_enabled=1` if `--debug` flag, and maybe sets `g_debug_level` if provided or uses default.
  - `g_always_print` might be set to 1 by default (as in debug.c it is set to 1 by default), meaning detail 0 messages show even if debug disabled. So detail=0 is effectively an unconditional print now (for things that should always be shown, such as maybe important info or critical errors, but error severity 0 already printed always anyway; so possibly they wanted a non-error always message option).

### 3.12 Versioning

**File:** `src/version.h`

- **Purpose:**  
  Contains definitions for version numbers to avoid hard-coding strings in multiple places.

- **Contents:**  
  - Likely macros or constants:
    - `#define VERSION "0.1.9"`
    - Maybe separate major/minor, but probably just a single string or compound define.
  - Possibly a `#define CODENAME "Nebula Nexus"` if they use codenames, as the architecture doc had a title Nebula Nexus (just a guess from context).
  - This header is included where needed (main for `--version` print, README generator might embed it, etc.).

- **Use:**  
  - The game on `--version` will print out that string.
  - Documentation and logs might reference it via macro expansion if they integrated that, but mostly for internal consistency.

---

## 4. Inter-Module Interactions and Data Flow

This section explains how modules communicate and share data during execution. It provides a high-level picture of data flow in key scenarios.

- **Startup Sequence (Main → Game):**  
  `main.c` processes arguments and then calls `game_loop()`. Before that, it may set some global flags (e.g., `g_fullscreen`, debug flags). When `game_loop` begins, it uses those flags to initialize SDL window and internal settings. Main does not feed data continuously to Game; it’s a one-time setup handoff.

- **Real-Time Gameplay (Game Loop):**  
  Inside `game_loop`:
  - The **Game module** polls input. The state of keys (e.g., bool thrustOn, bool leftPressed) is determined and then used to update the **Player module** state. No direct function call perhaps; game might manipulate player’s struct directly or via a small wrapper.
  - Game then calls `update_enemies(enemies, player.x, player.y, diffScale, &bulletPool)` (Enemy module). Here, game passes player’s position and possibly a difficulty scale (diffScale might be derived from score, making enemies faster as score grows). The Enemy module moves enemies and may call Bullet module functions to shoot.
    - When an enemy decides to shoot, it calls something like `spawn_bullet(&bulletPool, enemy.x, enemy.y, angleToPlayer, 1)` to create an enemy bullet (Bullet module function). So data flows: Enemy → Bullet (new bullet data).
  - After enemy update, game calls `update_bullets(&bulletPool)` (Bullet module) to move all bullets.
  - **Collisions:** Game doesn’t have a distinct module, it handles in-line but conceptually:
    - For each collision event:
      - If player bullet hits enemy: game updates Enemy (reduces health, might call an Enemy function like `damage_enemy(enemy, dmg)` or just do it directly since struct is known). If enemy dies, game triggers explosion: writing into an Explosion struct (which is part of Game, not a separate module).
      - If enemy bullet hits player: game updates Player (health--, also sets global shakeTimer var which is defined in config or game). Shield check uses Player’s shield status field.
      - If enemy collides with player: game updates Player (health-- etc.) and Enemy (mark inactive). 
    - These interactions involve Player and Enemy data structures, but handled in Game logic rather than calling a function in those modules. It’s shared data usage, indicating those structures are accessible at game scope. This is possible because player, enemies, bullets etc. are likely defined in game.c (like static arrays or local arrays passed around).
  - **Rendering:** Game calls:
    - `draw_background(renderer, cam_x, cam_y, w, h)` – Background module uses cam and screen info to draw. Data: it reads global background objects (randomly initialized at start) and takes current camera offset from game.
    - `render_text(renderer, font, ... HUD...)` – part of game or maybe Score module if HUD included score. But `render_text` is a helper in game.c (since we saw it defined in game.c code).
    - `draw_bullets(&bulletPool, renderer, cam_x, cam_y)` – Bullet module draws each bullet at (bullet.x - cam_x, bullet.y - cam_y).
    - `draw_enemies(enemies, renderer, cam_x, cam_y)` – Enemy module draws each enemy similarly offset.
    - `draw_player(&player, renderer, screen_center_x, screen_center_y)` – Player module draws the ship at the center of screen coordinates.
    - These calls illustrate data flow: Game provides each module with the necessary context (like camera offset or target position, e.g., player.x passed to enemy updates) but modules do their internal tasks. The Player draw function doesn’t need the camera offset because we always draw player at screen center (the world moves around the player).
  - Between frames, there’s not much persistent cross-module data aside from the game state containers (player struct, enemy array, bullet pool). Each module function typically only requires a pointer/reference to those.

- **High Score Submission (Game → Score/Blockchain/Signature):**  
  When game ends:
  - Game collects final `score` and `username`.
  - It calls Score’s `get_last_block_for_user(username, &lastBlock)` to retrieve the last blockchain entry for that user (if any). This function internally:
    - Opens blockchain.txt, scans for records matching username, verifies signatures as it goes (so it doesn’t pick an invalid block). The snippet shows it comparing signature and only considering the score if signature is valid.
    - It returns a copy of the last valid block and perhaps the top score for reference (although game just needs the last block to chain).
  - Game prepares `ScoreBlock newBlock` with username, score, timestamp.
  - Calls Blockchain’s `add_score_block(&newBlock, prevExists ? &lastBlock : NULL, DIFFICULTY)`. This populates prev_hash, computes PoW, and sets nonce and proof_of_work in newBlock.
  - Game then calls `sign_score(&newBlock, username, newBlock.signature)`. Signature module loads the private key and signs the block’s data, writing hex signature into newBlock.signature. Now newBlock is fully assembled.
  - Game opens `blockchain.txt` and writes the new block as a JSON line. This is pure file I/O with fprintf.
  - If writing fails, logs an error; if success, logs success.
  - Data flow here:
    - Score/Blockchain retrieval: Score returns some data to game.
    - Game updates newBlock.
    - Blockchain function modifies newBlock with PoW.
    - Signature function modifies newBlock with signature.
    - Game outputs newBlock to file.
    - After that, Score might also update `highscore/username.txt` or similar with this score if higher than previous, but that’s optional.

- **Verification (Script side, but if inside program):** If the `--highscores` flag is used:
  - Possibly Game or Score module will call a function to read all blockchain entries:
    - Use Blockchain’s verify functions or replicate logic: open file, read all lines into ScoreBlock array, call verify_blockchain on the whole set, and also for each block call verify_score_signature (Signature module) with the public key.
    - Filter out invalid ones, sort valid ones, print out.
  - However, since this might be heavy, the actual program could just spawn `verify_scores.py` and capture output, but likely not; they probably implemented a simpler direct approach with the code they have (since they already have C logic to parse and verify as seen).

- **Inter-module data summary:**
  - Player, Enemy, Bullet, Explosion, etc., share position data in a common coordinate system (world coordinates). The camera in Game mediates converting those to screen coordinates for rendering.
  - Timing and frame pacing is mostly within Game; other modules do not manage time.
  - The blockchain and score modules interface with file storage, while game and others interface with real-time input and graphics devices.

## 5. Algorithms and Key Concepts

This section dives deeper into a few critical algorithms and design concepts in QuantumStriker.

### 5.1 Dynamic Ship Sizing

- **Concept:** Allow the player to change the size of their ship during gameplay, affecting difficulty and gameplay style.
- **Implementation:**  
  - Player’s `size` attribute can be modified by input:
    - Pressing **UP** arrow increases size by a small increment (e.g., +1 unit, clamped to some upper limit).
    - Pressing **DOWN** decreases size (clamped to a lower limit, not allowing zero or negative).
    - Pressing **Right Shift** resets to default size.
  - The change in `size` directly affects:
    - The vertices used in `draw_player` (ship shape scaled by this size).
    - The position where bullets spawn relative to player (we ensure bullets start at tip of the ship, which is `size` units from center).
    - Collision detection threshold for enemy hitting player might implicitly change because if size is larger, the distance < 20 check would trigger slightly earlier if we considered the shape. However, currently the collision distance (20) is static, so effectively a larger ship is easier to hit because its corners might extend beyond 20. In a more precise system, we’d use size as part of that check.
  - The algorithm to apply sizing is straightforward increment/decrement and assign. Notably, resizing is purely client-side (no network or external effect).
- **Why it’s complex:** It’s unusual in games to allow dynamic hitbox changes; it could affect balance. Also, need to ensure shape drawing and bullet offsets always use the updated size correctly, which the code handles by referencing `player.size` whenever needed rather than assuming a constant.

### 5.2 Advanced Enemy AI

The AI logic for enemies, especially the shooter type, is a highlight of the game’s complexity.

- **Shooter Enemy Behavior (Detailed):**  
  - **Maintaining Distance:** The shooter computes its distance to the player each frame:
    - If distance < ~50 (a minimum safety radius), it will thrust in the opposite direction from the player to avoid getting too close.
    - If distance > ~300 (too far), it will move towards the player to close the gap.
    - Ideally, it wants to stay around 200-250 units away (mid-range). In v0.1.9, it’s tuned to target ~225 units.
  - **Continuous Tracking:** Every frame, the shooter recalculates the vector to the player. There’s no heavy pathfinding; it’s a simple reactive approach (steer towards or away).
  - **Shooting with Imperfect Aim:** When within a reasonable range (~ within 300 units and maybe line-of-sight), the shooter fires bullets:
    - Each shot’s angle = exact angle to player + a random offset between -5° and +5°. This means not every shot is perfectly on target, giving the player a slight chance to dodge even if on a straight line.
    - Rate of fire: in v0.1.9, they reduced the shooter’s shoot cooldown (maybe from 120 frames to something lower, perhaps 60 or 30), making it shoot more frequently.
  - **No Friendly Fire/Collision among enemies:** The AI does not consider other enemies or obstacles; they all chase the player independently.
  - **Shooter State Variables:** Implementation uses a `shootTimer` per shooter enemy. When <= 0, trigger shoot and reset to some value (depending on difficulty maybe).
  - **Outcome:** This AI leads to a behavior where shooter enemies strafe around or keep a distance while peppering the player with bullets, which is more challenging than a basic homing enemy.

- **Other AI Types:** (Simpler algorithms)
  - **Evasive:** Could involve a random perpendicular component to movement to not come directly at player, or switching direction frequently. Perhaps it picks a random angle around player instead of straight line, making its path unpredictable.
  - **Fast:** Just like basic but with a higher velocity, making it harder to hit or avoid.
  - **Tank:** Moves slowly; algorithm might just be “if far, move towards player; if within some range, stop”, relying on high health as its feature.
  - **Splitter:** Possibly upon death, spawn two smaller basic enemies at the death location (algorithm: in collision handling, if type == splitter, spawn two new basics with half health or something).
  - **Stealth:** Maybe toggles visible/invisible (could be implemented by not drawing it every few frames, or by having a cloak timer).
  - **Bosses:** Typically combine patterns:
    - e.g., Boss3 might have logic: if health drops below X, increase speed or spawn minions. Not sure if implemented, but design could allow it.

- **Spawn Algorithm:**  
  This algorithm ensures progression:
  - It uses random percentage rolls to decide type, with thresholds that shift as score increases.
  - This way, early on, mostly basics; mid-game adds some shooters and tanks; late game adds everything with some rare bosses.
  - It doesn’t explicitly limit how many enemies at once except by MAX_ENEMIES (50). The spawn rate likely is controlled by a timer or by continuously spawning when count is below a certain threshold relative to score (not explicitly shown, but perhaps `if (rand() < some_prob(score)) spawn_enemy()` each frame).
  - This keeps difficulty scaling manageable—e.g., you won’t see a boss before 2000 score, etc.

- **AI Loops Complexity:**  
  We must be cautious with performance:
  - Checking each enemy each frame (up to 50) is fine.
  - For each, maybe scanning for player is constant time, minimal math.
  - The heavy part could be collisions double-loop (50 enemies * bullets, up to maybe 100 bullets = 5000 checks worst-case, which is okay).
  - The AI as implemented is simple enough not to bottleneck, but effective in gameplay.

### 5.3 Collision Detection System

Collision checks are vital for gameplay correctness. Q-Striker uses simple distance-based checks, which are computationally straightforward but effective given the shapes involved.

- **Player Bullet vs Enemy:**  
  - If the distance between a bullet and an enemy is less than 15 units, consider it a hit.
  - This effectively models a radius of 15 around the enemy (or bullet, depending how you think of it) where a collision occurs. Given the enemy sizes (some bigger than 15 radius), this might slightly disadvantage larger ones (they might get hit even if bullet is near their edge).
  - The algorithm: for each active bullet (player bullet) and each active enemy, compute 
    \[
    \text{dist} = \sqrt{(bullet.x - enemy.x)^2 + (bullet.y - enemy.y)^2} 
    \]
    (They likely compare squared distance to 15^2 to avoid the sqrt for performance, but the snippet shows a sqrtf call, which is fine at these scales).
  - If dist < 15:
    - Reduce enemy.health by bullet.damage.
    - If not in dev mode with piercing, deactivate bullet (so it doesn’t hit multiple).
    - If enemy.health <= 0:
      - Mark enemy inactive (so it’s removed).
      - Trigger explosion (only for bullet kills).
      - Increment kill count.
  
- **Enemy Bullet vs Player:**  
  - For each enemy bullet, compute distance to player similarly < 15 check.
  - If hit:
    - If shield is off:
      - player.health--.
      - Trigger camera shake (set shakeTimer=20, shakeMagnitude=10).
    - If shield on:
      - No health loss (bullet effectively blocked).
    - Bullet is always removed (hit or blocked).
  
- **Enemy vs Player (Physical collision):**  
  - For each enemy, compute distance to player < 20 check.
  - If collision:
    - If shield off:
      - player.health--.
      - Trigger shake.
    - If shield on:
      - (Possible design: maybe shield absorbs the impact too, maybe taking extra energy, but not in code; code just logs bullet block, and likely similar for enemy, just doesn’t reduce health).
    - In both cases, remove enemy (the enemy basically crashes and is destroyed).
    - There’s no explicit damage to enemy because it’s assumed to die on impact (especially smaller ones). Bosses colliding might be edge-case; ideally, bosses wouldn’t charge directly or if they do, maybe it should hurt them too, but given design, enemy is removed regardless of type.
  
- **Explosion vs anything:**  
  - Explosions are just visual, not actually causing further damage (no chain reactions or area damage).
  
- **No player bullet vs player or enemy bullet vs enemy checks** (friendly fire or self-harm):
  - Player bullets presumably don’t hit player (spawn ahead of ship and isEnemy prevents that logic).
  - Enemies likely cannot shoot each other (and even if bullet is friendly only to them, no logic to check enemy bullet vs other enemies).
  - This simplifies checks.

- **Performance Consideration:**  
  - The double loop (bullets vs enemies) at most 100*50 = 5000 checks per frame if both arrays are full, which is fine in C at 60 FPS (300k checks/sec).
  - The enemy vs player loop is 50 checks per frame, negligible.
  - Using sqrt in inner loops repeatedly could be a micro-optimization target (they could use squared distance compare to avoid sqrt). Since they didn’t, perhaps clarity was prioritized or it was fast enough.

- **Accuracy:**  
  - Pure distance checks treat objects as circles. Our player ship is a polygon but roughly size ~10 units radius, so 15 threshold is a bit generous, meaning near misses might count as hits sometimes. But this is acceptable in an arcade game and might even be necessary if bullet and enemy move fast (to not slip through without registering due to frame step).
  - The shield mechanism effectively widens the player’s collision tolerance (with shield on, you can ignore bullet and enemy hits because you don’t lose health, but still possibly see shake or at least see collisions in logs).
  - If needed, collision algorithm could be refined (polygon intersect for player shape vs bullet, etc.), but that’s more complex and not needed for this style.

### 5.4 Cryptographic Score Security

This aspect sets QuantumStriker apart: using blockchain and cryptography to secure high scores.

- **Proof-of-Work (PoW) Algorithm:**  
  - Each score submission requires finding a SHA-256 hash with a certain pattern (leading zeros).
  - This is analogous to mining a block in a cryptocurrency blockchain, albeit with a much lower difficulty.
  - The data hashed: `username|score|timestamp|prev_hash|nonce`. Note it doesn’t include the signature or proof_of_work in the hashing process (those are computed after).
  - Difficulty = 4 means the hash must start with "0000". In hex, each '0' means 4 bits of zero, so effectively 16 leading zero bits. The expected number of tries is 16^4 = 65536 on average (since each hex digit is uniformly random if hash is random). That’s pretty quick.
  - This difficulty is fixed in code. If cheating became rampant or machines got so fast, one could raise it to, say, 5 or 6 for more work.
  - The nonce is a 32-bit number cycling from 0 upwards. There’s no upper bound check in `compute_proof_of_work`, it will go indefinitely until it finds a valid hash. But since difficulty is low, it's fine.
  - The result, once found, is stored in `block.proof_of_work` and the nonce in `block.nonce`.
  - This ensures that generating a block (score entry) involves some computation, preventing spamming of fake scores or at least making it computationally costly to alter a score (because any change to the data invalidates the PoW and you'd have to recompute it).
  - Also, it ties into verification: if someone manually edits their score in the file, the proof_of_work no longer matches and verify_scores.py will catch it.

- **Digital Signatures (RSA):**  
  - Each player has a unique RSA key pair. This binds scores to an identity.
  - The **signature covers the same data fields** that are hashed (username, score, timestamp, prev_hash, nonce). So effectively, the player signs the block (except the signature itself, obviously).
  - When verifying, we check the signature using the public key for that username:
    - If someone tries to fake another user’s score, they’d have to produce a valid signature with that user’s key, which they can’t without the private key.
    - If someone changes the score or timestamp of an existing block, the signature won’t match because the signed data changed.
  - Key management: The private key is stored locally for the player (not in the repo), so only they can create a signature for themselves. The public key is stored in the repo (or at least accessible to verify script) so others can verify it.
  - RSA 2048 is presumably used. Signature length ~256 bytes, hex string length 512 characters (hence SIG_STR_LEN=513 with null).
  - The game code automatically generates a key if not found, meaning any new username gets keys. If two people use the same username (coincidentally), they'd each generate different keys and potentially conflict on verification (one's scores would appear invalid under the other’s public key). This is a potential issue if not managed (like by having a global registry of usernames). But assuming mostly single player, it's fine.

- **Blockchain Linking:**  
  - Each block contains `prev_hash` which should equal the previous block’s `proof_of_work` (for that user’s chain).
  - This means all of a single user’s scores form a chain. If someone tries to remove an earlier block or insert a fake block in middle, the chain breaks and verify catches it.
  - For multiple users, since all blocks are stored in one file, it’s more like multiple parallel chains. The verify script probably doesn’t enforce an order across different users (like a global chain) because prev_hash of a new user’s genesis is zeros and doesn’t link to others. So each username starts a new chain.
  - This way, each user’s history is secure, but one user’s chain doesn’t necessarily lock in another user’s chain. The design could have been one global chain, but that would require consensus on ordering (which commit appended first, etc.). The chosen approach isolates users, which is simpler.

- **Verification Process (end-to-end):**  
  1. For each block line read:
     - Parse JSON to ScoreBlock struct.
     - Recompute hash of [username|score|timestamp|prev_hash|nonce] and compare to stored proof_of_work.
       - If mismatch or no leading zeros: mark invalid (maybe cheater).
     - Load that username’s public key and verify signature on same data.
       - If signature invalid: mark invalid.
     - If not genesis (not first for user), ensure prev_hash equals last seen valid block’s proof_of_work for that user.
       - If not: mark invalid or at least break that user’s chain (so maybe treat subsequent scores of that user as invalid too unless starting new chain).
  2. Collate results:
     - Valid blocks are candidates for leaderboard. Possibly only take each user’s highest score among their valid blocks (the game design implies one user can have many entries, but only highest matters).
     - Invalid blocks: list them under cheaters, because they indicate either someone altered a score or something went wrong (like if someone tried to submit a score without properly signing, etc.).
  3. Update README accordingly.

- **Security Considerations:**  
  - It’s not impossible to cheat if someone really wanted to:
    - They could find the private key file of a user and use it to sign fake data (so protect your private key).
    - They could brute force a collision for PoW and signature, but that’s astronomically hard if signature is needed (basically break RSA or SHA256).
  - But for an open-source game, this is quite robust for casual cheating prevention. It’s educational too.

- **Performance:**  
  - PoW difficulty 4 is light, maybe a second or two to compute in C worst-case. RSA signing is also fast enough (ms range). This doesn’t hamper user experience much.
  - The verification script in Python doing many SHA256 and RSA verifies could be slower, but running once a week is fine.

---

## 6. Conclusion and Future Directions

As of version 0.1.9, QuantumStriker’s architecture is a blend of classic game development practices and innovative security measures:

- The **core game loop** ensures smooth real-time action, managing input, physics, AI, collisions, and rendering in a deterministic, fixed-step manner.
- A **modular code structure** divides responsibilities, making the code easier to maintain and extend. For instance, the addition of fullscreen support and visual effects in v0.1.9 was largely confined to the Game module (for camera shake logic, window flag changes) and did not require changes to unrelated modules.
- **Advanced features** like dynamic ship sizing and evolving enemy AI make gameplay more engaging, while **robust cryptographic verification** of high scores provides a level of trust and integrity rarely seen in offline games.

The most complex areas—enemy AI and the blockchain system—demonstrate how seemingly disparate domains (game AI and cryptography) can coexist in one project. Careful consideration was given to ensure these systems do not interfere with each other’s performance or logic; they intersect only at defined points (game over, or when spawning an enemy shooter’s bullet, etc.), preserving modularity.

**Recent Improvements Recap (v0.1.9):**  
- We polished the AI for shooter enemies, making them smarter and more challenging without adding much overhead.
- We introduced fullscreen mode and special effects (camera shake, explosions) to enhance the game’s feel, all integrated seamlessly into the existing loop.
- We fixed prior bugs in collision detection and removed redundant code (like duplicate functions), which simplifies the codebase and reduces potential errors.

**Future Directions:**  
Looking ahead, several paths can further elevate QuantumStriker:

- **Enhanced AI Coordination:** We could implement group tactics for enemies (e.g., formation flying or flanking maneuvers) to increase depth. This might involve an AI manager module orchestrating enemy patterns, which could still feed into the existing Enemy module structure.
- **Networking and Multiplayer:** Though ambitious, converting the high score blockchain into an online decentralized leaderboard, or even adding multiplayer gameplay, would be a significant next step. The cryptographic foundations are laid; extending them to a network context (like a server verifying scores or a peer-to-peer verification among players) could be explored.
- **Graphics and Engine Upgrades:** Currently using SDL2 for all rendering, one might integrate a GPU for more effects (OpenGL/Vulkan) especially if adding particle systems or more complex animations. The architecture supports replacing the rendering backend while keeping game logic intact.
- **Modularity & Scripting:** To allow easier AI tweaks or new enemy types, a scripting system (Lua or Python integration) could let developers or modders define behaviors without recompiling C code. The modular separation would help slot this in, perhaps at the Enemy module level.
- **Cross-Platform Optimization:** Ensure the game and cryptography run efficiently on different platforms (Windows, Linux, macOS, possibly consoles or mobile). This may entail abstracting some platform-specific bits (already relatively abstract with SDL and OpenSSL).
- **User Experience:** Add menus, pause, game settings (to toggle fullscreen or adjust difficulty), and more polished HUD elements. These don’t deeply affect the architecture but enhance the overall product.

In conclusion, QuantumStriker’s architecture is robust and flexible, balancing game performance with security. By separating concerns into modules and using established libraries for heavy lifting (SDL for graphics, OpenSSL for crypto), the project achieves a lot with a small codebase. The current design is scalable for many future enhancements, providing a strong foundation as we move from Nebula Nexus (v0.1.9) towards more feature-rich releases. Each new feature can be layered on by extending the relevant module or adding a new one, ensuring that QuantumStriker can continue to evolve without sacrificing the clarity and reliability of its core structure.

