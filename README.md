# 🎯 Console Projectile Game

<div align="center">

![C++](https://img.shields.io/badge/Language-C%2B%2B17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-brightgreen?style=for-the-badge&logo=windows&logoColor=white)
![Physics](https://img.shields.io/badge/Engine-Custom%202D%20Physics-orange?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Completed-success?style=for-the-badge)

**A high-precision, trajectory-based 2D arcade game running entirely inside your terminal.**  
*Calculate velocity, adjust angles, select custom projectile mechanics, and destroy targets.*

[Key Features](#-key-features) • [Projectile Types](#-projectile-types) • [Physics Engine](#-physics--trajectory-engine) • [How to Run](#-quick-start--installation) • [Scoring](#-star--rating-system)

---

</div>

## 📌 Overview

**Console Projectile Game** is an engaging terminal-based projectile physics simulation and arcade puzzle game built in modern C++. The player calculates trajectories by adjusting initial velocity and launch angle on an ASCII/character grid ($62 	imes 17$).

With obstacles, destructible walls, multi-tier enemy configurations, and dynamic projectile variations, every shot requires tactical trajectory planning and fine physics estimation.

---

## ✨ Key Features

- 🎮 **10 Challenging Levels:** Carefully tuned obstacle geometry, target layouts, and par limits.
- 📐 **Discrete 2D Trajectory Simulation:** Dynamic vector trajectory rendering taking gravity, drag, and bounce into account.
- 💥 **Tactical Destructibility:** Environmental interactions including destructible barriers, target chain-reactions, and explosive impacts.
- ⭐ **Par-Based Progression:** 3-Star rating evaluation, high-score tracking, and persistence across sessions.
- 💾 **State & Progress Persistence:** Automatic save/load system for level progression, high scores, and stars (`.txt`/file storage).
- 🖥️ **Cross-Console Rendering:** Clean double-buffered ASCII screen drawing and frame timing.

---

## 🚀 Projectile Types

Select tactical munitions tailored for distinct obstacle configurations:

| Projectile | Icon | Description & Tactical Advantage |
| :--- | :---: | :--- |
| **Normal** | ⚪ | Standard ballistic trajectory governed by classic gravity equations. |
| **Bomb** | 💣 | Radial explosive shockwave upon impact; wipes clusters of enemies and walls. |
| **Pull / Vortex** | 🌀 | Gravitational singularity pulling nearby enemies into the impact zone. |
| **Bounce** | 🪃 | Reflects off rigid non-destructible barriers using angle of incidence physics. |
| **Drill / Piercing** | 🔩 | Pierces through outer barriers to neutralize protected interior targets. |
| **Split / Cluster** | 💥 | Splits into multiple cluster warheads mid-air to carpet-bomb wide sectors. |

---

## ⚙️ Physics & Trajectory Engine

The game engine numerically computes continuous 2D projectile motion:

$$x(t) = x_0 + v_0 \cos(	heta) \cdot t$$
$$y(t) = y_0 + v_0 \sin(	heta) \cdot t - rac{1}{2} g t^2$$

- **Gravity Constant ($g$):** $2.0\,	ext{m/s}^2$ default tuning
- **Simulation Time-step ($\Delta t$):** $0.10\,	ext{s}$ for continuous collision sweep
- **Dynamic Limits:** Angle range $[1^\circ, 90^\circ]$, Initial Speed range $[1, 25]$

---

## ⚡ Quick Start & Installation

### Prerequisites
- Modern C++ Compiler (`g++`, `clang++`, or `MSVC` with C++11/17 support)
- Command Line Terminal (Windows CMD/PowerShell, Linux Bash/Zsh, or macOS Terminal)

### Build & Run

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/your-username/console-projectile-game.git
   cd console-projectile-game
   ```

2. **Compile:**
   - **Linux / macOS:**
     ```bash
     g++ -O2 -std=c++17 console_projectile_game.cpp -o projectile_game
     ```
   - **Windows (MinGW):**
     ```powershell
     g++ -O2 -std=c++17 console_projectile_game.cpp -o projectile_game.exe
     ```

3. **Execute:**
   ```bash
   ./projectile_game
   ```

---

## 🕹️ Controls & Navigation

| Action | Input / Method |
| :--- | :--- |
| **Menu Navigation** | Numeric key selection (`1-4`, `Level Select`, `How to Play`) |
| **Aim Angle** | Enter integer $[1^\circ - 90^\circ]$ |
| **Launch Speed** | Enter scalar $[1 - 25]$ |
| **Select Munition** | In-game prompt before launching |

---

## ⭐ Star & Rating System

Levels are evaluated using a strict **Par Shot** benchmark:

$$	ext{Stars Awarded} = egin{cases} 
\star\star\star & 	ext{if } 	ext{shots} \le 	ext{Par} \ 
\star\star & 	ext{if } 	ext{Par} < 	ext{shots} \le 	ext{Par} + 2 \ 
\star & 	ext{if } 	ext{shots} > 	ext{Par} + 2 
\end{cases}$$

---

## 📂 Project Architecture

```
├── console_projectile_game.cpp  # Main game loop, physics engine, UI & levels
├── progress.txt                 # Auto-saved user progress & star ratings
├── .gitignore                   # Ignore builds, object files and binaries
└── README.md                    # Project documentation & specs
```

---

<div align="center">

*Developed with passion for terminal gaming and physics-based simulations.*

</div>
