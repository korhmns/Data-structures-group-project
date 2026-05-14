# 🃏 Blackjack Casino System

A console-based Blackjack game written in C++ for the CPE112: Programming with Data Structures final project (2/2025 Semester).

---

## 📋 Table of Contents

- [Features](#features)
- [Data Structures Used](#data-structures-used)
- [Requirements](#requirements)
- [Installation & Compilation](#installation--compilation)
- [How to Use](#how-to-use)
- [Gameplay Rules](#gameplay-rules)
- [Project Structure](#project-structure)

---

## ✨ Features

- **1–4 player** local multiplayer support
- **Bet system** with undo functionality (stack-based)
- **Double Down** option during player turns
- **Blackjack (3:2 payout)** auto-detection
- **Persistent leaderboard** with win/loss/draw tracking across rounds
- **Player search** by name via hash table lookup
- **Chip status** display at any time
- **Automatic deck reshuffle** each round using Fisher-Yates algorithm

---

## 🏗 Data Structures Used

| Structure | Used For | Why |
|-----------|----------|-----|
| **Stack** | Card deck (deal from top) + Bet undo history | O(1) deal and undo operations |
| **Linked List** | Each player's hand of cards | Dynamic size — unknown number of cards at runtime |
| **Queue** | Player turn order | FIFO — turns follow seating order |
| **Hash Table** | Leaderboard / player stats | O(1) average lookup by player name |

**Algorithms:**
- **Fisher-Yates Shuffle** — O(n) deck randomization
- **Ace Resolution** — Greedy: Ace = 11, reduced to 1 if hand busts
- **Insertion Sort** — Leaderboard sorted descending by chip count
- **Polynomial Rolling Hash** — with linear probing for collision resolution

---

## 💻 Requirements

- A C++ compiler with C++11 support or later (`g++` recommended)
- Terminal / Command Prompt

---

## 🚀 Installation & Compilation

**1. Clone the repository**
```bash
git clone https://github.com/your-username/blackjack-casino.git
cd blackjack-casino
```

**2. Compile**
```bash
g++ -o blackjack blackjack.cpp
```

**3. Run**
```bash
./blackjack
```

> On Windows, run `blackjack.exe` after compiling with MinGW or use WSL.

---

## 🎮 How to Use

### Starting the Game

When the program launches, you will see the casino banner and be prompted to set up players.

```
How many players? (1-4): 2
Enter name for Player 1: Alice
Enter name for Player 2: Bob
```

Each player starts with **1,000 chips**.

---

### Main Menu

After setup, the main menu appears each round:

```
╔═══════════════════════════╗
║        MAIN MENU          ║
╠═══════════════════════════╣
║  [1] Play Round           ║
║  [2] View Leaderboard     ║
║  [3] Chip Status          ║
║  [4] Search Player        ║
║  [5] Quit                 ║
╚═══════════════════════════╝
```

| Option | Description |
|--------|-------------|
| `1` | Start a new round (betting → deal → player turns → dealer → results) |
| `2` | View the leaderboard sorted by chip count |
| `3` | Show each player's current chip count |
| `4` | Search and display stats for a specific player |
| `5` | Quit the game |

---

### Playing a Round

#### Step 1 — Place Bets
Each active player enters a bet between 1 and their current chip count.

```
Alice (Chips: 1000) — Bet amount: 200
✔ Alice bets 200 chips.

Undo a bet? (y/n): n
```

You can type `y` to undo the **last** bet placed (stack-based undo).

#### Step 2 — View Your Hand
Cards are displayed as `RANK-SUIT_INITIAL`. The dealer's first card is hidden.

```
Dealer: [??] K-H   
Alice:  7-S A-H    (Total: 18)
```

#### Step 3 — Choose an Action

```
[1] Hit   [2] Stand   [3] Double Down
Choice:
```

| Action | Description |
|--------|-------------|
| **Hit** | Draw one more card. Bust if total exceeds 21. |
| **Stand** | Keep your current hand and end your turn. |
| **Double Down** | Double your bet, draw exactly one card, then automatically stand. |

#### Step 4 — Dealer's Turn
The dealer reveals their hidden card and hits until reaching **17 or higher**.

#### Step 5 — Results
Hands are compared and chips are paid out automatically.

```
Alice (19)  vs  Dealer (17): ✅ WIN  +200 chips
Bob   (14)  vs  Dealer (17): ❌ LOSE  -150 chips
```

---

### Payout Table

| Outcome | Payout |
|---------|--------|
| Win | Bet × 2 (net +1×) |
| Blackjack (21 with 2 cards) | Bet × 2.5 (net +1.5×) |
| Draw (Push) | Bet returned |
| Loss / Bust | Bet lost |

---

### Leaderboard

Displays all players sorted by chip count:

```
╔══════════════════════════════════════════╗
║            🏆  LEADERBOARD               ║
╠══════════════════════════════════════════╣
║  #   Name            Chips   W    L    D ║
╠══════════════════════════════════════════╣
║  1   Alice           1450    3    1    0 ║
║  2   Bob              800    1    3    0 ║
╚══════════════════════════════════════════╝
```

---

### Elimination

A player is **eliminated** when their chip count reaches **0**. The game ends when all players are eliminated.

---

## 📁 Project Structure

```
blackjack-casino/
└── blackjack.cpp       # Full source code (single-file project)
```

---

## 👨‍💻 Author

CPE112 — Programming with Data Structures  
Semester 2/2025
