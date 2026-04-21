// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "pzip/common.h"
#include <array>
#include <vector>

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
typedef struct hmac_ctx_st HMAC_CTX;

namespace pzip {

// AES 块大小
constexpr size_t AES_BLOCK_SIZE = 16;

/**
 * @brief WinZip AES 加密器
 *
 * 实现 WinZip AES 加密规范：
 * - PBKDF2-HMAC-SHA1 密钥派生
 * - AES-CTR 模式加密（手动实现，counter 小端序递增）
 * - HMAC-SHA1 认证码
 *
 * 线程安全：每个 FileTask 应创建独立的 AESEncryptor 实例
 */
class AESEncryptor {
public:
    AESEncryptor(const std::string& password, EncryptionMethod method);
    ~AESEncryptor();

    AESEncryptor(const AESEncryptor&) = delete;
    AESEncryptor& operator=(const AESEncryptor&) = delete;

    AESEncryptor(AESEncryptor&& other) noexcept;
    AESEncryptor& operator=(AESEncryptor&& other) noexcept;

    const std::vector<uint8_t>& salt() const { return salt_; }
    uint16_t passwordVerification() const { return passwordVerification_; }

    size_t encrypt(const uint8_t* input, size_t inputSize, uint8_t* output);
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& input);

    std::array<uint8_t, WINZIP_AES_AUTH_CODE_SIZE> authenticationCode();

    EncryptionMethod method() const { return method_; }
    bool isValid() const { return valid_; }
    const std::string& lastError() const { return lastError_; }

    size_t encryptedSize(size_t originalSize) const;

private:
    bool generateSalt();
    bool deriveKeys(const std::string& password);
    bool initCipher();
    bool initHMAC();
    void incrementCounter();
    bool aesCrypt(uint8_t* data, size_t length);

    EncryptionMethod method_;
    std::vector<uint8_t> salt_;
    std::vector<uint8_t> aesKey_;
    std::vector<uint8_t> hmacKey_;
    uint16_t passwordVerification_;

    EVP_CIPHER_CTX* aesCtx_;
    HMAC_CTX* hmacCtx_;

    bool valid_;
    std::string lastError_;

    std::vector<uint8_t> counterBlock_;
    std::vector<uint8_t> pad_;
    size_t padOffset_;
    uint64_t counter_;
};

/**
 * @brief WinZip AES 解密器
 */
class AESDecryptor {
public:
    AESDecryptor(const std::string& password, EncryptionMethod method,
                 const std::vector<uint8_t>& salt, uint16_t passwordVerification);
    ~AESDecryptor();

    AESDecryptor(const AESDecryptor&) = delete;
    AESDecryptor& operator=(const AESDecryptor&) = delete;

    AESDecryptor(AESDecryptor&& other) noexcept;
    AESDecryptor& operator=(AESDecryptor&& other) noexcept;

    bool verifyPassword() const { return passwordVerified_; }

    size_t decrypt(const uint8_t* input, size_t inputSize, uint8_t* output);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& input);

    bool verifyAuthenticationCode(const std::array<uint8_t, WINZIP_AES_AUTH_CODE_SIZE>& authCode);

    bool isValid() const { return valid_; }
    const std::string& lastError() const { return lastError_; }

private:
    bool deriveKeys(const std::string& password);
    bool initCipher();
    void incrementCounter();
    bool aesCrypt(uint8_t* data, size_t length);

    EncryptionMethod method_;
    std::vector<uint8_t> salt_;
    uint16_t expectedPasswordVerification_;
    uint16_t actualPasswordVerification_;
    std::vector<uint8_t> aesKey_;
    std::vector<uint8_t> hmacKey_;

    EVP_CIPHER_CTX* aesCtx_;
    HMAC_CTX* hmacCtx_;

    bool valid_;
    bool passwordVerified_;
    std::string lastError_;

    std::vector<uint8_t> counterBlock_;
    std::vector<uint8_t> pad_;
    size_t padOffset_;
};

} // namespace pzip
