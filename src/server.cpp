#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <mutex>
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
    #include <unistd.h> // untuk close() dan gethostname()
    #include <netdb.h> // untuk gethostbyname()
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
#endif

// --- Global Variables ---
std::mutex cout_mutex;

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

std::string get_local_ip() {
    char host_name[256];
    if (gethostname(host_name, sizeof(host_name)) == SOCKET_ERROR) {
        return "<unknown>";
    }
    hostent* host_entry = gethostbyname(host_name);
    if (host_entry == nullptr) {
        return "<unknown>";
    }
    char* ip_addr = inet_ntoa(*(struct in_addr*)*host_entry->h_addr_list);
    return std::string(ip_addr);
}

// --- Thread untuk Menerima Pesan ---
void receive_thread(socket_t socket, const std::string& key, const std::string& my_username, const std::string& peer_username) {
    char buffer[4096];
    while (true) {
        int bytes_received = recv(socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            std::string encrypted_msg(buffer, bytes_received);
            try {
                std::string decrypted_msg = des_decrypt(encrypted_msg, key);
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "\r" << peer_username << ": " << decrypted_msg << std::endl << my_username << ": ";
                if (decrypted_msg == "exit") {
                    closesocket(socket);
                    exit(0);
                }
            } catch (const std::exception& e) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cerr << "\rDecryption failed: " << e.what() << std::endl << my_username << ": ";
            }
        } else if (bytes_received == 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "\r" << peer_username << " disconnected." << std::endl;
            closesocket(socket);
            exit(0);
        } else {
            #ifdef _WIN32
                int error_code = WSAGetLastError();
                if (error_code != WSAEWOULDBLOCK) {
                    print_error("recv failed");
                    closesocket(socket);
                    exit(1);
                }
            #else
                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                    print_error("recv failed");
                    closesocket(socket);
                    exit(1);
                }
            #endif
        }
    }
}

// --- Main Program ---

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
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

    // 2. Buat & Bind Socket
    socket_t listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == INVALID_SOCKET) {
        print_error("Socket creation failed");
        cleanup_sockets();
        return 1;
    }

    int port = std::stoi(argv[1]);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        print_error("Bind failed");
        closesocket(listen_socket);
        cleanup_sockets();
        return 1;
    }

    // 3. Listen
    if (listen(listen_socket, 1) == SOCKET_ERROR) {
        print_error("Listen failed");
        closesocket(listen_socket);
        cleanup_sockets();
        return 1;
    }

    std::cout << "Server listening on " << get_local_ip() << ":" << port << std::endl;
    std::cout << "Waiting for a client to connect..." << std::endl;

    // 4. Accept
    socket_t client_socket = accept(listen_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET) {
        print_error("Accept failed");
        closesocket(listen_socket);
        cleanup_sockets();
        return 1;
    }

    closesocket(listen_socket);
    std::cout << "Client connected." << std::endl;

    // 5. Tukar Username
    std::string my_username, client_username;
    std::cout << "Enter your username: ";
    std::getline(std::cin, my_username);

    // Terima username dari client
    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        print_error("Failed to receive username from client");
        closesocket(client_socket);
        cleanup_sockets();
        return 1;
    }
    client_username = des_decrypt(std::string(buffer, bytes_received), key);

    // Kirim username ke client
    std::string encrypted_username = des_encrypt(my_username, key);
    send(client_socket, encrypted_username.c_str(), encrypted_username.length(), 0);

    std::cout << "You are now chatting with " << client_username << ". Type 'exit' to end." << std::endl;

    // 6. Buat dan jalankan thread penerima
    std::thread receiver(receive_thread, client_socket, key, my_username, client_username);
    receiver.detach();

    // 7. Loop Komunikasi (Hanya Mengirim)
    std::string message;
    while (true) {
        std::cout << my_username << ": ";
        std::getline(std::cin, message);
        try {
            std::string encrypted_response = des_encrypt(message, key);
            if (send(client_socket, encrypted_response.c_str(), encrypted_response.length(), 0) == SOCKET_ERROR) {
                print_error("Send failed");
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
    cleanup_sockets();
    std::cout << "Connection closed." << std::endl;

    return 0;
}