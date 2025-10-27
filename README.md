# Implementasi Komunikasi Client-Server Terenkripsi DES (Cross-Platform)

Proyek ini adalah implementasi sistem komunikasi dua arah (*bidirectional*) antara dua *device* yang diamankan menggunakan algoritma enkripsi **Data Encryption Standard (DES)**. Sistem ini dibangun dengan arsitektur **Client-Server** menggunakan C++, kompatibel untuk **Windows** dan **Linux**.

| Nama                   | NRP        | Kelas |
| ---------------------- | ---------- | ----- |
| Naswan Nashir Ramadhan | 5025231246 | C     |

> **Peringatan Keamanan Penting**
> Implementasi ini dibuat untuk **tujuan edukasi dan studi akademis**. DES merupakan standar enkripsi legasi yang dianggap **tidak aman** untuk aplikasi modern. **Jangan gunakan kode ini untuk mengamankan data sensitif pada sistem produksi.**

## 1. Arsitektur Sistem

Sistem ini terdiri dari dua komponen utama:

1.  **Server (`server.exe` di Windows, `server` di Linux):**
    *   Berperan sebagai pendengar pasif. Menunggu koneksi masuk dari klien pada *port* jaringan yang ditentukan.

2.  **Client (`client.exe` di Windows, `client` di Linux):**
    *   Berperan sebagai penghubung aktif. Menghubungi alamat IP dan *port* server untuk memulai komunikasi.

Setelah koneksi TCP berhasil dibuat, komunikasi menjadi **sepenuhnya dua arah (full-duplex)**.

## 2. Struktur Direktori Proyek

```
.
├── src/
│   ├── des.h           # Header untuk fungsi-fungsi DES
│   ├── des.cpp         # Implementasi logika enkripsi/dekripsi DES
│   ├── server.cpp      # Kode sumber Cross-Platform untuk server
│   └── client.cpp      # Kode sumber Cross-Platform untuk client
├── Makefile            # File instruksi kompilasi untuk `make` (Linux / Advanced)
├── build.bat           # Skrip kompilasi untuk Windows
├── key.txt             # File untuk menyimpan kunci DES (8 karakter ASCII)
└── README.md           # Dokumentasi ini
```

## 3. Prasyarat Lingkungan

*   **Sistem Operasi:** Windows atau Linux.
*   **Kompiler C++:** `g++` (misalnya dari MinGW/MSYS2 di Windows, atau `build-essential` di Linux).
*   **Jaringan:** Dua perangkat (fisik atau virtual) yang terhubung dalam satu jaringan (misalnya, LAN atau WLAN yang sama).

## 4. Cara Kompilasi

Pilih metode yang sesuai dengan sistem operasi Anda.

### Metode 1: Windows (Cara Mudah)

1.  Pastikan Anda memiliki `g++` yang terinstal dan dapat diakses dari terminal.
2.  Buka Command Prompt atau PowerShell di direktori utama proyek.
3.  Jalankan skrip `build.bat`:

    ```sh
    .\build.bat
    ```

4.  Ini akan menghasilkan `server.exe` dan `client.exe`.

### Metode 2: Linux atau Windows dengan `make`

1.  Pastikan Anda memiliki `g++` dan `make`.
2.  Buka terminal di direktori utama proyek.
3.  Jalankan perintah `make`:

    ```sh
    make
    ```

4.  Ini akan menghasilkan `server` dan `client` (di Linux) atau `server.exe` dan `client.exe` (di Windows).

## 5. Cara Penggunaan

### 5.1. Persiapan Kunci

*   Buka file `key.txt`.
*   Isi file tersebut dengan sebuah kunci rahasia yang terdiri dari **tepat 8 karakter ASCII**. Contoh: `kunci123`.
*   Pastikan file `key.txt` ini identik di kedua perangkat.

### 5.2. Menjalankan Server

1.  Di perangkat pertama, buka terminal dan masuk ke direktori proyek.
2.  **Cari tahu alamat IP lokal perangkat:**
    *   Di **Windows:** Jalankan `ipconfig` dan cari alamat "IPv4 Address".
    *   Di **Linux:** Jalankan `ip addr` atau `hostname -I` dan cari alamat IP Anda.
3.  **Jalankan server** dengan menentukan *port* (misal: `8080`).
    *   Di **Windows:**
        ```sh
        .\server.exe 8080
        ```
    *   Di **Linux:**
        ```sh
        ./server 8080
        ```
4.  Server akan menampilkan pesan bahwa ia sedang menunggu koneksi.

### 5.3. Menjalankan Klien dan Memulai Chat

1.  Di perangkat kedua, buka terminal dan masuk ke direktori proyek.
2.  **Jalankan klien** dengan memasukkan **alamat IP server** dan **port** yang sama.
    *   Di **Windows:** (Ganti `192.168.1.10` dengan IP server)
        ```sh
        .\client.exe 192.168.1.10 8080
        ```
    *   Di **Linux:** (Ganti `192.168.1.10` dengan IP server)
        ```sh
        ./client 192.168.1.10 8080
        ```
3.  Jika koneksi berhasil, kedua terminal akan menampilkan pesan bahwa chat telah dimulai.
4.  Ketik pesan di salah satu terminal, tekan Enter, dan pesan akan muncul di terminal lainnya.
5.  Untuk mengakhiri sesi, ketik `exit` di salah satu terminal.