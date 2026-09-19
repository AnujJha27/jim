# Jim

A lightweight, native code editor built with C++ and Qt6. Fast, zero-trace, and purpose-built with deep, native functionalities for hackers, auditors, and Web3 security researchers above everyone else.

**Note:** This project was vibecoded — built through natural conversation and iteration with AI assistance.

## Screenshots

<p align="center">
  <img src="Screenshot 2026-03-14 205007.png" width="800" alt="Welcome Screen">
  <br>
  <em>Welcome Screen</em>
</p>

<p align="center">
  <img src="Screenshot 2026-03-14 205023.png" width="800" alt="File Editor and Tree">
  <br>
  <em>File Editor and Tree</em>
</p>

<p align="center">
  <img src="Screenshot 2026-03-14 205031.png" width="800" alt="Animation dock and file explorer">
  <br>
  <em>Animation dock and file explorer</em>
</p>

## Features

### Core Editing
- **Syntax Highlighting** — 11 languages with advanced rules: C/C++ (C++20 & Qt), Python, JavaScript/TypeScript (ES2024), HTML5, CSS3, Rust, Go, JSON, YAML, Markdown, Assembly
- **Language Auto-Detection** — Automatically applies correct highlighting based on file extension
- **Code Folding** — Collapse/expand functions, classes, and blocks with gutter indicators
- **Line Numbers** — Always visible with auto-sizing and current-line highlight
- **Auto-Indentation** — Context-aware smart indenting
- **Auto-Pairing** — Automatic bracket `()`, `[]`, `{}` and quote `""`, `''` pairing with smart closing
- **Multiple Tabs** — Work on several files simultaneously with modification tracking
- **Split View** — View two files side by side (`Ctrl+\`)
- **Word Wrap** — Toggle line wrapping on demand
- **Mini Map** — Code overview with viewport indicator (`Ctrl+M`)
- **Smart Home Key** — Jumps to first non-whitespace character, then start of line
- **Multi-cursor editing** — `Alt+Click` to place arbitrary cursors, `Ctrl+D` to select next occurrence (`v1.6.0`)
- **Auto-close HTML/XML tags** — Detects `</` and auto-completes the closing tag (`v1.6.0`)
- **Color preview** — Show a small color swatch next to hex color codes (`#FF5733`) inline (`v1.6.0`)
- **Bracket Pair Colorization** — `()`, `[]`, `{}` coloured in three rotating neon hues with depth-tracking, painted directly in `paintEvent` for zero lag (`v1.8`)
- **Vim Mode** — Full modal Normal/Insert editing: `hjkl/w/b/0/$`, `G/gg`, `dd/yy`, `i/a/A/I/o/O/s/C`, undo/redo, paste, half-page scroll. Status-bar pill shows current mode. (`Ctrl+Alt+V`, `v1.8`)
- **Edit Heatmap Strip** — 3 px gutter strip colour-coded blue→red by per-line edit frequency since file open (`v1.8`)
- **Indent-Based Folding** — fold Python, YAML, and Markdown blocks by indentation level alongside brace-based folding for C++/JS/etc. (`v1.8.1`)
- **Persistent Scratchpad** — always-available tab (`Ctrl+Alt+S`) for mid-session notes; auto-saved to disk (`v1.8.1`)
- **Focus Fade** — all lines outside the current block dim while editing; snaps back on cursor move. Pure `paintEvent` overlay, toggle in View menu (`v1.8.1`)
- **Inline Image Preview** — hover over any image path string literal to see a thumbnail tooltip; resolved relative to the current file (`v1.8.1`)
- **Sticky Scroll** — as you scroll, the definition line sticks to the top edge of the editor so you never forget what scope you are in (`v0.8.0`)
- **Smart Paste** — automatically adjust the indentation of the pasted block to perfectly match the surrounding scope (`v0.8.0`)
- **Kinetic Smooth Scrolling** — buttery-smooth physics-based kinetic scroll with easing (`v0.8.0`)
- **Auto-Save on Focus Lost** — automatically save the active file whenever the editor window loses focus (`v0.8.0`)
- **Invisible Character Rendering** — optionally render trailing whitespaces, mixed tabs, and zero-width characters as faint dots (`v0.8.0`)
- **In-Line Git Blame** — toggle to render faint text at the end of the line showing who last modified it (`v0.8.0`)
- **Visual URL Paste** — pasting a URL over a highlighted word in Markdown formats it automatically (`v0.8.0`)
- **Hex Color Picker** — `Ctrl+Click` an inline hex code (`#FF5733`) to spawn a color wheel and edit in real-time (`v0.8.0`)

### Navigation
- **File Tree** — Browse and open files from a sidebar (`Ctrl+B`)
- **Search Everywhere** — tap `Shift` twice quickly to summon one quick-open search for files, symbols, actions, and recent history (`v0.8.0`)
- **Breadcrumb Navigation** — Shows `folder > file > function` context below the tab bar
- **Go to Line** — Jump to any line instantly (`Ctrl+G`)
- **Find & Replace** — Inline bar with regex, case, whole-word, selection-only, match count, and replacement support
- **Command Palette** — The same quick-open UI filtered to actions (`Ctrl+Shift+P`, `v1.8.1`)
- **Recent Files** — Access recently opened files from the File menu
- **Mini Map Navigation** — Click anywhere to jump to that part of your code
- **Quick-Switch Header/Source** — `Alt+O` to instantly toggle between `file.cpp` and `file.h` (`v0.8.0`)
- **Locate Current File** — `Ctrl+Alt+L` to snap the File Explorer open and highlight the active file (`v0.8.0`)

### Developer Tools
- **Integrated Terminal** — Embedded shell panel with animated slide-in/out (`Ctrl+\``)
- **Hex Editor** — Built-in binary file viewer/editor: hex + ASCII display, keyboard navigation, full save support, modification tracking
- **Disassembler** — Native disassembly via `objdump`/`llvm-objdump` with a parsed function list; click any function to jump to it (`Ctrl+Alt+D`)
- **Binary Inspector** — Pure native ELF and PE parser: headers, sections, imports/symbols, MD5 hashing, and string extraction — no external tools required (`Ctrl+Shift+I`)
- **Markdown Preview** — Live split-panel rendered preview with full dark theme, tables, code blocks, images, and task lists (`Ctrl+Shift+M`)
- **Neural Code Graph** — Force-directed dependency graph of all `#include` relations in the workspace, physics-simulated with draggable nodes (`Ctrl+Shift+N`)
- **Ghost Replay Mode** — Replays every insertion and deletion made since the file was opened, letting you watch the session back event-by-event (`Ctrl+Shift+G`)
- **File Watcher** — Detects external file changes and prompts to reload
- **AI Autocomplete** — Configurable OpenAI-compatible completions shown as muted ghost text; press `Tab` to accept or `Esc` to dismiss (Plugins menu)
- **Zero-Trace Paranoia Mode** — Global toggle that suppresses all disk persistence (recent files, session stats, auto-saves, file watching) for sensitive security sessions. Shows a glowing `☣ PARANOIA` pill in the status bar (`Ctrl+Alt+P`, `v2.0`)
- **Laser Vuln Underlines** — Native background regex scanner flags hardcoded secrets, unsafe C/C++ functions, and Solidity anti-patterns with glowing red laser underlines and a cyber-HUD tooltip (`Ctrl+Alt+X`, `v2.0`)

### Narrative Engine ✦
Jim has a full built-in interactive fiction engine. Open any `.story`, `.tw`, or `.twee` file to activate it.

| Feature | Shortcut | What it does |
|---------|----------|--------------|
| Story Graph | `Ctrl+Alt+N` | Live force-directed passage graph — click nodes to jump to passages |
| Playtest | `Ctrl+Alt+P` | Interactive reader panel — navigate choices, track variables |
| Export | `Ctrl+Alt+E` | Export to HTML, JSON, Ink, or Markdown |

**Syntax:** Twine-compatible plain text format.
```
:: Start
You wake up in a dark room.

[[Go through the door -> Hallway]]
[[Stay and wait -> WaitScene]]

:: Hallway [ending]
You escape into the light. ✦ The End
```
Node colors in the graph: 🟢 start · 🔵 normal · 🔴 dead end · 🟡 ending · ⚫ unreachable

### Binary Analysis Suite
Jim has a native binary analysis workflow requiring no external tools:

| Tool | Trigger | What it does |
|------|---------|--------------|
| Hex Editor | Auto on binary open | View/edit raw bytes |
| Disassembler | `Ctrl+Alt+D` | Disassemble via objdump, parsed function list |
| Binary Inspector | `Ctrl+Shift+I` | ELF/PE headers, sections, imports, strings |

Right-click any file in the Explorer to access all three tools directly.

### Visuals & Effects
- **Animation Cycler** — Matrix, Particles, Waves, Pulse, Starfield, Rain, Snow, Fire, **DJ Mode** (`Ctrl+Shift+A` / `Ctrl+Shift+J`)
- **Themes** — Light, Dark, Monokai, and **Noir Edition** (high-contrast grayscale)
- **CRT Post-Processing** — Scanline overlay, radial vignette, chromatic aberration edge fringe, and phosphor bloom (toggle in View menu)
- **Data Waterfall Minimap** — Matrix-style digital rain animated in the minimap background with the code overview rendered on top
- **Typing Sounds** — Satisfying mechanical keyboard click sounds on each keystroke (Toggle in View menu)
- **Laser Editing Effects** — Red laser slash and dissolving spark particles animate on the deleted line when `Ctrl+Shift+K` is used
- **Animated Panels** — Smooth slide animations for terminal, fade for welcome screen, flash on tab open
- **Custom Title Bar** — Frameless window with minimize, maximize/restore, and close
- **Keystroke Heatmap Overlay** — Floating QWERTY keyboard rendered over the editor, each key tinted dark-grey→blue→red by typing frequency; click to dismiss (`Ctrl+Alt+K`, `v1.8`)
- **Time-of-Day Ambient Theme** — Background tint shifts automatically every 10 min: dawn amber → neutral daytime → dusk rose → midnight indigo (`v1.8`)

### Productivity
- **Line Manipulation** — Duplicate (`Ctrl+D`), Move Up/Down (`Alt+↑/↓`), Delete (`Ctrl+Shift+K`), Toggle Comment (`Ctrl+/`)
- **Deleted Code History** — Dockable panel that automatically captures deleted code blocks (≥3 lines cut or deleted); click "Paste Selected" to restore any entry (`Ctrl+Alt+G`)
- **Cybernetic HUD** — Status bar widget showing a rolling hex counter, live CPU/Memory sparklines, and real-time WPM counter (Linux: `/proc/stat` + `/proc/meminfo`)
- **Trailing Whitespace** — Automatically trimmed on save
- **Session Statistics** — Tracks keystrokes, lines written, files opened, active time, and peak WPM; view as a styled stats card via `Tools > Session Statistics` (`v1.8.1`)
- **TODO/FIXME Panel** — Dockable panel scanning all open editors for `TODO`, `FIXME`, `HACK`, `NOTE`, `BUG` tags; colour-coded by type, double-click to jump (`v1.8.1`)
- **Scratchpad Send-To** — right-click to instantly copy highlighted code and append it to `jim_scratchpad.txt` with a timestamp (`v0.8.0`)
- **Dim Inactive Panes** — in split view, slightly dim the pane that does not have cursor focus (`v0.8.0`)
- **Command Line Support** — Open files and folders from terminal
- **Comprehensive Shortcuts** — All common operations keyboard-accessible

---

## Installation

### Prerequisites

**Debian / Ubuntu / Kali:**
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev-tools qt6-multimedia-dev build-essential
```

**Arch Linux:**
```bash
sudo pacman -S qt6-base qt6-tools
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase-devel gcc-c++ make
```

**macOS:**
```bash
brew install qt@6
```

**Windows:**
No prerequisites beyond Qt6 — use the PowerShell build script below. The build script auto-locates Qt6 and MinGW.

> **Note:** The integrated terminal uses one persistent QProcess shell (your `$SHELL` on Unix, `cmd.exe` on Windows), so commands such as `cd` persist for the session. It is a lightweight command console, not a full PTY emulator.

---

## Build and Run

### Linux / macOS / WSL

```bash
git clone https://github.com/AnujJha88/jim.git
cd jim
make
sudo make install   # installs to /usr/local/bin
```

To uninstall:
```bash
sudo make uninstall
```

### Windows (Native)

```powershell
git clone https://github.com/AnujJha88/jim.git
cd jim
.\build.ps1
.\release\jim.exe
```

The `build.ps1` script automatically:
1. Locates Qt6 and MinGW under `C:\Qt\`
2. Runs `qmake` to generate the Makefile
3. Compiles with `mingw32-make`
4. Runs `windeployqt` to bundle all Qt DLLs alongside the executable

---

## Usage

```bash
jim                        # open editor (welcome screen)
jim myfile.cpp             # open a specific file
jim .                      # open current directory as project
jim /path/to/project       # open a specific folder
```

---

## Keyboard Shortcuts

### File
| Shortcut | Action |
|----------|--------|
| `Ctrl+N` | New file |
| `Ctrl+O` | Open file |
| `Ctrl+Shift+O` | Open folder |
| `Ctrl+S` | Save |
| `Ctrl+Shift+S` | Save As |
| `Ctrl+W` | Close tab |
| `Ctrl+Q` | Quit |

### Editing
| Shortcut | Action |
|----------|--------|
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `Ctrl+X` | Cut |
| `Ctrl+C` | Copy |
| `Ctrl+V` | Paste |
| `Ctrl+A` | Select all |
| `Ctrl+D` | Duplicate line |
| `Alt+↑` | Move line up |
| `Alt+↓` | Move line down |
| `Ctrl+Shift+K` | Delete line |
| `Ctrl+/` | Toggle line comment |
| `Home` | Smart home (first non-whitespace → start of line) |

### Search
| Shortcut | Action |
|----------|--------|
| `Ctrl+F` | Find (inline bar) |
| `F3` | Find next |
| `Shift+F3` | Find previous |
| `Ctrl+H` | Replace |
| `Ctrl+G` | Go to line |

### View
| Shortcut | Action |
|----------|--------|
| `Ctrl+B` | Toggle file explorer |
| `Ctrl+M` | Toggle mini map |
| `` Ctrl+` `` | Toggle terminal |
| `Ctrl+\` | Toggle split view |
| `Ctrl+T` | Cycle themes (Light, Dark, Monokai, Noir Edition) |
| `Ctrl++` | Increase font size |
| `Ctrl+-` | Decrease font size |
| `Ctrl+Shift+M` | Toggle Markdown preview |
| `Ctrl+Shift+Z` | **Zen Mode** (Hide all UI chrome) |
| `Ctrl+Shift+J` | **DJ Mode** (Audio Visualizer) |

### Tools
| Shortcut | Action |
|----------|--------|
| `Ctrl+Shift+P` | **Command Palette** |
| `Ctrl+Shift+H` | Open current file in Hex Editor |
| `Ctrl+Shift+D` | Disassemble file |
| `Ctrl+Shift+I` | Open Binary Inspector |
| `Ctrl+Shift+N` | Neural Code Graph |
| `Ctrl+Shift+G` | Ghost Replay Mode |
| `Ctrl+Alt+G` | Toggle Deleted Code History panel |
| `Ctrl+Alt+S` | Open Persistent Scratchpad |
| `Ctrl+Alt+V` | Toggle Vim Mode |
| `Ctrl+Alt+K` | Keystroke Heatmap Overlay |
| `Ctrl+Alt+P` | Toggle Paranoia Mode |
| `Ctrl+Alt+X` | Toggle Vuln Scanner |
| `Ctrl+Alt+N` | Story Graph |
| `Ctrl+Alt+E` | Export Story |

---

## Configuration

Settings are automatically persisted across sessions:

| Setting | Default |
|---------|---------|
| Font size | 11pt |
| Word wrap | Off |
| Recent files | Last 10 |
| Window geometry | Restored |

Config location: `~/.config/TextEditor/Settings.conf` (Linux/macOS) or the registry (Windows via QSettings).

---

## Recent Updates

### v0.9.0 — The Narrative Engine ✦ (Current)
Jim now speaks story. Open any `.story`, `.tw`, or `.twee` file and a whole new creative mode activates — syntax highlighting, a live force-directed passage graph, an interactive playtest panel, and one-click export to HTML, JSON, Ink, or Markdown. Write branching fiction, game dialogue, or interactive tutorials right inside your code editor, no Twine required.

- **Story Graph** (`Ctrl+Alt+N`) — force-directed node graph of every passage and choice link. Color-coded: green = start, cyan = normal, red = dead end, yellow = ending, grey = unreachable. Click any node to jump the editor cursor to that passage. Updates live as you type.
- **Playtest Panel** (`Ctrl+Alt+P`) — terminal-style interactive reader. Navigate choices by clicking, track your breadcrumb trail, and watch variable state update in real time — all without leaving the editor.
- **Export** (`Ctrl+Alt+E`) — one dialog, four formats: self-contained HTML (playable in any browser), structured JSON (drop into Unity/Godot/custom engine), Ink (`.ink` for Inkle Studio), or flat Markdown for review.
- **Syntax Highlighting** — passage headers (`:: Name`) in green, `[[choice links]]` in magenta, `$variables` in yellow, macros in orange, comments in grey.
- **Story ✦** language label in the status bar when a story file is active.
- Twine-compatible `.story`/`.tw`/`.twee` file format — existing Twine stories open directly.

### v0.8.0
- Added **Zero-Trace Paranoia Mode** (`Ctrl+Alt+P`) — a global toggle for sensitive auditing sessions. When active, it suppresses all disk writes (recent files, telemetry, ghost replay logs, file watcher, scratchpad auto-save) and shows a glowing red `☣ PARANOIA` pill in the status bar.
- Added **Laser Vuln Underlines** (`Ctrl+Alt+X`) — a native, background regex scanner that flags hardcoded secrets (AWS keys, tokens), dangerous C/C++ functions (`strcpy`, `system`), and Solidity anti-patterns (`tx.origin`). Vulnerabilities are highlighted with glowing red laser underlines and a styled cyber-HUD tooltip on hover.

### v0.7.1
- Added **Command Palette** (`Ctrl+Shift+P`) — fuzzy-searchable frameless popup listing every editor action; arrow keys to navigate, Enter to trigger, Escape to dismiss.
- Added **TODO/FIXME Panel** — dockable panel that scans all open editors for `TODO`, `FIXME`, `HACK`, `NOTE`, `BUG` tags; colour-coded by type, double-click to jump to file+line (`Tools > TODO/FIXME Panel`).
- Added **Indent-Based Folding** — fold Python, YAML, and Markdown blocks by indentation level, alongside existing brace-based folding for C++/JS/etc.
- Added **Persistent Scratchpad** — always-available tab (`Ctrl+Alt+S`) for mid-session notes; auto-saved to disk atomically so nothing is ever lost.
- Added **Focus Fade** — all lines outside the current block dim while editing; snaps back on cursor move. Pure `paintEvent` overlay. Toggle in View menu.
- Added **Inline Image Preview** — hover over any image path string literal (`.png`, `.jpg`, `.svg`, `.gif`, `.bmp`, `.webp`, `.ico`) to see a thumbnail tooltip resolved relative to the current file. Toggle in View menu.
- Added **Session Statistics** — tracks keystrokes, lines written, files opened, active coding time, and peak WPM for the session; displayed as a styled stats card (`Tools > Session Statistics`).

### v0.7.0
- Added **Bracket Pair Colorization** — `()`, `[]`, `{}` coloured in rotating neon hues (cyan/magenta/yellow) with depth-tracking, painted in `paintEvent` for zero lag.
- Added **Vim Mode** — Full modal Normal/Insert editing: `hjkl/w/b/0/$`, `G/gg`, `dd/yy`, `i/a/A/I/o/O/s/C`, undo/redo, paste, half-page scroll. Status-bar pill shows current mode. (`Ctrl+Alt+V`)
- Added **Keystroke Heatmap Overlay** — Floating QWERTY keyboard rendered over the editor, each key tinted dark-grey to red by typing frequency; click to dismiss. (`Ctrl+Alt+K`)
- Added **Time-of-Day Ambient Theme** — Background tint shifts automatically every 10 min: dawn amber → neutral daytime → dusk rose → midnight indigo.
- Added **Edit Heatmap Strip** — 3 px gutter strip colour-coded blue to red by per-line edit frequency since file open.
- Added **Live WPM Counter** — Real-time words-per-minute in the Cybernetic HUD, calculated from a rolling 60-second keystroke window.
- **Major Refactor** — `texteditor.cpp` split into focused editor/app units including `codeeditor.cpp`, `animationwidget.cpp`, dedicated app widget sources, and `overlays.cpp`.
- **Bug fix** — DJ Mode dock no longer pushes the window off-screen; `maximumHeight` is clamped to available screen height before the dock attaches.

### v0.6.0
- Added **CRT Post-Processing** — scanline overlay, vignette, chromatic aberration fringe, and phosphor bloom; toggle in View menu.
- Added **Deleted Code History** — dockable panel that auto-captures deleted blocks of ≥3 lines with timestamps; double-click or "Paste Selected" to restore (`Ctrl+Alt+G`).
- Added **Data Waterfall Minimap** — Matrix-style animated digital rain rendered as the minimap background with the code overview overlaid on top.
- Added **Cybernetic HUD** — status bar telemetry widget: rolling hex counter + live CPU and Memory sparklines.
- Added **Laser Editing Effects** — `Ctrl+Shift+K` now spawns a red laser slash and dissolving spark particles on the deleted line.
- Added **Neural Code Graph** — force-directed `#include` dependency graph for the open workspace, with physics simulation and draggable nodes (`Ctrl+Shift+N`).
- Added **Ghost Replay Mode** — records every insertion and deletion from the moment a file is opened; replay plays the session back event-by-event (`Ctrl+Shift+G`).

### v0.5.0
- Added **Multi-cursor Editing** — `Alt+Click` to place multiple cursors; `Ctrl+D` now selects the next occurrence of the current word for simultaneous editing.
- Added **Secret DJ Mode** — An edgy system-audio reactive visualizer built into the animation dock (`Ctrl+Shift+J`).
- Added **Zen Mode** — Pure distraction-free writing: hides the explorer, terminal, tabs, and status bar instantly (`Ctrl+Shift+Z`).
- Added **Typing Sounds** — satisfying, low-latency mechanical click sounds as you type (Toggle via View menu).
- Added **Noir Edition** — A lethal high-contrast grayscale theme with a single red accent for the cursor.
- Added **Auto-close Tags** — Smart detection of `</` triggers automatic closing of HTML/XML tags.
- Added **Inline Color Previews** — Real-time color swatches rendered next to hex codes (`#RRGGBB`).
- Added **Session Time Tracker** — Status bar shows total coding time, persisted across sessions.

### v0.4.5
- Added **Find All Matches** — simultaneous highlighting of all search matches with a golden accent for the current choice; updates in real-time as you navigate.
- Added **Enhanced Syntax Highlighting** — significantly expanded keyword sets for C++20, Python 3.12, JavaScript/TypeScript, Rust, and Go.
- Added **Breadcrumb Symbol Detection** — Real-time regex-based function and class detection in the breadcrumb bar.

### v0.4.0
- Added **Markdown Preview** — live split-panel rendered preview (`Ctrl+Shift+M`). Supports headers, tables, code blocks with language labels, images, task lists, blockquotes, strikethrough, highlights, and raw HTML passthrough. Updates with 400 ms debounce as you type. No external libraries.
- Added **Disassembler** — wraps `objdump`/`llvm-objdump`, displays assembly with full syntax highlighting, parsed function list with click-to-jump (`Ctrl+Alt+D`)
- Added **Binary Inspector** — pure native ELF and PE parser (`Ctrl+Shift+I`): headers, sections, imports, MD5 hash, extracted strings. Handles ELF32/64 LE/BE and PE32/PE32+
- Added **Tools menu** with all binary analysis actions
- Added **right-click context menu** on file tree: Open, Hex Editor, Disassemble, Binary Inspector, Reveal in Explorer
- Added **animated terminal** — smooth slide up/down with easing curve (replaces instant show/hide)
- Added **welcome screen fade** — opacity animation on show/hide
- Added **tab flash** — accent colour flash when a new tab opens

### v0.3.0
- Added **AI Autocomplete** — configurable via any OpenAI-compatible API (Plugins menu)
- Added **inline Find bar** — shows match count, previous/next navigation, live highlight
- Added **Animation Cycler** expanded — Starfield, Rain, Snow, Fire added to Matrix/Particles/Waves/Pulse

### v0.2.2
- Added **Animation Cycler** (`Ctrl+Shift+A`)
- Improved **Hex Editor** — full save support, modification tracking (asterisk in tab)
- Enhanced **build system** — unified `GNUmakefile`, improved `build.ps1`
- Added **Smart Home Key**

### v0.2.1
- Added **Hex Editor** — automatic binary detection, integrated hex/ASCII viewer

### v0.2.0
- Added **Line Editing Power**: Duplicate, Move Up/Down, Delete, Toggle Comment
- Added **Trailing Whitespace** removal on save
- Added **custom frameless title bar**

### v0.1.0
- Welcome screen, Breadcrumb navigation, Integrated terminal
- Code folding, 11-language syntax highlighting, Language auto-detection
- File watcher, Monokai theme, modernised VS Code-inspired UI

### v0.0.0
- Multi-tab editor, syntax highlighting, auto-pairing, minimap, split view

---

## Contributing

Contributions welcome. See [ROADMAP.md](ROADMAP.md) for planned features. Keep PRs focused and match the existing code style.

## License

MIT License — see [LICENSE](LICENSE) for details.
