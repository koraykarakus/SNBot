# SN Bot

## Overview

SN Bot is a C++/CMake-based automation system designed to handle large-scale bot operations efficiently.

Currently, the project supports **Windows x86 and x64** builds. Linux support may be considered in the future but is not available at this time.

The bot system is capable of:

* Research automation
* Resource management
* Building upgrades
* Multiple play schedules (up to 10 configurable schedules)
* Running thousands of bots simultaneously

Unlike many PHP-based solutions, all calculations are performed outside of game scripts, preventing unnecessary load and blocking issues for real players.

The application supports secure connections to remote **MariaDB/MySQL** databases using SSL, while also allowing local database setups for development and testing.

Repository:

https://github.com/koraykarakus/SNBot

---

# Features

* C++20 / CMake project
* Windows x86 and x64 support
* MariaDB/MySQL integration
* SSL database connections
* Research automation
* Resource management
* Building upgrade automation
* Configurable bot schedules
* High scalability (thousands of bots)
* TOML configuration support
* Structured logging with spdlog

---

# Dependencies

This project uses two dependency management systems.

## Git Submodules

The following libraries are included as Git submodules:

* bcrypt
* tomlplusplus

Located inside the `deps` directory.

## vcpkg

The following libraries are installed through vcpkg:

* spdlog
* libmariadb

Required packages are defined in:

```text
vcpkg.json
```

---

# Project Structure

```text
SNBot/
│
├── deps/          Third-party libraries (Git submodules)
├── include/       Header files, structures, datasets and class definitions
├── language/      Language files (currently unfinished)
├── logs/          Log files
├── out/           Generated executable output
├── src/           Source code
│
├── .clang-format  Formatting rules
├── .gitignore     Git ignored files
├── .gitmodules    Git submodule definitions
└── vcpkg.json     vcpkg dependency manifest
```

---

# Getting Started

## Prerequisites

Install the following software:

* Git
* Visual Studio 2022
* Desktop Development with C++
* CMake support
* vcpkg

---

## Step 1 - Install Git

Download and install Git:

https://git-scm.com/download/win

---

## Step 2 - Clone the Repository

```bash
git clone --recursive https://github.com/koraykarakus/SNBot.git
```

The `--recursive` flag is required because the project uses Git submodules.

If you already cloned the repository without submodules:

```bash
git submodule update --init --recursive
```

---

## Step 3 - Install vcpkg

Clone vcpkg:

```bash
git clone https://github.com/microsoft/vcpkg.git
```

Bootstrap it:

```bash
cd vcpkg
bootstrap-vcpkg.bat
```

Integrate it with Visual Studio:

```bash
vcpkg integrate install
```

---

## Step 4 - Open the Project

Open **Visual Studio 2022** and select:

```text
File → Open → Folder
```

Select the cloned project directory.

Visual Studio should automatically detect the CMake project and install dependencies defined in `vcpkg.json`.

---

## Step 5 - Adjust Database Queries

Open:

```text
src/CDatabase.cpp
```

Modify SQL queries according to your project's database schema.

For example, if the original project uses:

```sql
rocket_launcher
```

and your database uses:

```sql
missile_launcher
```

simply update the corresponding query references.

Avoid adding new columns unless you also update the related source code.

---

## Step 6 - Build

Select your target platform:

* x64 for 64-bit Windows
* Win32 for 32-bit Windows

Make sure **SNBot** is selected as the startup target and build the project.

The executable will be generated in:

```text
out/
```

---

## Step 7 - Configure

Run `SNBot.exe` once.

On first launch, the application will generate:

```text
settings.toml
```

Open this file and configure your database connection settings.

---

## Step 8 - Tuning

Important configuration values:

```toml
default_bot_reload_time = 300
loop_time = 30
```

### default_bot_reload_time

Bots are reloaded from the database every 300 seconds.

### loop_time

Bot calculations are executed every 30 seconds.

Adjust these values according to your server requirements and hardware capacity.

---

## Step 9 - Run

Start the application.

Display available commands:

```text
/help
```

Start research and building automation:

```text
/start
```

---

# Notes

This project is provided as-is.

Feel free to modify, extend, optimize, or repurpose the code for your own projects.

Bug reports, suggestions, and contributions are welcome.

---

# Future Development

This README has been cleaned up and organized with AI assistance.

Future updates are not currently planned, but the repository will remain available for anyone who wishes to learn from it, improve it, or build upon it.
