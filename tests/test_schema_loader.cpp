#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <ionet/schema/SchemaLoader.h>

using namespace ionet::schema;
using namespace ionet::core;

const char* YAML_SCHEMA = R"(
schema:
  name: "TestSchema"
  version: "1.0"
  byte_order: "big"

packets:
  - id: 1
    name: "TestPacket"
    fields:
      - name: "value"
        type: "uint32"
      - name: "temp"
        type: "int16"
        scale: 0.01
        offset: -40.0
        unit: "celsius"
)";

const char* JSON_SCHEMA = R"({
  "schema": {
    "name": "TestSchema",
    "version": "1.0",
    "byte_order": "little"
  },
  "packets": [
    {
      "id": 2,
      "name": "JsonPacket",
      "fields": [
        { "name": "counter", "type": "uint16" },
        { "name": "flags", "type": "bitfield", "bits": 8, "flags": [
          { "bit": 0, "name": "active" }
        ]}
      ]
    }
  ]
})";

TEST_CASE("SchemaLoader - load from YAML string", "[schema_loader]") {
    auto result = SchemaLoader::fromYaml(YAML_SCHEMA);
    
    REQUIRE(result.ok());
    
    auto& schema = result.value();
    REQUIRE(schema.info().name == "TestSchema");
    REQUIRE(schema.info().version == "1.0");
    REQUIRE(schema.byteOrder() == ByteOrder::Big);
    REQUIRE(schema.packetCount() == 1);
    
    auto* packet = schema.findPacketById(1);
    REQUIRE(packet != nullptr);
    REQUIRE(packet->name == "TestPacket");
    REQUIRE(packet->fields.size() == 2);
    
    auto* tempField = packet->findField("temp");
    REQUIRE(tempField != nullptr);
    REQUIRE(tempField->scaling.has_value());
    REQUIRE_THAT(tempField->scaling->scale, Catch::Matchers::WithinAbs(0.01, 0.0001));
    REQUIRE_THAT(tempField->scaling->offset, Catch::Matchers::WithinAbs(-40.0, 0.0001));
    REQUIRE(tempField->unit == "celsius");
}

TEST_CASE("SchemaLoader - load from JSON string", "[schema_loader]") {
    auto result = SchemaLoader::fromJson(JSON_SCHEMA);
    
    REQUIRE(result.ok());
    
    auto& schema = result.value();
    REQUIRE(schema.info().name == "TestSchema");
    REQUIRE(schema.byteOrder() == ByteOrder::Little);
    
    auto* packet = schema.findPacketById(2);
    REQUIRE(packet != nullptr);
    REQUIRE(packet->name == "JsonPacket");
    
    auto* flags = packet->findField("flags");
    REQUIRE(flags != nullptr);
    REQUIRE(flags->bitCount == 8);
    REQUIRE(flags->bitFlags.size() == 1);
    REQUIRE(flags->bitFlags[0].name == "active");
}

TEST_CASE("SchemaLoader - auto-detect JSON format", "[schema_loader]") {
    auto result = SchemaLoader::fromString(JSON_SCHEMA);
    REQUIRE(result.ok());
    REQUIRE(result.value().byteOrder() == ByteOrder::Little);
}

TEST_CASE("SchemaLoader - auto-detect YAML format", "[schema_loader]") {
    auto result = SchemaLoader::fromString(YAML_SCHEMA);
    REQUIRE(result.ok());
    REQUIRE(result.value().byteOrder() == ByteOrder::Big);
}

TEST_CASE("SchemaLoader - missing required fields", "[schema_loader]") {
    const char* badSchema = R"(
packets:
  - id: 1
    name: "NoFields"
)";
    
    auto result = SchemaLoader::fromYaml(badSchema);
    REQUIRE(result.hasError());
}

TEST_CASE("SchemaLoader - invalid YAML syntax", "[schema_loader]") {
    const char* invalidYaml = R"(
packets:
  - id: [invalid
)";
    
    auto result = SchemaLoader::fromYaml(invalidYaml);
    REQUIRE(result.hasError());
}

TEST_CASE("SchemaLoader - invalid JSON syntax", "[schema_loader]") {
    const char* invalidJson = R"({ "packets": [ { invalid })";
    
    auto result = SchemaLoader::fromJson(invalidJson);
    REQUIRE(result.hasError());
}

TEST_CASE("SchemaLoader - unknown data type", "[schema_loader]") {
    const char* badType = R"(
packets:
  - id: 1
    name: "BadType"
    fields:
      - name: "field1"
        type: "unknown_type"
)";
    
    auto result = SchemaLoader::fromYaml(badType);
    REQUIRE(result.hasError());
}

TEST_CASE("SchemaLoader - bitfield with flags", "[schema_loader]") {
    const char* bitfieldSchema = R"(
packets:
  - id: 1
    name: "BitfieldTest"
    fields:
      - name: "status"
        type: "bitfield"
        bits: 8
        flags:
          - { bit: 0, name: "flag_a", description: "First flag" }
          - { bit: 1, name: "flag_b" }
          - { bit: 7, name: "flag_high" }
)";
    
    auto result = SchemaLoader::fromYaml(bitfieldSchema);
    REQUIRE(result.ok());
    
    auto* packet = result.value().findPacketById(1);
    REQUIRE(packet != nullptr);
    
    auto* status = packet->findField("status");
    REQUIRE(status != nullptr);
    REQUIRE(status->type == DataType::Bitfield);
    REQUIRE(status->bitCount == 8);
    REQUIRE(status->bitFlags.size() == 3);
    REQUIRE(status->bitFlags[0].bit == 0);
    REQUIRE(status->bitFlags[0].name == "flag_a");
    REQUIRE(status->bitFlags[0].description == "First flag");
}

TEST_CASE("SchemaLoader - field constraints", "[schema_loader]") {
    const char* constraintSchema = R"(
packets:
  - id: 1
    name: "ConstraintTest"
    fields:
      - name: "voltage"
        type: "uint16"
        min: 0.0
        max: 5.0
        scale: 0.001
        unit: "volts"
)";
    
    auto result = SchemaLoader::fromYaml(constraintSchema);
    REQUIRE(result.ok());
    
    auto* packet = result.value().findPacketById(1);
    auto* voltage = packet->findField("voltage");
    REQUIRE(voltage != nullptr);
    REQUIRE(voltage->constraints.min.has_value());
    REQUIRE(voltage->constraints.max.has_value());
    REQUIRE_THAT(*voltage->constraints.min, Catch::Matchers::WithinAbs(0.0, 0.0001));
    REQUIRE_THAT(*voltage->constraints.max, Catch::Matchers::WithinAbs(5.0, 0.0001));
}

TEST_CASE("StringSource - returns content", "[schema_source]") {
    StringSource source("test content", "test");
    
    auto result = source.read();
    REQUIRE(result.ok());
    REQUIRE(result.value() == "test content");
    REQUIRE(source.description() == "test");
}

TEST_CASE("FileSource - non-existent file", "[schema_source]") {
    FileSource source("/nonexistent/path/file.yaml");
    
    auto result = source.read();
    REQUIRE(result.hasError());
}