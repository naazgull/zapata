# Base Utilities API Reference

The `zapata-base` module provides fundamental utilities used throughout the framework.

**Header:** `#include <zapata/base.h>`

## Exceptions

All exceptions derive from `zpt::exception`, which extends `std::exception`.

### Exception Hierarchy

| Exception | Header | Description |
|-----------|--------|-------------|
| `zpt::exception` | `Exception.h` | Base exception class |
| `zpt::ExpectationException` | `ExpectationException.h` | Failed `expect()` assertion |
| `zpt::SyntaxErrorException` | `SyntaxErrorException.h` | Parser syntax error |
| `zpt::NoMoreElementsException` | `NoMoreElementsException.h` | Empty collection access |
| `zpt::ClosedException` | `ClosedException.h` | Operation on closed resource |
| `zpt::CastException` | `CastException.h` | Type conversion failure |
| `zpt::InterruptedException` | `InterruptedException.h` | Interrupted operation |
| `zpt::ParserEOF` | `ParserEOF.h` | Unexpected end-of-file |
| `zpt::NoAttributeNameException` | `NoAttributeNameException.h` | Missing attribute name |

### Example

```cpp
#include <zapata/exceptions/exceptions.h>

try {
    // ... code that may throw
} catch (zpt::ExpectationException const& e) {
    std::cerr << "Assertion failed: " << e.what() << std::endl;
    std::cerr << "Condition: " << e.description() << std::endl;
} catch (zpt::exception const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

## Assertion Macros

### expect(condition, message)

**Header:** `#include <zapata/base/expect.h>`

Runtime assertion that throws `zpt::ExpectationException` on failure.

```cpp
expect(ptr != nullptr, "Pointer must not be null");
expect(count > 0, "Count must be positive, got: " << count);
```

## RAII Utilities

### zpt::sentry

**Header:** `#include <zapata/base/sentry.h>`

Scope guard that executes a callback on destruction.

```cpp
auto* resource = acquire_resource();
zpt::sentry cleanup([resource]() { release_resource(resource); });
// resource is released when scope exits
```

## Logging

**Header:** `#include <zapata/log/log.h>`

### Log Levels

| Level | Value | Description |
|-------|-------|-------------|
| `zpt::emergency` | 0 | System unusable |
| `zpt::alert` | 1 | Immediate action required |
| `zpt::critical` | 2 | Critical conditions |
| `zpt::error` | 3 | Error conditions |
| `zpt::warning` | 4 | Warning conditions |
| `zpt::notice` | 5 | Normal but significant |
| `zpt::info` | 6 | Informational |
| `zpt::debug` | 7 | Debug messages |
| `zpt::trace` | 8 | Fine-grained tracing |
| `zpt::verbose` | 9 | Most verbose |

### Macros

```cpp
zlog("Server starting on port " << port, zpt::info);
zdbg("Debug: value = " << value);    // zpt::debug level
ztrace("Entering function");          // zpt::trace level
zverbose("Detailed info");            // zpt::verbose level
```

### Configuration

```cpp
zpt::log_lvl = zpt::debug;           // Set log level threshold
zpt::log_fd = &std::cerr;            // Set output stream
```

## Cryptographic Hashes

**Header:** `#include <zapata/crypto/sha1.h>`, `sha256.h`, `sha512.h`

### Quick Functions

```cpp
std::string hash1 = zpt::crypto::sha1("data");
std::string hash256 = zpt::crypto::sha256("data");
std::string hash512 = zpt::crypto::sha512("data");
```

### Incremental Hashing

```cpp
zpt::crypto::SHA256 hasher;
hasher.init();
hasher.update(reinterpret_cast<const unsigned char*>(data1), len1);
hasher.update(reinterpret_cast<const unsigned char*>(data2), len2);
unsigned char digest[zpt::crypto::SHA256::DIGEST_SIZE];
hasher.finalize(digest);
```

## Text Encoding

**Header:** `#include <zapata/text/convert.h>`

### Base64

```cpp
// In-place encoding
std::string data = "Hello";
zpt::base64::encode(data);  // data = "SGVsbG8="

// Returning functions
std::string encoded = zpt::base64::r_encode("Hello");
std::string decoded = zpt::base64::r_decode(encoded);

// URL-safe Base64
std::string url_safe = zpt::base64::r_url_encode("data");
```

### URL Encoding

```cpp
std::string url = "hello world";
zpt::url::encode(url);  // url = "hello%20world"

std::string encoded = zpt::url::r_encode("hello world");
std::string decoded = zpt::url::r_decode(encoded);
```

