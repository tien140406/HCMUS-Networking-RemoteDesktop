# Remote Administration Tool

A powerful C++ client-server application for remote system administration with both email-based and manual command interfaces.

## 🚀 Features

### Core Functionality

- **Dual Control Modes**: Email-based automation and manual GUI control
- **Real-time Connection**: TCP socket communication between client and server
- **Media Support**: Image and video display with playback controls
- **Modern UI**: ImGui-based interface with custom styling and scaling

### Remote Commands

- **Process Management**: List, start, and stop system processes
- **Application Control**: Launch and terminate applications
- **System Operations**: Shutdown and restart remote machines
- **Monitoring Tools**: Keylogger with configurable duration
- **Media Capture**: Screenshot capture and webcam recording
- **File Transfer**: Download files from remote systems

### Technical Features

- **Cross-platform Build**: CMake-based build system with MSYS2 support
- **Robust Networking**: Socket-based communication with error handling
- **Media Processing**: OpenCV integration for video/image handling
- **Email Integration**: Automated command processing via email
- **Scalable UI**: Automatic DPI scaling for different screen resolutions

## 📋 Requirements

### System Requirements

- Windows 10/11 (64-bit)
- MSYS2 environment
- MinGW64 compiler toolchain
- CMake 3.16 or higher
- Ninja build system

### Dependencies

- **OpenCV 4.x**: Computer vision and media processing
- **ImGui 1.91.0**: Immediate mode GUI framework
- **GLFW 3.4**: Window management and OpenGL context
- **libcurl**: HTTP/email communication
- **OpenGL 3.3+**: Graphics rendering
- **Winsock2**: Network communication

## 🔧 Installation

### 1. Install MSYS2

