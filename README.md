# QuantumStriker

<!-- HIGH_SCORE_BADGE_START -->
![1st Place](https://img.shields.io/badge/1st-testuser|120200120-gold)
![2nd Place](https://img.shields.io/badge/2nd-klein|278182-silver)
![3rd Place](https://img.shields.io/badge/3rd-default|5080-bronze)
<!-- HIGH_SCORE_BADGE_END -->

QuantumStriker is a high-performance 2D space shooter written in C using SDL2, SDL2_ttf, and SDL2_gfx. In this game, you pilot a futuristic spacecraft through a dynamically generated galactic environment. As your score increases, enemy spawn rates and difficulty scale up to challenge your reflexes and strategy. Your high score is recorded under your chosen username and becomes part of a community leaderboard that is automatically updated.

## Features

- **Dynamic Galactic Environment:**  
  Experience a procedurally generated universe with a rich background of stars, planets, and cosmic pickups.
  
- **Adaptive Difficulty:**  
  Enemy spawn rate and AI scale with your score, making the gameplay increasingly challenging.

- **High Score System:**  
  At game over, you are prompted directly within the game window for your username. Your high score is saved in `highscore/<username>_highscore.txt`.  
  The top three high scores (with 🥇, 🥈, and 🥉 icons) are automatically parsed and inserted into this README by a GitHub Actions workflow that runs every Monday at 00:00 UTC.

- **Command-Line Flags:**  
  - `--version`: Print the current version of QuantumStriker.  
  - `--help`: Display this help message.  
  - `--debug`: Activate debug mode (coming soon).  
  - `--fullscreen`: Launch the game in fullscreen mode (coming soon).  
  - `--highscores`: Display a table of all high scores in the terminal.

- **Responsive Controls:**  
  Use WASD for movement relative to the ship’s tip (W = forward, A = strafe left, S = backward, D = strafe right) and the arrow keys to rotate. Holding the Control key doubles your movement and rotation speed.

- **Modular Architecture:**  
  The code is organized into distinct modules for players, enemies, bullets, background rendering, high score management, and versioning.

## Installation

### Dependencies

Ensure you have the following installed:
- **GCC** (or another C99-compliant compiler)
- **SDL2**: For graphics and input.
- **SDL2_ttf**: For text rendering.
- **SDL2_gfx**: For additional graphics primitives.
- **Make**

On Debian/Ubuntu, install dependencies with:

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

For a debug build (object files are retained - this will be expanded in future additions), run:

```bash
make debug
```

## Usage

Run the game with:

```bash
./QuantumStriker
```

### Command-Line Flags

- **`--version`**: Displays the current version of QuantumStriker.
- **`--help`**: Shows this help message.
- **`--debug`**: Activates debug mode (additional debug info will be shown; coming soon).
- **`--fullscreen`**: Launches the game in fullscreen mode (coming soon).
- **`--highscores`**: Displays a table of all high scores in the terminal.

## Architecture

The repository is structured as follows:

```
QuantumStriker/
├── src/
│   ├── main.c           # Entry point; parses command-line flags and starts the game.
│   ├── game.c           # Main game loop and overall logic.
│   ├── player.c / .h    # Player spaceship logic, movement, rendering, and collisions.
│   ├── enemy.c / .h     # Enemy AI, behavior, and rendering.
│   ├── bullet.c / .h    # Bullet management and dynamic bullet pool.
│   ├── background.c / .h# Dynamic, procedural background rendering.
│   ├── score.c / .h     # High score and username management.
│   └── version.h        # Contains the current version number.
├── highscore/           # Directory for per-user high score files.
├── scripts/
│   └── update_highscores.py  # Script to update README.md with top scores.
├── .github/
│   └── workflows/
│       └── update-highscores.yml  # GitHub Action to auto-update high scores.
├── .gitignore
├── README.md
├── TODO.md
├── CONTRIBUTING.md
└── CHANGELOG.md
```

## High Score System

When you lose, the game prompts you in the window for your username. Your high score is saved in `highscore/<username>_highscore.txt`. The top three scores are automatically parsed and inserted into this README between the markers below. The GitHub Actions workflow runs every Monday at 00:00 UTC, so the leaderboard is kept current.

<!-- TOP_HIGHSCORES_START -->
| Rank | Username           | Score | Trophy |
|------|--------------------|-------|--------|
| 1    | testuser           | 120200120 | 🥇 |
| 2    | klein              | 278182 | 🥈 |
| 3    | default            | 5080  | 🥉 |
<!-- TOP_HIGHSCORES_END -->

## How It Works

- **Game Loop & Difficulty:**  
  The main game loop (in `game.c`) processes input, updates game objects, and adjusts enemy spawn rates based on your score. As you score more, enemies spawn more frequently and become tougher.

- **Player Controls:**  
  The ship moves relative to its tip's direction using WASD and rotates with the arrow keys. The movement functions ensure that forward (W) always moves in the direction the ship's tip is facing.

- **Bullet Management:**  
  Bullets are managed by a dynamic pool that allows infinite firing. They originate from the ship's tip and travel straight forward.

- **Background & Rendering:**  
  The background is generated with a grid, stars, planets, and health pickups for an immersive experience.

- **Versioning & Flags:**  
  Command-line flags (`--version`, `--help`, `--debug`, `--fullscreen`, `--highscores`) provide additional functionality and information.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for a complete history of changes.

## TODO

See [TODO.md](TODO.md) for planned improvements and future features.
