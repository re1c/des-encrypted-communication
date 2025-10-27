#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "des.h"

#pragma comment(lib, "ws2_32.lib")

void print_error(const std::string& message) {
    std::cerr << "Error: " << message << " - " << WSAGetLastError() << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>" << std::endl;
        return 1;
    }

    // 1. Baca Kunci dari file
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

    // 2. Inisialisasi Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        print_error("WSAStartup failed");
        return 1;
    }

    // 3. Buat Socket
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == INVALID_SOCKET) {
        print_error("Socket creation failed");
        WSACleanup();
        return 1;
    }

    // 4. Connect ke Server
    const char* server_ip = argv[1];
    int port = std::stoi(argv[2]);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (connect(client_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        print_error("Connect failed");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server. You can start the chat now." << std::endl;
    std::cout << "Type 'exit' to end the session." << std::endl;

    // 5. Loop Komunikasi
    char buffer[4096];
    std::string message;
    while (true) {
        // Mengirim pesan ke server
        std::cout << "Client: ";
        std::getline(std::cin, message);
        try {
            std::string encrypted_message = des_encrypt(message, key);
            int bytes_sent = send(client_socket, encrypted_message.c_str(), encrypted_message.length(), 0);
            if (bytes_sent == SOCKET_ERROR) {
                print_error("send failed");
                break;
            }
            if (message == "exit") {
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Encryption failed: " << e.what() << std::endl;
        }

        // Menerima balasan dari server
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            std::string encrypted_response(buffer, bytes_received);
            try {
                std::string decrypted_response = des_decrypt(encrypted_response, key);
                std::cout << "Server: " << decrypted_response << std::endl;
                if (decrypted_response == "exit") {
                    break;
                }
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

    // 6. Cleanup
    closesocket(client_socket);
    WSACleanup();
    std::cout << "Connection closed." << std::endl;

    return 0;
}
