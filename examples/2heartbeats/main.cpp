#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <netinet/in.h> // For sockaddr_in (IPv4)
#include <arpa/inet.h>  // For inet_pton
#include <sys/socket.h> // For socket functions
#include <unistd.h>     // For close
#include "../../include/ionet/schema/SchemaLoader.h"
#include "../../include/ionet/codec/Encoder.h"
#include "../../include/ionet/schema/Packet.h"
#include <thread>

void print_usage(const char* progname) {
    std::cout << "Usage: " << progname << " <IPv4/DNS> <port> <schema_file>\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    std::string ip_or_dns = argv[1];
    int port = std::atoi(argv[2]);
    std::string schema_path = argv[3];

    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number: " << argv[2] << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    std::ifstream schema_ifs(schema_path);
    if (!schema_ifs) {
        std::cerr << "Schema file not found: " << schema_path << std::endl;
        return 1;
    }

    // Load heartbeat schema definition
    auto schema_result = ionet::schema::SchemaLoader::fromFile(schema_path);
    if (!schema_result.ok()) {
        std::cerr << "Failed to load schema: " << schema_result.error().message << std::endl;
        return 1;
    } else {
        std::cout << "Schema loaded successfully from " << schema_path << std::endl;

    }
    const auto& schema = schema_result.value();
    std::cout << "Loaded schema: " << schema.info().name << " v" << schema.info().version << std::endl;

    // IPv4 only: resolve address
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_or_dns.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid IPv4 address: " << ip_or_dns << std::endl;
        return 1;
    }

    // Heartbeat logic runs directly in main
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to server" << std::endl;
        close(sockfd);
        return 1;
    }

    uint16_t message_id = ntohs(server_addr.sin_port); // Using port as message ID
    uint64_t latest_timestamp = static_cast<uint64_t>(time(nullptr)) * 1000; // Current time in milliseconds
    // update timestamp in schemas id message_id
    ionet::codec::Encoder encoder(*schema);

    ionet::schema::Packet packet = schema->findPacketById(message_id) ? 
                    ionet::core::Packet(message_id, schema->findPacketById(message_id)->name) :
                    ionet::core::Packet(message_id, "UnknownPacket");

    if (packet.id == message_id) {
        std::cout << "Found packet definition: " << packet.name << std::endl;
        packet.set("timestamp", latest_timestamp);
        packet.set("process_id", static_cast<int16_t>(getpid()));
        packet.set("status", static_cast<uint8_t>(0)); // OK status
        auto result = encoder.encode(packet);
    } else {
        std::cout << "Packet definition not found for ID: " << message_id << std::endl;
    }

    close(sockfd);
    std::cout << "Socket created and closed successfully." << std::endl;
    std::cout << "Arguments parsed, schema loaded, and heartbeat logic finished." << std::endl;
    return 0;
}