Download and install MSYS2 from [https://www.msys2.org/](https://www.msys2.org/)

### 2. Install Dependencies

```bash
# Update package database
pacman -Syu

# Install development tools
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-ninja

# Install libraries
pacman -S mingw-w64-x86_64-opencv
pacman -S mingw-w64-x86_64-curl
pacman -S mingw-w64-x86_64-openssl
```

### 3. Clone and Build

```bash
git clone https://github.com/tien140406/HCMUS-Networking-RemoteDesktop

./build.bat

mkdir build && cd build
cmake .. -G "Ninja" -DOpenCV_DIR="C:/msys64/mingw64/lib/cmake/opencv4"
ninja
```

## 🎮 Usage

### Starting the Applications

#### Server

```bash
cd build
./Server.exe
```

The server will start listening on port 8888 by default.

#### Client

```bash
cd build
./Client.exe
```

The client GUI will launch in fullscreen mode.

### Connection Setup

1. Enter server IP address (default: 127.0.0.1)
2. Enter server port (default: 8888)
3. Click "Connect" to establish connection
4. Use ESC key to exit the application

### Operating Modes

#### Email Mode

- Automatically checks for email commands every 5 seconds
- Commands must be sent with subject line "REMOTE CONTROL"
- Supports all available remote operations
- Provides real-time status updates

#### Manual Mode

Interactive GUI with organized command categories:

**Process Management**

- List all running processes
- Start new processes by name
- Kill existing processes

**Application Management**

- List installed applications
- Launch applications
- Stop running applications

**System Commands**

- Shutdown remote system
- Restart remote system

**Monitoring & Files**

- Keylogger with configurable duration
- Webcam recording (start/stop)
- File download from remote paths
- Screenshot capture

## 📧 Email Commands

Send emails with subject "REMOTE CONTROL" followed by these commands:

| Feature                   | Command                                                           | Example                                                          |
| ------------------------- | ----------------------------------------------------------------- | ---------------------------------------------------------------- |
| List/Start/Stop Processes | `list_process`<br>`start_process [name]`<br>`stop_process [name]` | `list_process`<br>`start_process calc`<br>`stop_process notepad` |
| Start/Stop Applications   | `start_program [name]`<br>`stop_program [name]`                   | `start_program notepad`<br>`stop_program calc`                   |
| Screen Capture            | `get_screenshot`                                                  | `get_screenshot`                                                 |
| Keylogger                 | `keylogger`<br>`keylogger [seconds]`                              | `keylogger`<br>`keylogger 30`                                    |
| Webcam Recording          | `start_recording`<br>`stop_recording`                             | `start_recording`<br>`stop_recording`                            |
| File Transfer             | `send_file [full path]`                                           | `send_file C:\Documents\file.txt`                                |
| System Control            | `reset`<br>`shutdown`                                             | `reset`<br>`shutdown`                                            |

## 🏗️ Project Structure

```
├── .vscode/               # VS Code configuration
├── build/                 # Build output directory
├── Client/
│   ├── ClientHandle/      # Command processing and email handling
│   │   ├── checkCommand.cpp/.h     # Command validation and processing
│   │   ├── commandUtils.cpp/.h     # Utility functions for commands
│   │   └── sendEmail.cpp/.h        # Email communication handling
│   ├── ClientUI/          # User interface components
│   │   ├── AdminUI/       # Main administration UI
│   │   │   ├── MediaHandling.cpp   # Image/video display and processing
│   │   │   ├── RemoteAdminUI.cpp   # Main UI class implementation
│   │   │   ├── RemoteAdminUI.h     # UI class header
│   │   │   └── Renderer.cpp        # UI rendering methods
│   │   ├── CreateUI.cpp/.h         # UI initialization and main loop
│   │   └── Types.cpp/.h            # UI data types and custom styling
│   └── client.cpp         # Client application entry point
├── Core/
│   ├── lib.h              # Common headers and dependencies
│   └── socketFiles.cpp/.h # File transfer over socket implementation
├── Fonts/                 # UI font files
│   ├── SEGOEUI.TTF        # Regular Segoe UI font
│   ├── SEGOEUIB.TTF       # Bold Segoe UI font
│   ├── SEGOEUII.TTF       # Italic variants
│   ├── SEGOEUIL.TTF       # Light variants
│   ├── SEGOEUISL.TTF      # Semi-light variants
│   ├── SEGOEUIZ.TTF       # Additional font weights
│   ├── SEGUIBL.TTF        # Segoe UI Black variants
│   ├── SEGUIBLI.TTF
│   ├── SEGUILI.TTF
│   ├── SEGUISB.TTF
│   ├── SEGUISBI.TTF
│   └── SEGUISLI.TTF
├── Server/
│   ├── Application/       # Server-side command implementations
│   │   ├── executeCommand.cpp/.h    # Command execution logic
│   │   ├── keylogger.cpp/.h         # Keystroke monitoring
│   │   ├── listProgram.cpp/.h       # Application listing
│   │   ├── recording.cpp/.h         # Webcam recording functionality
│   │   ├── sendPicture.cpp/.h       # Image capture and transmission
│   │   ├── sendScreenshot.cpp/.h    # Screenshot capture
│   │   └── shutdownProgram.cpp/.h   # System shutdown/restart
│   ├── commandProcessor.cpp/.h      # Main command processing
│   ├── server.cpp         # Server application entry point
│   └── serverHandler.cpp/.h         # Client connection handling
├── thirdparty/            # Third-party dependencies
│   ├── cacert.pem         # SSL certificate bundle
│   └── stb_image.h        # Single-header image loading library
├── .gitignore             # Git ignore rules
├── build.bat              # MSYS2 build script
├── CMakeLists.txt         # CMake configuration
└── README.md              # Project documentation
```

## 🔒 Security Considerations

⚠️ **WARNING**: This tool provides powerful remote access capabilities. Use responsibly and only on systems you own or have explicit permission to access.

- Ensure secure network connections
- Implement proper authentication in production
- Monitor and log all remote access activities
- Follow local laws and regulations regarding remote access tools

## 🛠️ Development

### Building from Source

The project uses CMake with automatic dependency fetching for ImGui and GLFW. OpenCV and CURL must be installed separately.

### Code Organization

- **Core**: Shared utilities and socket-based file transfer implementation
- **Client**: Complete GUI application with command handling and UI components
  - **ClientHandle**: Command processing, validation, and email communication
  - **ClientUI**: ImGui-based interface with admin controls and media handling
- **Server**: Console server with modular command implementations
  - **Application**: Individual command modules (keylogger, screenshot, etc.)
- **Fonts**: Complete Segoe UI font family for consistent UI rendering
- **thirdparty**: External dependencies and certificates

### Adding New Commands

1. Add new `CommandState` enum value in `Client/ClientUI/Types.h`
2. Implement server-side logic in `Server/Application/` directory
3. Add command handling in `RemoteAdminUI::ExecuteCommand()`
4. Update UI controls in appropriate render method in `Renderer.cpp`
5. Add email command processing in `ClientHandle/checkCommand.cpp`

## 📖 API Reference

### Key Classes

#### `RemoteAdminUI`

Main UI class handling all client-side operations

- Connection management
- Command execution
- Media display and playback
- Email monitoring

#### `ResultMessage`

Message container with timestamp and color coding

- Automatic timestamp generation
- Color-coded message types (Success, Error, Warning, Info)

### File Formats Supported

- **Images**: PNG, JPG, JPEG, BMP, TGA
- **Videos**: MP4, AVI, MOV, MKV, FLV, WMV
- **Text Files**: Process lists, keylogger output, application lists

## 🐛 Troubleshooting

### Common Issues

**Build Failures**

- Ensure MSYS2 is properly installed and in PATH
- Verify all dependencies are installed via pacman
- Check that OpenCV CMake files are available

**Connection Issues**

- Verify server is running and listening on correct port
- Check firewall settings
- Ensure both client and server use same port number

**UI Scaling Problems**

- Font files must be present in `Fonts/` directory
- Requires Segoe UI font family (SEGOEUI.TTF, SEGOEUIB.TTF, etc.)
- Automatic DPI scaling based on screen resolution

**File Transfer Issues**

- Check file permissions and paths
- Ensure `received_files/` directory exists
- Verify network stability for large file transfers

## 📄 License

This project is for educational purposes. Please ensure compliance with local laws and regulations when using remote administration tools.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## 📞 Support

For issues and questions:

- Check the troubleshooting section
- Review code comments for implementation details
- Ensure all dependencies are properly installed

---

**Note**: This tool is designed for legitimate system administration purposes. Users are responsible for ensuring compliance with applicable laws and obtaining proper authorization before use.