### UTF-8

```cpp
std::string text = "...";
zpt::utf8::encode(text);  // Ensure valid UTF-8
zpt::utf8::decode(text);  // Decode UTF-8 sequences
```

## String Manipulation

**Header:** `#include <zapata/text/manip.h>`

```cpp
std::string s = "  hello  ";
zpt::trim(s);      // s = "hello"
zpt::ltrim(s);     // Left trim
zpt::rtrim(s);     // Right trim

zpt::replace(s, "find", "replace");

// Returning versions
std::string trimmed = zpt::r_trim("  hello  ");
```

## Key Generation

**Header:** `#include <zapata/text/convert.h>`

```cpp
std::string key = zpt::generate::r_key(32);  // 32-char random key
std::string hash = zpt::generate::r_hash();  // Random hash
std::string uuid = zpt::generate::r_uuid();  // New UUID
```

## Input Validation

**Header:** `#include <zapata/text/convert.h>`

```cpp
bool valid_email = zpt::test::email("user@example.com");
bool valid_uuid = zpt::test::uuid("550e8400-e29b-41d4-a716-446655440000");
bool valid_uri = zpt::test::uri("https://example.com/path");
bool valid_phone = zpt::test::phone("+1-555-123-4567");
bool valid_ts = zpt::test::timestamp("2024-01-15T10:30:00Z");
bool matches = zpt::test::regex("hello123", "^[a-z]+[0-9]+$");
```

## Time Utilities

**Header:** `#include <zapata/text/convert.h>`

```cpp
// Current time in various formats
auto timestamp = zpt::now<std::string>();    // ISO 8601
auto millis = zpt::now<uint64_t>();          // Milliseconds since epoch
auto seconds = zpt::now<double>();           // Seconds since epoch

// Convert millis to timestamp
std::string ts = zpt::timestamp_to_str(1705315800000);
```

## UUID

**Header:** `#include <zapata/uuid.h>`

```cpp
zpt::uuid id;                    // Generate new UUID
std::cout << id << std::endl;    // "550e8400-e29b-41d4-a716-446655440000"

zpt::uuid parsed("550e8400-e29b-41d4-a716-446655440000");
std::string str = id.to_string();

if (id1 == id2) { /* ... */ }
```

## Concurrency Primitives

### zpt::padded_atomic

**Header:** `#include <zapata/atomics/padded_atomic.h>`

Cache-line aligned atomic to prevent false sharing.

```cpp
zpt::padded_atomic<size_t> counter{0};
counter = 42;
size_t val = counter;
counter->fetch_add(1);  // Access underlying std::atomic
```

### zpt::locks::spin_mutex

**Header:** `#include <zapata/locks/spin_mutex.h>`

Spin-lock based reader-writer mutex.

```cpp
zpt::locks::spin_mutex mutex;

// Exclusive lock
mutex.lock();
// ... modify data ...
mutex.unlock();

// Shared lock
mutex.lock_shared();
// ... read data ...
mutex.unlock_shared();
```

## Memory Pool

**Header:** `#include <zapata/allocator/allocator.h>`

```cpp
// Create a 1MB memory pool
zpt::mem::pool my_pool{1024 * 1024};

// Use with STL containers
zpt::allocator<int> alloc{my_pool};
std::vector<int, zpt::allocator<int>> vec{alloc};

// Global pool
auto& global = zpt::MEM_POOL(1024 * 1024);
```

## Network Utilities

**Header:** `#include <zapata/net/manip.h>`

```cpp
std::string ip = zpt::net::getip("eth0");  // Get IP of interface
std::string any = zpt::net::getip();       // First non-loopback IP
```

## File Utilities

**Header:** `#include <zapata/file/manip.h>`

```cpp
std::vector<std::string> files;
zpt::glob("/src", files, "*.cpp", 3);  // Find .cpp files, 3 levels deep
```

## Memory Usage

**Header:** `#include <zapata/mem/usage.h>`

```cpp
double vm, rss;
zpt::process_mem_usage(vm, rss);
std::cout << "Virtual: " << vm << " KB, RSS: " << rss << " KB\n";
```

## Email

**Header:** `#include <zapata/mail/manip.h>`

```cpp
zpt::sendmail("to@example.com", "from@example.com",
              "Subject", "Message body", "reply-to@example.com");
```

## See Also

- [Architecture Overview](../architecture/overview.md)
- [Installation Guide](../getting-started/installation.md)
