# FTP File Server

A simple FTP file server written in C++ that supports file uploads and downloads over TCP sockets.

## Features

- File upload from clients to the server
- File download from the server to clients
- Concurrent connection handling using `poll`
- Multithreaded design with a dedicated thread for accepting file transfers and another for graceful shutdown

## Requirements

- C++17 compatible compiler
- CMake 3.24 or higher
- POSIX-compatible operating system (Linux, macOS)

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

Run the server:

```bash
./FTP_File_Server
```

By default, the server listens on port **8080**. Type `exit` in the console to stop the server.

### Connecting

Clients connect via TCP and send a header line terminated by `\n`:

- **Upload** – send the header `UPLOAD\n`, then stream the file data. The server saves the file and closes the connection when the client finishes sending.
- **Download** – send a header containing the requested filename (e.g., `DOWNLOAD filename\n`). The server streams the file back to the client.

## Project Structure

| File | Description |
|------|-------------|
| `main.cpp` | Entry point – creates the server, binds the socket, and starts listener and shutdown threads |
| `FtpServer.h` | `FtpServer` class declaration |
| `FtpServer.cpp` | `FtpServer` class implementation (socket setup, file I/O, connection handling) |
| `CMakeLists.txt` | CMake build configuration |

## License

This project does not currently specify a license.
