# Implementasi Sistem Komunikasi Client-Server dengan Enkripsi DES

## Abstrak

Proyek ini merupakan implementasi dari sebuah sistem komunikasi *client-server* berbasis teks yang diamankan menggunakan algoritma kriptografi simetris Data Encryption Standard (DES). Sistem ini dirancang untuk dapat beroperasi secara lintas platform (Windows dan Linux) dan mengimplementasikan model komputasi konkuren untuk memungkinkan komunikasi dua arah secara simultan. Tujuan dari proyek ini adalah untuk mendemonstrasikan pemahaman dan penerapan konsep-konsep fundamental dalam keamanan informasi dan pemrograman jaringan, termasuk kriptografi, soket TCP/IP, dan *multithreading*.

| Nama                   | NRP        | Kelas |
| ---------------------- | ---------- | ----- |
| Naswan Nashir Ramadhan | 5025231246 | C     |

> **Catatan Keamanan:** Implementasi ini dirancang secara spesifik untuk tujuan edukasional. Algoritma DES merupakan standar enkripsi legasi yang memiliki kerentanan dan tidak direkomendasikan untuk pengamanan data pada aplikasi modern.

---

## 1. Fungsionalitas Sistem

Sistem yang diimplementasikan memiliki fungsionalitas sebagai berikut:

* **Enkripsi Data:** Seluruh data percakapan yang ditransmisikan melalui jaringan dienkripsi menggunakan algoritma DES.
* **Komunikasi Dua Arah:** Sistem memungkinkan pertukaran pesan secara *full-duplex*, di mana pengguna dapat mengirim dan menerima pesan secara bersamaan.
* **Kompatibilitas Lintas Platform:** Aplikasi dapat dikompilasi dan dijalankan pada sistem operasi Windows dan Linux tanpa perubahan kode sumber.
* **Identitas Pengguna Dinamis:** Setiap pengguna dapat menetapkan *username* pada awal sesi komunikasi.
* **Terminasi Program yang Stabil:** Sesi koneksi dapat diakhiri secara terkendali (*gracefully*) oleh kedua belah pihak, memastikan stabilitas program.

## 2. Arsitektur dan Desain Perangkat Lunak

Arsitektur sistem ini dibangun di atas tiga komponen utama yang saling terintegrasi.

### 2.1. Modul Kriptografi (DES)

Komponen ini bertanggung jawab atas kerahasiaan data.

* **Logika:** Implementasi algoritma DES terdapat pada `src/des.h` dan `src/des.cpp`.
* **Mekanisme:** Sistem mengadopsi skema kriptografi kunci simetris. Sebuah kunci rahasia tunggal berukuran 8-byte, yang dibaca dari `key.txt`, digunakan untuk proses enkripsi pada sisi pengirim dan dekripsi pada sisi penerima.

### 2.2. Modul Jaringan (Soket TCP/IP)

Komponen ini mengelola konektivitas dan transmisi data antara dua entitas.

* **Model:** Sistem mengadopsi arsitektur *Client-Server*. Entitas *server* berfungsi sebagai *listener* pasif yang menunggu koneksi pada *port* yang ditentukan, sementara entitas *client* secara aktif menginisiasi koneksi ke alamat IP dan *port* server.
* **Protokol:** Digunakan protokol TCP/IP yang bersifat *connection-oriented* untuk menjamin reliabilitas, integritas, dan keterurutan pengiriman data.
* **Implementasi Lintas Platform:** Kode sumber memanfaatkan direktif *preprocessor* (`#ifdef _WIN32`) untuk menggunakan pustaka yang sesuai pada setiap sistem operasi (Winsock untuk Windows, POSIX Sockets untuk Linux).
* **Optimalisasi Soket:** Opsi `SO_REUSEADDR` diaktifkan pada soket *server* untuk memitigasi error `Address already in use` dan meningkatkan reliabilitas saat server perlu di-restart.

### 2.3. Model Konkurensi (Multithreading)

Untuk mencapai komunikasi dua arah yang simultan, sistem menerapkan model eksekusi konkuren menggunakan `std::thread`.

* **Thread Utama:** Didedikasikan untuk menangani input dari pengguna melalui `std::getline` dan mengirimkan data yang telah dienkripsi ke jaringan.
* **Thread Penerima:** Dijalankan secara terpisah di latar belakang. *Thread* ini secara eksklusif bertugas untuk menunggu data masuk dari soket (`recv()`). Ketika data diterima, *thread* ini akan melakukan dekripsi dan menampilkannya ke konsol.
* **Sinkronisasi:** Sebuah `std::mutex` diimplementasikan untuk mengatur akses ke sumber daya bersama, yaitu `std::cout`. Ini mencegah terjadinya *race condition* yang dapat merusak tampilan output di konsol.

### 2.4. Penanganan Koneksi dan Terminasi

Sistem dirancang untuk dapat melakukan terminasi sesi secara bersih.

* **Mekanisme:** Ketika koneksi berakhir (baik karena perintah `exit` maupun karena terputus), *thread* penerima tidak mematikan program secara paksa. Sebaliknya, ia akan memicu `std::getline` pada *thread* utama untuk berhenti memblokir. Hal ini memungkinkan *thread* utama untuk keluar dari *loop*-nya secara alami, melakukan pembersihan sumber daya (seperti penutupan soket), dan mengakhiri program dengan stabil.

## 3. Panduan Kompilasi dan Operasional

### 3.1. Prasyarat Sistem

* **Sistem Operasi:** Windows atau distribusi Linux.
* **Kompiler:** `g++` (bagian dari MinGW-w64 di Windows atau `build-essential` di Linux).
* **Utilitas `make`:** Direkomendasikan untuk kompilasi di lingkungan Linux.

### 3.2. Prosedur Kompilasi

Navigasikan ke direktori root proyek melalui terminal atau command prompt.

* **Pada Windows:**

  ```sh
  .\build.bat
  ```

  Perintah ini akan menghasilkan `server.exe` dan `client.exe`.
* **Pada Linux:**

  ```sh
  make
  ```

  Perintah ini akan menghasilkan *executable* `server` dan `client`.

### 3.3. Prosedur Operasional

1. **Konfigurasi Kunci:** Sunting file `key.txt` dan isi dengan kunci rahasia berukuran **tepat 8 karakter**. Pastikan file ini identik pada kedua sistem.
2. **Inisiasi Server:** Pada mesin pertama, jalankan *executable* server dengan argumen berupa nomor *port*.

   * Contoh:
     ```sh
     # Windows
     .\server.exe 8080

     # Linux
     ./server 8080
     ```
3. **Inisiasi Client:** Pada mesin kedua, jalankan *executable* client dengan argumen berupa alamat IP server dan nomor *port* yang sama.

   * Contoh (asumsi IP server adalah `192.168.1.5`):
     ```sh
     # Windows
     .\client.exe 192.168.1.5 8080

     # Linux
     ./client 192.168.1.5 8080
     ```
4. **Sesi Komunikasi:** Setelah koneksi terjalin, kedua pengguna akan diminta memasukkan *username*. Selanjutnya, pertukaran pesan dapat dilakukan. Sesi dapat diakhiri dengan mengetik `exit`.

---
