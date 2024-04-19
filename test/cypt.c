#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define SALT_LEN 16
#define HASH_LEN 64 // 64 字节的 bcrypt 哈希值长度

void generate_salt(unsigned char *salt) {
    RAND_bytes(salt, SALT_LEN);
}

void hash_password(const char *password, const unsigned char *salt, unsigned char *hash) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_bf_cbc(), NULL); // 使用 bcrypt 算法
    EVP_DigestUpdate(ctx, password, strlen(password));
    EVP_DigestUpdate(ctx, salt, SALT_LEN);
    EVP_DigestFinal_ex(ctx, hash, NULL);
    EVP_MD_CTX_free(ctx);
}

int main() {
    const char *password = "user_password";
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];

    // 生成盐值
    generate_salt(salt);

    // 哈希密码并存储哈希值
    hash_password(password, salt, hash);

    // 将哈希值和盐值存储在数据库中

    // 模拟密码匹配过程
    const char *input_password = "user_password"; // 用户提供的密码
    unsigned char input_hash[HASH_LEN];

    // 重新计算提供的密码的哈希值
    hash_password(input_password, salt, input_hash);

    // 比较重新计算的哈希值与存储的哈希值
    if (memcmp(hash, input_hash, HASH_LEN) == 0) {
        printf("Password is correct!\n");
    } else {
        printf("Password is incorrect!\n");
    }

    return 0;
}
