# IA Gagnante de la promo L3 informatique Paris 8, 2025.

# IA Ludii Interface - TicTacToe Demo
*n@up8.edu 2025*

This repository demonstrates how to interface AI players with Ludii for the TicTacToe game:
- C++ implementation: `tic_tac_toc_player.cpp`
- Python implementation: `tic_tac_toc_player.py`

## Prerequisites
Assumes that `Ludii-1.3.11.jar` is in the current directory. If not, modify the `LUDII_JAR_FILE` variable in the bash scripts.

## Setup Instructions
You need to modify `TicTacTocPlayer.java` to play with either the C++ or Python player as described below.

### C++ Player
```bash
# Compile the player
sh ./make_bin.sh

# Configure TicTacTocPlayer.java
# - Set CPP_ACTIVE to true (line 34)
# - Set PYTHON_ACTIVE to false (line 36)

# Generate Ludii jar
sh ./make_jar.sh

# Run with Ludii UI
sh ./make_run.sh

# Run multiple games for stats
sh ./make_run_many_games.sh

# Clean up
sh ./make_clean.sh
```

### Python Player
```bash
# Check python shebang in tic_tac_toc_player.py
# Ensure execution permissions: chmod +x tic_tac_toc_player.py

# Configure TicTacTocPlayer.java
# - Set PYTHON_ACTIVE to true (line 36)
# - Set CPP_ACTIVE to false (line 34)

# Generate Ludii jar
sh ./make_jar.sh

# Run with Ludii UI
sh ./make_run.sh

# Run multiple games for stats
sh ./make_run_many_games.sh

# Clean up
sh ./make_clean.sh
```

### Remote Python Player

#### On Remote Machine
1. Place `tic_tac_toc_player.py` and `tic_tac_toc_remote_player.sh` on remote host
2. Verify shebangs and virtual environments
3. Ensure execution permissions: `chmod +x tic_tac_toc_remote_player.sh`
4. Test remote execution: `ssh mil-www ttt-2025/tic_tac_toc_remote_player.sh x.x...... x`

#### On Local Machine
1. Define `remote_player_str` in `TicTacTocRemotePlayer.java` (line 35)
2. Define `remote_host_str` in `TicTacTocRemotePlayer.java` (line 36)

```bash
# Generate Ludii jar
sh ./make_jar_remote.sh

# Run with Ludii UI
sh ./make_run.sh

# Run multiple games for stats
sh ./make_run_many_games.sh

# Clean up
sh ./make_clean.sh
```
