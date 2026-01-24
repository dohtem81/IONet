#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <ctime>
#include <chrono>
#include <vector>
#include <cerrno>
#include <netinet/in.h> // For sockaddr_in (IPv4)
#include <arpa/inet.h>  // For inet_pton
#include <sys/socket.h> // For socket functions
#include <unistd.h>     // For close, getpid
#include <fcntl.h>      // For fcntl
#include "../../include/ionet/schema/SchemaLoader.h"
#include "../../include/ionet/codec/Encoder.h"
#include "../../include/ionet/codec/Decoder.h"
#include "../../include/ionet/schema/Packet.h"
#include <thread>

/**
 * @brief Prints the usage instructions for the program.
 *
 * This function outputs the correct command-line usage format for the program,
 * including the required arguments: process ID, IPv4/DNS address, input port,
 * output port, and schema file.
 *
 * @param progname The name of the executable (typically argv[0]).
 */
void print_usage(const char* progname) {
    std::cout << "Usage: " << progname << " <processId> <IPv4/DNS> <inport> <outport> <schema_file>\n";
}

struct SocketPair {
    int send_fd;                /**< File descriptor for the send socket. */
    int recv_fd;                /**< File descriptor for the receive socket. */
    struct sockaddr_in out_addr; /**< Address structure for outgoing packets. */
};

/**
 * @brief Sets up UDP sockets for sending and receiving heartbeats.
 *
 * This function creates two UDP sockets: one for sending data to a specified
 * IP address and port, and one for receiving data on a specified port. The
 * receive socket is bound to the in_port and made non-blocking.
 *
 * @param in_port The port number to bind the receive socket to.
 * @param out_port The port number to send data to.
 * @param ip_or_dns The IPv4 address or DNS name to send data to.
 * @param process_id The process ID for logging purposes.
 * @return A SocketPair struct containing the send socket file descriptor,
 *         receive socket file descriptor, and the outgoing address.
 * @throws std::runtime_error If socket creation, binding, or IP resolution fails.
 */
SocketPair setup_udp_sockets(int in_port, int out_port, const std::string& ip_or_dns, int process_id) {
    // IPv4 only: resolve addresses
    struct sockaddr_in out_addr;
    std::memset(&out_addr, 0, sizeof(out_addr));
    out_addr.sin_family = AF_INET;
    out_addr.sin_port = htons(out_port);

    struct sockaddr_in in_addr;
    std::memset(&in_addr, 0, sizeof(in_addr));
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(in_port);
    in_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to any interface

    if (inet_pton(AF_INET, ip_or_dns.c_str(), &out_addr.sin_addr) <= 0) {
        std::cerr << "[Process " << process_id << "] Invalid IPv4 address: " << ip_or_dns << std::endl;
        throw std::runtime_error("Invalid IP address");
    }

    // Create UDP sockets
    int send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_sockfd < 0) {
        std::cerr << "[Process " << process_id << "] Failed to create send socket" << std::endl;
        throw std::runtime_error("Failed to create send socket");
    }

    int recv_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_sockfd < 0) {
        std::cerr << "[Process " << process_id << "] Failed to create receive socket" << std::endl;
        close(send_sockfd);
        throw std::runtime_error("Failed to create receive socket");
    }

    // Bind receive socket to in_port
    if (bind(recv_sockfd, (struct sockaddr*)&in_addr, sizeof(in_addr)) < 0) {
        std::cerr << "[Process " << process_id << "] Failed to bind receive socket to port " << in_port << std::endl;
        close(send_sockfd);
        close(recv_sockfd);
        throw std::runtime_error("Failed to bind receive socket");
    }

    // Make receive socket non-blocking
    int flags = fcntl(recv_sockfd, F_GETFL, 0);
    fcntl(recv_sockfd, F_SETFL, flags | O_NONBLOCK);

    std::cout << "[Process " << process_id << "] UDP sockets created: sending to " << ip_or_dns << ":" << out_port 
              << ", receiving on port " << in_port << std::endl;

    return {send_sockfd, recv_sockfd, out_addr};
}

