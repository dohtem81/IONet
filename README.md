# IONet

A runtime-configurable telemetry decoder library for parsing binary data streams into structured data and vice versa.

## Overview

IONet allows you to define binary message formats in configuration files (YAML/JSON) rather than hardcoding them. The library parses incoming byte streams according to these schemas and produces structured, typed data - without recompilation.

This approach decouples protocol definitions from application logic, making it easier to:
- Update message formats without code changes
- Support multiple protocol versions simultaneously
- Protect against malformed or unexpected messages
- Test and validate communication independently

## Features

- **Schema-driven parsing** - Define packet structures in YAML/JSON configuration files
- **Zero-copy buffer operations** - Efficient parsing directly from byte buffers
- **Bit-level field support** - Handle telemetry with bit-packed fields common in aerospace
- **Endianness handling** - Automatic byte order conversion (big/little endian)
- **Scaled integers** - Convert raw sensor values to real units (e.g., ADC counts → temperature)
- **Runtime configuration** - Load and switch schemas without recompilation
- **Type-safe decoding** - Strongly typed field access with validation

## Use Case

Originally conceived for aerospace telemetry systems where:
- Bandwidth is limited (satellites, rockets)
- Binary protocols are mandatory
- Message formats evolve over time
- Multiple ground stations need consistent parsing

The same approach applies to any domain with structured binary protocols: IoT sensors, industrial control systems, network protocols, file format parsing.

## Example Schema

```yaml
schema:
  name: "RocketTelemetry"
  version: "1.0"
  byte_order: "big"

packets:
  - id: 0x01
    name: "FlightData"
    fields:
      - name: "timestamp"
        type: "uint64"
        unit: "microseconds"
      - name: "altitude"
        type: "float32"
        unit: "meters"
      - name: "temperature"
        type: "int16"
        scale: 0.01
        offset: -40.0
        unit: "celsius"
      - name: "engine_status"
        type: "bitfield"
        bits: 8
        flags:
          - { bit: 0, name: "engine_1_active" }
          - { bit: 1, name: "engine_2_active" }
          - { bit: 7, name: "abort_commanded" }
```

## Example Usage

```cpp
#include <ionet/schema/SchemaLoader.h>
#include <ionet/codec/Decoder.h>

// Load schema from file
auto schemaResult = ionet::schema::SchemaLoader::fromFile("telemetry.yaml");
if (!schemaResult.ok()) {
    // handle error
}
auto schema = schemaResult.value();

// Decode incoming data
std::vector<uint8_t> rawData = receiveFromNetwork();
ionet::codec::Decoder decoder(schema);

auto packetResult = decoder.decode(0x01, rawData);
if (packetResult.ok()) {
    const auto& packet = packetResult.value();
    auto altitude = packet.get<double>("altitude");
    auto engineStatus = packet.field("engine_status");
    // Use altitude and engineStatus as needed
}
```

## Prerequisites

- CMake 3.20+
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- Ninja build system (optional but recommended)
- Docker & Docker Compose (for containerized builds)

## Building

```bash
# Using Docker (recommended)
docker compose run --rm dev
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Or directly on host
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## Running Tests

```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
./build/tests/ionet_tests
```

See [tests/](tests/) for available test suites.

## Example Schemas

Check [schemas/example_telemetry.yaml](schemas/example_telemetry.yaml) for a complete schema example.

## Project Status

### Implemented
- Schema loading from YAML and JSON
- Core decoding functionality (Decoder, DecodedPacket)
- ByteBufferReader/Writer with endian support
- Bitfield and scaling support
- Type-safe field access
- Constraint validation
- Comprehensive unit tests

### In Progress
- Encoder (binary serialization from structured data)
- Advanced bitfield manipulation
- Schema validation improvements

### Planned
- Schema versioning and migration tools
- Performance optimizations
- Extended protocol features (custom field types, arrays)
- CLI tools for schema inspection and test generation
- More example schemas and usage guides

## Project Structure

```
IONet/
├── include/ionet/
│   ├── core/           # Buffer, endian, types
│   ├── schema/         # Schema representation
│   └── codec/          # Encoder/decoder
├── src/
├── tests/
├── schemas/            # Example schema files
└── docker/
```

## Background

This project is a C++ implementation of an idea I first developed years ago in C++ - a system where all TCP communication is defined in configuration files. The actual application code doesn't need to know anything about the underlying message format; it simply accesses ready-to-use structured data.

This design:
- Separates protocol concerns from business logic
- Enables protocol updates without recompilation  
- Provides a layer of protection against malformed messages
- Makes testing and debugging communication straightforward

## License

MIT
