# JAM Shell GUI (Wrapper for JAM Shell CLI)
This is a GUI wrapper that integrates the JAM Shell CLI with  features like a full-screen Qt GUI terminal for interacting with a JAM REPL, featuring command menus, styled output, tmux session management, and integrated terminal launching for advanced commands.

## Required Directory Structure
   
   ```bash
   JAM-Shell-Project/
│
├── GUI/                    
│   ├── jamshellwindow.cpp
│   ├── jamshellwindow.h
│   ├── main.cpp
│   ├── jamshell.pro
│   ├── jam                    # JAM SHELL Executable output after building
│   ├── jamshell               # JAM SHELL GUI Executable output after make
   ```
   
## Installing Dependencies
   **Install required dependencies for compiling and running the JAM Shell GUI:**
     
     1. If you’re on Ubuntu/Debian:
        ```bash
        sudo apt install tmux gnome-terminal cmake qtbase5-dev qttools5-dev qttools5-dev-tools libqt5svg5-dev qtchooser
        ```

     2. If you’re on Fedora/RHEL:
        ```bash
        sudo dnf install tmux gnome-terminal cmake qt6-qtbase-devel qt6-qttools-devel qt6-qtsvg-devel
        ```

## Configuration
   **Fix for no scrolling in TMUX terminal window (Needs to be done once):**
   1. Open TMUX configuration file from terminal using:
   ```bash
   nano ~/.tmux.conf
   ```
   2. Paste the below config , Save and exit using Ctrl+O → Enter → Ctrl+X:

   ```bash
   # Enable mouse support for scrolling, selecting panes, and resizing
   set -g mouse on

   # Use vi-style keybindings in copy mode (makes navigation like Vim)
   setw -g mode-keys vi

   # Enable mouse wheel scrolling in copy-mode
   bind-key -T copy-mode-vi WheelUpPane send-keys -X scroll-up
   bind-key -T copy-mode-vi WheelDownPane send-keys -X scroll-down
   ```
   3. Reload tmux config:
   tmux source-file ~/.tmux.conf

##  Build and Run the Program
1. **Generate the Makefile with qmake** 
   Open your terminal in the Shell directory and run:
   
   ```bash
   qmake
   ```

2. **Clean old build files (optional but good practice)**
   After successful compilation, run the integrated JAM shell:
  
   ```bash
   make clean
   ```

3. **Build the program**
  
   ```bash
   make
   ```

4. **Run the program**
   After successful compilation, run the integrated JAM Shell GUI:
  
   ```bash
   ./jamshell
   ```