struct Args {
    int process_id;             /**< The process ID (1001 or 1002). */
    std::string ip_or_dns;      /**< The IPv4 address or DNS name to send to. */
    int in_port;                /**< The port to bind for receiving. */
    int out_port;               /**< The port to send to. */
    std::string schema_path;    /**< Path to the schema file. */
};

/**
 * @brief Parses and validates command-line arguments.
 *
 * This function parses the command-line arguments, validates their values
 * (e.g., port ranges, file existence, process ID), and returns a struct
 * containing the parsed arguments. If validation fails, it prints error
 * messages and throws an exception.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return An Args struct containing the parsed process_id, ip_or_dns,
 *         in_port, out_port, and schema_path.
 * @throws std::runtime_error If argument count is wrong, values are invalid,
 *         or the schema file cannot be opened.
 */
Args parse_args(int argc, char* argv[]) {
    if (argc != 6) {
        print_usage(argv[0]);
        throw std::runtime_error("Invalid number of arguments");
    }

    int process_id = std::atoi(argv[1]);
    std::string ip_or_dns = argv[2];
    int in_port = std::atoi(argv[3]);
    int out_port = std::atoi(argv[4]);
    std::string schema_path = argv[5];

    if (in_port <= 0 || in_port > 65535) {
        std::cerr << "[Process " << process_id << "] Invalid in port number: " << argv[3] << std::endl;
        print_usage(argv[0]);
        throw std::runtime_error("Invalid in port");
    }

    if (out_port <= 0 || out_port > 65535) {
        std::cerr << "[Process " << process_id << "] Invalid out port number: " << argv[4] << std::endl;
        print_usage(argv[0]);
        throw std::runtime_error("Invalid out port");
    }

    std::ifstream schema_ifs(schema_path);
    if (!schema_ifs) {
        std::cerr << "[Process " << process_id << "] Schema file not found: " << schema_path << std::endl;
        throw std::runtime_error("Schema file not found");
    }

    // as of now processid can be only 1001 or 1002
    if (process_id != 1001 && process_id != 1002) {
        std::cerr << "[Process " << process_id << "] Invalid process ID. Must be 1001 or 1002." << std::endl;
        print_usage(argv[0]);
        throw std::runtime_error("Invalid process ID");
    }

    return {process_id, ip_or_dns, in_port, out_port, schema_path};
}


