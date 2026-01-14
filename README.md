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
auto schema = ionet::schema::SchemaLoader::fromFile("telemetry.yaml");

// Decode incoming data
std::vector<uint8_t> rawData = receiveFromNetwork();
ionet::codec::Decoder decoder(schema);

auto packet = decoder.decode(0x01, rawData);
if (packet.ok()) {
    auto altitude = std::get<double>(packet.value()["altitude"]);
    auto engineOn = std::get<std::map<std::string, bool>>(packet.value()["engine_status"]);
}
```

## Building

Requires C++20 compatible compiler.

```bash
# Using Docker (recommended)
docker compose run --rm dev
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure

# Or directly on host
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

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

This project is a C++ implementation of an idea I first developed years ago in C# - a system where all TCP communication is defined in configuration files. The actual application code doesn't need to know anything about the underlying message format; it simply accesses ready-to-use structured data.

This design:
- Separates protocol concerns from business logic
- Enables protocol updates without recompilation  
- Provides a layer of protection against malformed messages
- Makes testing and debugging communication straightforward

## License

MIT
