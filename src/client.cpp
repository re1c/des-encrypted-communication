#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <mutex>
#include <csignal>
#include "des.h"

// --- Cross-Platform Socket Headers ---
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> // untuk close()
    #include <sys/ioctl.h>
    #include <fcntl.h>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
#endif

// --- Global Variables ---
std::mutex cout_mutex;
socket_t client_socket = INVALID_SOCKET;

// --- Fungsi Bantuan Jaringan ---

void cleanup_sockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void handle_signal(int signum) {
    std::cout << "\nCaught signal " << signum << ". Shutting down gracefully." << std::endl;
    if (client_socket != INVALID_SOCKET) {
        closesocket(client_socket);
    }
    cleanup_sockets();
    exit(signum);
}

void init_sockets() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: WSAStartup failed" << std::endl;
        exit(1);
    }
#endif
}

void print_error(const std::string& message) {
#ifdef _WIN32
    std::cerr << "Error: " << message << " - " << WSAGetLastError() << std::endl;
#else
    perror(("Error: " + message).c_str());
#endif
}

// --- Thread untuk Menerima Pesan ---
void receive_thread(const std::string& key, const std::string& peer_username) {
    char buffer[4096];
    while (true) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        // Kondisi untuk berhenti
        if (bytes_received <= 0) {
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << std::endl << peer_username << " disconnected. Press Enter to exit." << std::endl;
            }
            #ifdef _WIN32
                // Workaround untuk unblock getline di Windows
                INPUT_RECORD r[1] = {}; 
                r[0].EventType = KEY_EVENT; 
                r[0].Event.KeyEvent.bKeyDown = TRUE; 
                r[0].Event.KeyEvent.wVirtualKeyCode = VK_RETURN; 
                DWORD written;
                WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), r, 1, &written);
            #else
                // Cara standar untuk unblock getline di POSIX
                shutdown(STDIN_FILENO, SHUT_RD);
            #endif
            return; // Keluar dari thread
        }

        std::string encrypted_response(buffer, bytes_received);
        try {
            std::string decrypted_response = des_decrypt(encrypted_response, key);
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << std::endl << peer_username << ": " << decrypted_response << std::endl;
            }
            if (decrypted_response == "exit") {
                // Pihak lain ingin keluar, unblock getline di main thread
                #ifdef _WIN32
                    INPUT_RECORD r[1] = {}; r[0].EventType = KEY_EVENT; r[0].Event.KeyEvent.bKeyDown = TRUE; r[0].Event.KeyEvent.wVirtualKeyCode = VK_RETURN; DWORD written; WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), r, 1, &written);
                #else
                    shutdown(STDIN_FILENO, SHUT_RD);
                #endif
                return; // Keluar dari thread
            }
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Decryption failed for incoming message: " << e.what() << std::endl;
        }
    }
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

    signal(SIGINT, handle_signal);
    init_sockets();

    // 2. Buat Socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
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

    std::cout << "Connected to server." << std::endl;

    // 4. Tukar Username
    std::string my_username, server_username;
    std::cout << "Enter your username: ";
    std::getline(std::cin, my_username);

    // Kirim username ke server
    std::string encrypted_username = des_encrypt(my_username, key);
    send(client_socket, encrypted_username.c_str(), encrypted_username.length(), 0);

    // Terima username dari server
    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        print_error("Failed to receive username from server");
        closesocket(client_socket);
        cleanup_sockets();
        return 1;
    }
    server_username = des_decrypt(std::string(buffer, bytes_received), key);

    std::cout << "You are now chatting with " << server_username << ". Type 'exit' to end." << std::endl;

    // 5. Buat dan jalankan thread penerima
    std::thread receiver(receive_thread, key, server_username);
    receiver.detach();

    // 6. Loop Komunikasi (Hanya Mengirim)
    std::string message;
    while (std::getline(std::cin, message)) {
        if (client_socket == INVALID_SOCKET) break; // Keluar jika socket sudah ditutup oleh thread lain

        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << my_username << ": " << message << std::endl;
        }
        try {
            std::string encrypted_message = des_encrypt(message, key);
            if (send(client_socket, encrypted_message.c_str(), encrypted_message.length(), 0) == SOCKET_ERROR) {
                print_error("Send failed");
                break;
            }
            if (message == "exit") {
                break;
            }
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Encryption failed for outgoing message: " << e.what() << std::endl;
        }
    }

    // 7. Cleanup
    if (client_socket != INVALID_SOCKET) {
        closesocket(client_socket);
    }
    cleanup_sockets();
    std::cout << "Connection closed." << std::endl;

    return 0;
}