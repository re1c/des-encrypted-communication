#ifndef DES_H
#define DES_H

#include <string>
#include <vector>

// Deklarasi fungsi utama untuk enkripsi dan dekripsi
// Menggunakan std::string untuk kemudahan, bisa diubah ke vector<char> jika perlu
std::string des_encrypt(const std::string& plaintext, const std::string& key);
std::string des_decrypt(const std::string& ciphertext, const std::string& key);

#endif // DES_H
