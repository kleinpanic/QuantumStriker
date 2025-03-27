# QuantumStriker

## Disclaimer 

> if you want to enjoy this game, and submit your high score, please be aware that it may be lost due to future updates. 
> if anyone besides myself is to ever play this game and contribute a score, I will try to develop a full proof mechanism to have past blockchain entries 
> be compliant with future additions. I will save your score, and do my best to make sure it is preserved for future iterations, but I canot make that a 
> promise. 

## Top Scores & Cheaters 

<!-- TOP_SCORES_START -->
| Rank | Username           | Score | Timestamp |
|------|--------------------|-------|-----------|
| 1    | kleinpanic         | 2585  | 2025-03-26 16:02:08 |
<!-- TOP_SCORES_END -->

<!-- CHEATERS_START -->
No cheaters found.
<!-- CHEATERS_END -->

QuantumStriker is a high-performance 2D space shooter written in C using SDL2, SDL2_ttf, and SDL2_gfx. In this game, you pilot a futuristic spacecraft through a dynamically generated galactic environment. As your score increases, enemy spawn rates and difficulty scale up to challenge your reflexes and strategy. When you lose, your score is recorded in a blockchain-based high score system that validates your score using cryptographic proof-of-work and digital signatures. Invalid scores (or those that fail verification) are flagged as cheaters and will be listed in a dedicated cheaters section in the repository README.

## Features

- **Dynamic Galactic Environment:**  
  Explore a procedurally generated universe filled with stars, planets, and cosmic pickups.

- **Adaptive Difficulty:**  
  The enemy spawn rate and AI difficulty increase as your score grows.

- **Blockchain High Score System:**  
  - At game over, you are prompted in-game for your username.  
  - Your score is stored in a blockchain file (`highscore/blockchain.txt`) with a computed proof-of-work and digital signature.
  - A separate Python script (`verify_scores.py`) validates each block’s cryptographic integrity.
  - When updating the leaderboard, the verification script distinguishes valid scores from those that fail verification.  
  - Valid scores are displayed in the top scorers section; invalid (cheating) scores are recorded in a **Cheaters** section in the README.

- **Automated Leaderboard Update:**  
  A GitHub Actions workflow runs the `update_highscores.py` script every Monday at 00:00 UTC to update the README with the latest top scores and a cheaters list.

- **Command-Line Flags:**  
  - `--version`: Print the current version of QuantumStriker.  
  - `--help`: Display usage help.  
  - `--debug`: Activate debug mode (detailed logging output during development).  
  - `--fullscreen`: Launch the game in fullscreen mode (coming soon).  
  - `--highscores`: Display a table of all high scores in the terminal.

- **Responsive Controls:**  
  Use **WASD** for movement relative to the ship’s tip (W = forward, A = strafe left, S = backward, D = strafe right) and the arrow keys to rotate. Holding the Control key doubles your movement and rotation speed.

- **Modular Architecture:**  
  The code is split into distinct modules for players, enemies, bullets, background rendering, high score management, and versioning.

## Installation

### Dependencies

Ensure you have installed:
- **GCC** (or another C99-compliant compiler)
- **SDL2** – for graphics and input.
- **SDL2_ttf** – for text rendering.
- **SDL2_gfx** – for additional graphics primitives.
- **Make**

On Debian/Ubuntu, install with:

```bash
sudo apt-get install build-essential libsdl2-dev libsdl2-ttf-dev libsdl2-gfx-dev
```

### Building

Clone the repository and navigate to the project directory:

```bash
git clone https://github.com/yourusername/QuantumStriker.git
cd QuantumStriker
```

Build the project with:

```bash
make
```

For a debug build, run:

```bash
make debug
```

## Usage

Run the game with:

```bash
./QuantumStriker
```

### Command-Line Flags

- **`--version`**: Displays the current version.
- **`--help`**: Shows this help message.
- **`--debug`**: Activates debug mode (extra log output; for development).
- **`--fullscreen`**: Launches the game in fullscreen (coming soon).
- **`--highscores`**: Displays a table of all high scores in the terminal.

## Architecture

The repository is organized as follows:

```
QuantumStriker/
├── src/
│   ├── main.c               # Entry point; processes command-line flags and starts the game.
│   ├── game.c               # Main game loop and overall logic.
│   ├── player.c / player.h  # Player spaceship logic: movement, rendering, and collisions.
│   ├── enemy.c / enemy.h    # Enemy behavior and rendering.
│   ├── bullet.c / bullet.h  # Bullet management with a dynamic pool.
│   ├── background.c / background.h  # Procedural background rendering.
│   ├── score.c / score.h    # High score and username management.
│   ├── blockchain.c / blockchain.h  # Blockchain-based high score storage and PoW verification.
│   ├── signature.c / signature.h    # Digital signature generation and verification.
│   └── encryption.c / encryption.h  # Cryptographic functions (hashing, RSA key generation, etc.).
├── highscore/               # Directory for per-user high score files and blockchain storage.
├── scripts/
│   └── update_highscores.py  # Script to update README.md with top scores and cheaters.
├── .github/
│   └── workflows/
│       └── update-highscores.yml  # GitHub Action to auto-update high scores every Monday.
├── .gitignore
├── README.md
├── TODO.md
├── CONTRIBUTING.md
└── CHANGELOG.md
```

## High Score & Cheater System

- **Score Submission:**  
  When you lose, the game prompts you for your username. Your score is saved in a blockchain file (`highscore/blockchain.txt`) along with a proof-of-work and digital signature.

- **Score Verification:**  
  The `verify_scores.py` script (or its integration within `update_highscores.py`) verifies every block by:
  - Recomputing the hash of the block data.
  - Checking that the stored proof-of-work is valid and meets the required difficulty.
  - Ensuring the digital signature is present and meets minimal criteria.
  - Confirming proper chain linkage (for non-genesis blocks).

- **Leaderboard Update:**  
  The `update_highscores.py` script:
  - Runs through the blockchain, extracts the highest valid score for each user, and sorts them.
  - Updates the top scores section in the README.
  - **New:** Any block that fails verification is flagged as a cheater. The corresponding username and score are recorded under a **Cheaters** section in the README.

### README Sections Updated by the Scripts

- **Top Scores Section:**  
  Inserted between markers:
  ```markdown
  <!-- TOP_HIGHSCORES_START -->
  ... (table of top scores)
  <!-- TOP_HIGHSCORES_END -->
  ```

- **Cheaters Section:**  
  Inserted between markers:

```markdown
<!-- CHEATERS_START -->
No cheaters found.
<!-- CHEATERS_END -->
```

The GitHub Actions workflow (via `.github/workflows/update-highscores.yml`) runs the `update_highscores.py` script periodically to update these sections.

## How It Works

- **Game Loop & Difficulty:**  
  The main loop (in `game.c`) handles input, updates game state, and scales enemy spawns based on your score.

- **Player Controls:**  
  The ship moves relative to its tip's direction (WASD for movement, arrow keys for rotation). This ensures that forward always aligns with the ship's tip.

- **Bullet and Collision Management:**  
  Bullets are spawned from the ship’s tip and managed dynamically. Collision detection is used both for enemy destruction and player damage.

- **Blockchain Verification:**  
  When a score is submitted, a new block is added to the blockchain with a computed proof-of-work and signature. Later, the `verify_scores.py` script (called by `update_highscores.py`) verifies the integrity of each block.  
  - Valid blocks contribute to the leaderboard.
  - Blocks that fail validation are marked as cheaters and displayed separately.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for a complete history of changes.

## TODO

See [TODO.md](TODO.md) for planned improvements and future features.

