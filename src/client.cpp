#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "des.h"

// --- Cross-Platform Socket Headers ---
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> // untuk close()
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
#endif

// --- Fungsi Bantuan Jaringan ---

void init_sockets() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: WSAStartup failed" << std::endl;
        exit(1);
    }
#endif
}

void cleanup_sockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void print_error(const std::string& message) {
#ifdef _WIN32
    std::cerr << "Error: " << message << " - " << WSAGetLastError() << std::endl;
#else
    perror(("Error: " + message).c_str());
#endif
}

// --- Main Program ---

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>" << std::endl;
        return 1;
    }

    // 1. Baca Kunci
    std::string key;
    std::ifstream key_file("key.txt");
    if (!key_file) {
        std::cerr << "Error: Cannot open key.txt" << std::endl;
        return 1;
    }
    std::getline(key_file, key);
    if (key.length() != 8) {
        std::cerr << "Error: Key must be exactly 8 characters long." << std::endl;
        return 1;
    }

    init_sockets();

    // 2. Buat Socket
    socket_t client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        print_error("Socket creation failed");
        cleanup_sockets();
        return 1;
    }

    // 3. Connect ke Server
    const char* server_ip = argv[1];
    int port = std::stoi(argv[2]);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

#ifdef _WIN32
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
#else
    if (inet_addr(server_ip) == INADDR_NONE) {
        std::cerr << "Error: Invalid IP address" << std::endl;
        closesocket(client_socket);
        cleanup_sockets();
        return 1;
    }
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
#endif

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        print_error("Connect failed");
        closesocket(client_socket);
        cleanup_sockets();
        return 1;
    }

    std::cout << "Connected to server. You can start the chat now." << std::endl;
    std::cout << "Type 'exit' to end the session." << std::endl;

    // 4. Loop Komunikasi
    char buffer[4096];
    std::string message;
    while (true) {
        std::cout << "Client: ";
        std::getline(std::cin, message);
        try {
            std::string encrypted_message = des_encrypt(message, key);
            send(client_socket, encrypted_message.c_str(), encrypted_message.length(), 0);
            if (message == "exit") break;
        } catch (const std::exception& e) {
            std::cerr << "Encryption failed: " << e.what() << std::endl;
        }

        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            std::string encrypted_response(buffer, bytes_received);
            try {
                std::string decrypted_response = des_decrypt(encrypted_response, key);
                std::cout << "Server: " << decrypted_response << std::endl;
                if (decrypted_response == "exit") break;
            } catch (const std::exception& e) {
                std::cerr << "Decryption failed: " << e.what() << std::endl;
            }
        } else if (bytes_received == 0) {
            std::cout << "Server disconnected." << std::endl;
            break;
        } else {
            print_error("recv failed");
            break;
        }
    }

    // 5. Cleanup
    closesocket(client_socket);
    cleanup_sockets();
    std::cout << "Connection closed." << std::endl;

    return 0;
}