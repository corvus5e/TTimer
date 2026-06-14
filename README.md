# ⏱️ TTimer

> [!WARNING]
> **TTimer is currently under development.** Features may change, and you might encounter bugs. Contributions and feedback are welcome!

**TTimer** is a lightweight, terminal-based timer and time tracking application built in C. Designed for developers and terminal enthusiasts, it helps you track time spent on tasks with a clean, distraction-free interface.

![TTimer UI Placeholder](https://via.placeholder.com/800x400.png?text=TTimer+Terminal+Interface)

## ✨ Features

- **Real-time Tracking:** Accurate time measurement with a custom-built TUI abstraction (`HomeTUI`).
- **Data Persistence:** Automatically saves your sessions and settings to a local **SQLite** database (bundled with the project).
- **Visual Analytics:** Built-in graph view to visualize your time-tracking history.
- **Customizable:** Configure app behavior (e.g., auto-start, save on exit) via the settings menu.
- **Keyboard Driven:** Fully navigable via intuitive hotkeys.

## 🚀 Quick Start

### Prerequisites

**Supported OS:** Linux, MacOS.

Ensure you have the following installed:
- `clang` or `gcc`
- `ncurses` (with wide-character support)

*Note: SQLite3 is bundled with the source code, so no separate installation is required.*

### Installation

1. Clone the repository (including submodules):
   ```bash
   git clone --recursive https://github.com/corvus5e/ttimer.git
   cd ttimer
   ```

2. Build the project:
   ```bash
   make
   ```

3. Run TTimer:
   ```bash
   make run
   ```

## ⌨️ Keybindings

| Key | Action |
|-----|--------|
| `Space` | Start / Pause / Resume timer |
| `s` | Open **Settings** |
| `g` | Open **Graph / Stats** |
| `h` | Open **Help** |
| `ESC` | Return to Timer view |
| `q` | Quit and save session |

## 🛠️ Built With

- **C11** - The core language.
- **Ncurses** - For the terminal user interface.
- **SQLite3** - For robust data persistence.
- **HomeTUI** - A custom UI library developed specifically for this project.

---

*Made with ❤️ for the Terminal.*