/**
 * @brief Main entry point for the 2heartbeats example application.
 *
 * This program demonstrates sending and receiving heartbeat packets using
 * the IONet library. It loads a schema, sets up UDP sockets, and sends
 * periodic heartbeats while listening for incoming packets. The process
 * ID determines which heartbeat packet type to use (1001 or 1002).
 *
 * Usage: program <processId> <IPv4/DNS> <inport> <outport> <schema_file>
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
    Args args;
    try {
        args = parse_args(argc, argv);
    } catch (const std::runtime_error& e) {
        return 1;
    }

    // Setup UDP sockets
    SocketPair sockets;
    try {
        sockets = setup_udp_sockets(args.in_port, args.out_port, args.ip_or_dns, args.process_id);
    } catch (const std::runtime_error& e) {
        return 1;
    }

    /*
     * 1. read schema from the file
     * 2. read packets
     * 3. set heartbeat
     * 4. serialize
     */
    auto schema_result = ionet::schema::SchemaLoader::fromFile(args.schema_path);
    if (!schema_result.ok()) {
        std::cerr << "[Process " << args.process_id << "] Failed to load schema: " << schema_result.error().message << std::endl;
        return 1;
    } else {
        std::cout << "[Process " << args.process_id << "] Schema loaded successfully from " << args.schema_path << std::endl;
    }

    std::unique_ptr<ionet::schema::Schema> schema_ = 
        std::make_unique<ionet::schema::Schema>(std::move(schema_result.value()));

    uint16_t message_id = ntohs(sockets.out_addr.sin_port); // Using port as message ID
    uint64_t latest_timestamp = static_cast<uint64_t>(time(nullptr)) * 1000; // Current time in milliseconds
    uint64_t start_timestamp = static_cast<uint64_t>(time(nullptr)) * 1000; // Current time in milliseconds

    // Create encoder
    ionet::codec::Encoder encoder(*schema_);
    ionet::codec::Decoder decoder(*schema_);

    // Create and send heartbeat
    if (auto packet_def = schema_->findPacketById(args.process_id)) {
        ionet::schema::Packet packet = *packet_def;

        // Send heartbeats every second for 10 seconds
        while (true) {
            latest_timestamp = static_cast<uint64_t>(time(nullptr)) * 1000;
            if (latest_timestamp - start_timestamp >= 500) {
                break;
            }
            // Set heartbeat fields 
            packet.set("timestamp", latest_timestamp);
            packet.set("process_id", static_cast<uint32_t>(getpid()));
            packet.set("status", static_cast<uint8_t>(0)); // OK status

            auto encode_result = encoder.encode(packet);
            if (encode_result.ok()) {
                const auto& buffer = encode_result.value();
                ssize_t sent = sendto(sockets.send_fd, buffer.data(), buffer.size(), 0, 
                                    (struct sockaddr*)&sockets.out_addr, sizeof(sockets.out_addr));
                if (sent < 0) {
                    std::cerr << "[Process " << args.process_id << "] Failed to send heartbeat" << std::endl;
                } else {
                    std::cout << "[Process " << args.process_id << "] Sent heartbeat (" << sent << " bytes)" << std::endl;
                }
            } else {
                std::cerr << "[Process " << args.process_id << "] Failed to encode heartbeat packet: " << encode_result.error().message << std::endl;
            }

            // Try to receive incoming packets
            std::vector<uint8_t> recv_buffer(1024);
            struct sockaddr_in sender_addr;
            socklen_t sender_len = sizeof(sender_addr);
            ssize_t received = recvfrom(sockets.recv_fd, recv_buffer.data(), recv_buffer.size(), 0,
                                       (struct sockaddr*)&sender_addr, &sender_len);
            if (received > 0) {
                std::cout << "[Process " << args.process_id << "] Received heartbeat (" << received << " bytes)" << std::endl;
                // Try to decode assuming it's a heartbeat packet with the same ID
                auto byteBufferReader = ionet::core::ByteBufferReader(recv_buffer.data(), received);
                auto decode_result = decoder.decode(args.process_id, byteBufferReader);
                if (decode_result.ok()) {
                    const auto& decoded_packet = decode_result.value();
                    auto timestamp = *decoded_packet.get<uint64_t>("timestamp");
                    auto process_id = *decoded_packet.get<uint64_t>("process_id");
                    auto status = *decoded_packet.get<uint64_t>("status");
                    std::cout << "[Process " << process_id << "] Received heartbeat: timestamp=" << timestamp
                              << ", process_id=" << process_id
                              << ", status=" << (int)status
                              << std::endl;
                } else {
                    std::cerr << "[Process " << args.process_id << "] Failed to decode received packet: " << decode_result.error().message << std::endl;
                }
            }

            //wait for 1 second before sending next heartbeat
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    } else {
        std::cerr << "[Process " << args.process_id << "] Packet definition not found for heartbeat (ID " << args.process_id << ")" << std::endl;
    }



    close(sockets.send_fd);
    close(sockets.recv_fd);
    std::cout << "[Process " << args.process_id << "] UDP sockets created and closed successfully." << std::endl;
    std::cout << "[Process " << args.process_id << "] Arguments parsed, schema loaded, and heartbeat logic finished." << std::endl;
    return 0;
}
