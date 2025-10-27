# Implementasi Komunikasi Client-Server Terenkripsi DES

Proyek ini adalah implementasi sistem komunikasi dua arah (bidirectional) antara dua *device* yang diamankan menggunakan algoritma enkripsi **Data Encryption Standard (DES)**. Sistem ini dibangun dengan arsitektur **Client-Server** menggunakan C++ dan library **Winsock** untuk komunikasi jaringan berbasis TCP.

| Nama                   | NRP        | Kelas |
| ---------------------- | ---------- | ----- |
| Naswan Nashir Ramadhan | 5025231246 | C     |

> **Peringatan Keamanan Penting**
> Implementasi ini dibuat untuk **tujuan edukasi dan studi akademis**. DES merupakan standar enkripsi legasi yang dianggap **tidak aman** untuk aplikasi modern. **Jangan gunakan kode ini untuk mengamankan data sensitif pada sistem produksi.**

## 1. Arsitektur Sistem

Sistem ini terdiri dari dua komponen utama:

1.  **Server (`server.exe`):**
    *   Berperan sebagai pendengar pasif.
    *   Ketika dijalankan, server akan membuka *port* jaringan dan menunggu koneksi masuk dari klien.
    *   Setelah klien terhubung, server dapat menerima pesan terenkripsi, mendekripsinya, dan mengirim balasan yang terenkripsi.

2.  **Client (`client.exe`):**
    *   Berperan sebagai penghubung aktif.
    *   Ketika dijalankan, klien akan mencoba terhubung ke alamat IP dan *port* yang ditentukan pengguna.
    *   Setelah terhubung, klien dapat mengirim pesan terenkripsi, menerima balasan terenkripsi dari server, dan mendekripsinya.

Setelah koneksi TCP berhasil dibuat, komunikasi menjadi **sepenuhnya dua arah (full-duplex)**, di mana kedua belah pihak dapat mengirim dan menerima pesan secara bergantian.

## 2. Struktur Direktori Proyek

```
.
├── src/
│   ├── des.h           # Header untuk fungsi-fungsi DES
│   ├── des.cpp         # Implementasi logika enkripsi/dekripsi DES
│   ├── server.cpp      # Kode sumber untuk program server
│   └── client.cpp      # Kode sumber untuk program client
├── build.bat           # Skrip untuk kompilasi server dan client
├── key.txt             # File untuk menyimpan kunci DES (8 karakter ASCII)
└── README.md           # Dokumentasi ini
```

## 3. Prasyarat Lingkungan

*   **Sistem Operasi:** Windows (karena menggunakan library Winsock).
*   **Kompiler C++:** g++ (MinGW) atau kompiler lain yang kompatibel. Pastikan `g++` ada di PATH environment variable Anda.
*   **Jaringan:** Dua perangkat (fisik atau virtual) yang terhubung dalam satu jaringan (misalnya, LAN atau WLAN yang sama) agar bisa saling berkomunikasi melalui alamat IP lokal.

## 4. Cara Kompilasi

1.  Buka terminal (Command Prompt atau PowerShell) di direktori utama proyek (`des-encrypted-communication`).
2.  Jalankan skrip kompilasi:

    ```sh
    .\build.bat
    ```

3.  Jika berhasil, akan ada dua file baru yang dihasilkan di direktori utama: `server.exe` dan `client.exe`.

## 5. Cara Penggunaan

### 5.1. Persiapan Kunci

*   Buka file `key.txt`.
*   Isi file tersebut dengan sebuah kunci rahasia yang terdiri dari **tepat 8 karakter ASCII**. Contoh: `kunci123`.
*   Pastikan file `key.txt` ini sama persis di kedua perangkat (perangkat server dan perangkat klien).

### 5.2. Menjalankan Server

1.  Di perangkat pertama, buka terminal.
2.  Cari tahu alamat IP lokal perangkat tersebut. Buka Command Prompt dan ketik `ipconfig`. Cari alamat IPv4 Anda (misalnya, `192.168.1.10`).
3.  Jalankan `server.exe` dengan menentukan *port* yang akan digunakan. Port `8080` adalah pilihan umum.

    ```sh
    .\server.exe 8080
    ```

4.  Server akan menampilkan pesan bahwa ia sedang menunggu koneksi di alamat IP dan port tersebut.

### 5.3. Menjalankan Klien dan Memulai Chat

1.  Di perangkat kedua, buka terminal.
2.  Jalankan `client.exe` dengan memasukkan **alamat IP server** dan **port** yang sama.

    ```sh
    .\client.exe 192.168.1.10 8080
    ```
    *(Ganti `192.168.1.10` dengan alamat IP server yang sebenarnya)*

3.  Jika koneksi berhasil, kedua terminal (klien dan server) akan menampilkan pesan bahwa chat telah dimulai.
4.  Anda sekarang dapat mengetik pesan di salah satu terminal, tekan Enter, dan pesan tersebut akan muncul di terminal lainnya setelah dienkripsi, dikirim, dan didekripsi.
5.  Untuk mengakhiri sesi, ketik `exit` di salah satu terminal.

## 6. Detail Implementasi Teknis

*   **Adaptasi DES:** Logika DES dari Tugas 1 diadaptasi untuk bekerja dengan `std::string` di memori, bukan file.
*   **Padding:** Algoritma DES beroperasi pada blok 64-bit (8 byte). Untuk mengenkripsi pesan yang panjangnya bukan kelipatan 8, standar **PKCS#5 Padding** diimplementasikan. Sebelum enkripsi, byte tambahan ditambahkan untuk mencapai panjang kelipatan 8. Setelah dekripsi, byte-byte ini dihapus.
*   **Networking:** Komunikasi jaringan ditangani oleh **Winsock API**. Server menggunakan alur `socket() -> bind() -> listen() -> accept()`, sementara klien menggunakan `socket() -> connect()`. Pengiriman dan penerimaan data dilakukan dengan `send()` dan `recv()`.