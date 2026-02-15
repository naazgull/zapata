# Installation

This guide covers building Zapata from source.

## Requirements

### Compiler

Zapata requires a C++20 compatible compiler:

- GCC 10 or later
- Clang 12 or later

### Build Tools

- CMake 3.18 or later
- Make or Ninja

### Required Dependencies

| Dependency | Purpose | Package (Debian/Ubuntu) | Package (Arch) |
|------------|---------|-------------------------|----------------|
| OpenSSL | SSL/TLS support | `libssl-dev` | `openssl` |

### Optional Dependencies

| Dependency | Purpose | Package (Debian/Ubuntu) | Package (Arch) |
|------------|---------|-------------------------|----------------|
| SQLite3 | SQLite database | `libsqlite3-dev` | `sqlite` |
| MySQL Client | MySQL database | `libmysqlclient-dev` | `mariadb-libs` |
| Lua 5.4 | Lua scripting | `liblua5.4-dev` | `lua` |
| libmagic | MIME type detection | `libmagic-dev` | `file` |
| libuuid | UUID generation | `uuid-dev` | `util-linux-libs` |
| systemd | Journal integration | `libsystemd-dev` | `systemd-libs` |

## Building from Source

### Clone the Repository

```bash
git clone https://github.com/naazgull/zapata.git
cd zapata
```

### Configure

```bash
mkdir build
cd build
cmake ..
```

### Build

```bash
make -j$(nproc)
```

### Install

```bash
sudo make install
```

By default, this installs to `/usr/local`. To change the prefix:

```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/zapata ..
```

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Debug` | Build type: `Debug`, `Release`, `RelWithDebInfo` |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | Installation directory |
| `WITH_ASAN` | `OFF` | Enable Address Sanitizer |
| `WITH_TSAN` | `OFF` | Enable Thread Sanitizer |
| `WITH_UBSAN` | `OFF` | Enable UndefinedBehavior Sanitizer |

### Example: Release Build

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Example: Debug Build with Sanitizers

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DWITH_ASAN=ON ..
make -j$(nproc)
```

## Verifying the Installation

After installation, verify the libraries are available:

```bash
# Check if headers are installed
ls /usr/local/include/zapata/

# Check if libraries are installed
ls /usr/local/lib/libzapata-*
```

## Using Zapata in Your Project

### CMake

Add to your `CMakeLists.txt`:

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZAPATA REQUIRED zapata-base zapata-parser-json zapata-engine-rest)

target_include_directories(myapp PRIVATE ${ZAPATA_INCLUDE_DIRS})
target_link_libraries(myapp ${ZAPATA_LIBRARIES})
```

### Manual Compilation

```bash
g++ -std=c++20 myapp.cpp -o myapp \
    -I/usr/local/include \
    -L/usr/local/lib \
    -lzapata-base -lzapata-parser-json -lzapata-engine-rest \
    -lssl -lcrypto -lpthread
```

## Installed Components

### Libraries

| Library | Description |
|---------|-------------|
| `libzapata-base` | Core utilities, exceptions, crypto |
| `libzapata-parser-json` | JSON parser |
| `libzapata-parser-uri` | URI parser |
| `libzapata-parser-http` | HTTP parser |
| `libzapata-events` | Event dispatcher |
| `libzapata-ontology` | Message semantics |
| `libzapata-io-stream` | Stream abstractions |
| `libzapata-io-socket` | Socket I/O |
| `libzapata-net-transport` | Transport abstraction |
| `libzapata-net-http` | HTTP transport |
| `libzapata-net-tcp` | TCP transport |
| `libzapata-net-websocket` | WebSocket transport |
| `libzapata-storage-connector` | Database abstraction |
| `libzapata-storage-sqlite` | SQLite connector |
| `libzapata-storage-mysqlx` | MySQL connector |
| `libzapata-engine-startup` | Configuration |
| `libzapata-engine-rest` | REST engine |
| `libzapata-engine-transport` | Transport engine |

### Executables

| Executable | Description |
|------------|-------------|
| `zrestgen` | REST API code generator |

## Troubleshooting

### Missing OpenSSL

```
Could not find OpenSSL
```

Install OpenSSL development package:
```bash
# Debian/Ubuntu
sudo apt install libssl-dev

# Arch
sudo pacman -S openssl
```

### C++20 Not Supported

```
error: 'std::span' has not been declared
```

Upgrade your compiler or explicitly set:
```bash
cmake -DCMAKE_CXX_COMPILER=g++-12 ..
```

### Library Not Found at Runtime

```
error while loading shared libraries: libzapata-base.so
```

Add the library path:
```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

Or run `ldconfig` after installation:
```bash
sudo ldconfig
```

## Next Steps

- [Quickstart](quickstart.md) - Build your first REST API
- [Project Structure](project-structure.md) - How to organize your code
