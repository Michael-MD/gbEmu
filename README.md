# Monochrome Gameboy Emulator in C++

Welcome to the Simple Monochrome GameBoy Emulator! This project is a straightforward C++ emulator aimed at simplicity and ease of understanding, making it ideal for learning about emulation concepts or experimenting with GameBoy development.

<!-- Top row -->
<p align="center">
  <img src="Title Screens/F-1 Race.png" width="250" alt="Image 1">
  <img src="Title Screens/Kirby's Dream Land.png" width="250" alt="Image 2">
  <img src="Title Screens/Pokemon Red.png" width="250" alt="Image 3">
</p>

<!-- Bottom row -->
<p align="center">
  <img src="Title Screens/Super Mario Land.png" width="250" alt="Image 4">
  <img src="Title Screens/Tetris.png" width="250" alt="Image 5">
</p>


## Features

- **Easy-to-Understand Code**: The emulator is designed with clarity in mind, with extensive comments throughout the codebase to explain its functionality.
- **Efficient**: Consumes very low memory through the use of things like bitfields and can run at very high FPS.
- **Controller Support**: Supports xbox and PS controller as well as keyboard, click [here](#controls) for mapping.

## Controls
| Action       | Keyboard  | PlayStation Controller | Xbox Controller |
|--------------|-----------|------------------------|-----------------|
| Move Right   | → | D-Pad Right | D-Pad Right |
| Move Left    | ← | D-Pad Left | D-Pad Left |
| Move Up      | ↑ | D-Pad Up | D-Pad Up |
| Move Down    | ↓ | D-Pad Down | D-Pad Down |
| Button A     | V | ╳ | A |
| Button B     | C | ◯ | B |
| SELECT       | Z | Select | Back |
| START        | X | Start | Start |


## Getting Started

### Running in Visual Studio

Ensure you have SDL2 installed on your system. Follow the instructions below based on your operating system:

##### Windows:

1. Download the SDL2 development libraries for Visual Studio from [here](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.0).
2. Extract the downloaded files and copy the contents to a directory on your system.
3. In Visual Studio, open your project and navigate to `Project Properties` > `Configuration Properties` > `VC++ Directories`.
4. Add the directory containing the SDL2 header files to the "Include Directories" field.
5. Add the directory containing the SDL2 `.lib` files to the "Library Directories" field.
6. Finally, add `SDL2.lib` and `SDL2main.lib` to the "Additional Dependencies" field under `Linker` > `Input`.

## Compatibility

This emulator was developed and tested primarily in Visual Studio on Windows. While it may require minor adjustments, it should be relatively easy to adapt for other operating systems. Feel free to experiment and contribute to make it more compatible across different platforms since no compiler/platform specific code is used.

## MBC Support
The emulator supports the following Memory Bank Controllers (MBCs):
  - [x] No MBC
  - [x] MBC1
  - [x] MBC2
  - [x] MBC3
  - [ ] MBC5
  - [ ] MBC6
  - [ ] MBC7
  - [ ] MMM01
  - [ ] M161
  - [ ] HuC1
  - [ ] HuC-3

Most of these are for other gameboy models anyway so they aren't relavent here anyway. 

## Contributing

Welcome! I'm thrilled that you're interested in contributing to our project. Collaboration is at the heart of what we do, and we value the unique perspectives and skills that each contributor brings to the table.

### How to Contribute

Contributing is easy! Here's how you can get started:

1. Fork the repository to your GitHub account.
2. Make your desired changes or additions.
3. Commit your changes and push them to your fork.
4. Submit a pull request with a clear description of your changes.

## License

This project is licensed under the GNU GENERAL PUBLIC LICENSE - see the [LICENSE](LICENSE.txt) file for details.

## Disclaimer

Please note that this project is created by myself and is not affiliated with or endorsed by Nintendo. All trademarks, registered trademarks, product names, and company names mentioned herein are the property of their respective owners.
