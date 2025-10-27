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

std::string get_local_ip() {
    char host_name[256];
    if (gethostname(host_name, sizeof(host_name)) == SOCKET_ERROR) {
        return "<unknown>";
    }
    addrinfo hints = {0};
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* info = nullptr;
    if (getaddrinfo(host_name, NULL, &hints, &info) != 0) {
        return "<unknown>";
    }

    char ip_str[INET_ADDRSTRLEN];
    sockaddr_in* sa = (sockaddr_in*)info->ai_addr;
    inet_ntop(AF_INET, &sa->sin_addr, ip_str, sizeof(ip_str));
    
    freeaddrinfo(info);
    return std::string(ip_str);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
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
    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        print_error("Socket creation failed");
        WSACleanup();
        return 1;
    }

    // 4. Bind Socket
    int port = std::stoi(argv[1]);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen di semua interface

    if (bind(listen_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        print_error("Bind failed");
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    // 5. Listen
    if (listen(listen_socket, 1) == SOCKET_ERROR) {
        print_error("Listen failed");
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on " << get_local_ip() << ":" << port << std::endl;
    std::cout << "Waiting for a client to connect..." << std::endl;

    // 6. Accept Connection
    SOCKET client_socket = accept(listen_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET) {
        print_error("Accept failed");
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    closesocket(listen_socket); // Kita tidak butuh listen socket lagi
    std::cout << "Client connected. You can start the chat now." << std::endl;
    std::cout << "Type 'exit' to end the session." << std::endl;

    // 7. Loop Komunikasi
    char buffer[4096];
    std::string message;
    while (true) {
        // Menerima pesan dari client
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            std::string encrypted_msg(buffer, bytes_received);
            try {
                std::string decrypted_msg = des_decrypt(encrypted_msg, key);
                std::cout << "Client: " << decrypted_msg << std::endl;
                if (decrypted_msg == "exit") {
                    break;
                }
            } catch (const std::exception& e) {
                std::cerr << "Decryption failed: " << e.what() << std::endl;
            }
        } else if (bytes_received == 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        } else {
            print_error("recv failed");
            break;
        }

        // Mengirim pesan ke client
        std::cout << "Server: ";
        std::getline(std::cin, message);
        try {
            std::string encrypted_response = des_encrypt(message, key);
            int bytes_sent = send(client_socket, encrypted_response.c_str(), encrypted_response.length(), 0);
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
    }

    // 8. Cleanup
    closesocket(client_socket);
    WSACleanup();
    std::cout << "Connection closed." << std::endl;

    return 0;
}
