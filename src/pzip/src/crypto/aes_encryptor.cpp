// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pzip/crypto/aes_encryptor.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <algorithm>
#include <cstring>

namespace pzip {

//=============================================================================
// AESEncryptor 实现
//=============================================================================

AESEncryptor::AESEncryptor(const std::string& password, EncryptionMethod method)
    : method_(method)
    , aesCtx_(nullptr)
    , hmacCtx_(nullptr)
    , valid_(false)
    , counter_(0)
    , padOffset_(AES_BLOCK_SIZE)
{
    if (method == EncryptionMethod::None || password.empty()) {
        lastError_ = "Invalid encryption method or empty password";
        return;
    }

    size_t keySize = encryptionKeySize(method);
    size_t saltSize = encryptionSaltSize(method);

    if (keySize == 0 || saltSize == 0) {
        lastError_ = "Unsupported encryption method";
        return;
    }

    salt_.resize(saltSize);
    aesKey_.resize(keySize);
    hmacKey_.resize(keySize);
    counterBlock_.resize(AES_BLOCK_SIZE, 0);
    pad_.resize(AES_BLOCK_SIZE, 0);

    if (!generateSalt()) {
        lastError_ = "CSPRNG failed: RAND_bytes could not generate salt";
        return;
    }

    if (!deriveKeys(password)) {
        return;
    }

    if (!initCipher()) {
        return;
    }

    if (!initHMAC()) {
        return;
    }

    valid_ = true;
}

AESEncryptor::~AESEncryptor() {
    if (aesCtx_) {
        EVP_CIPHER_CTX_free(aesCtx_);
        aesCtx_ = nullptr;
    }
    if (hmacCtx_) {
        HMAC_CTX_free(hmacCtx_);
        hmacCtx_ = nullptr;
    }
    std::fill(aesKey_.begin(), aesKey_.end(), 0);
    std::fill(hmacKey_.begin(), hmacKey_.end(), 0);
    std::fill(counterBlock_.begin(), counterBlock_.end(), 0);
    std::fill(pad_.begin(), pad_.end(), 0);
}

AESEncryptor::AESEncryptor(AESEncryptor&& other) noexcept
    : method_(other.method_)
    , salt_(std::move(other.salt_))
    , aesKey_(std::move(other.aesKey_))
    , hmacKey_(std::move(other.hmacKey_))
    , passwordVerification_(other.passwordVerification_)
    , aesCtx_(other.aesCtx_)
    , hmacCtx_(other.hmacCtx_)
    , valid_(other.valid_)
    , lastError_(std::move(other.lastError_))
    , counterBlock_(std::move(other.counterBlock_))
    , pad_(std::move(other.pad_))
    , padOffset_(other.padOffset_)
    , counter_(other.counter_)
{
    other.aesCtx_ = nullptr;
    other.hmacCtx_ = nullptr;
    other.valid_ = false;
}

AESEncryptor& AESEncryptor::operator=(AESEncryptor&& other) noexcept {
    if (this != &other) {
        if (aesCtx_) {
            EVP_CIPHER_CTX_free(aesCtx_);
        }
        if (hmacCtx_) {
            HMAC_CTX_free(hmacCtx_);
        }

        method_ = other.method_;
        salt_ = std::move(other.salt_);
        aesKey_ = std::move(other.aesKey_);
        hmacKey_ = std::move(other.hmacKey_);
        passwordVerification_ = other.passwordVerification_;
        aesCtx_ = other.aesCtx_;
        hmacCtx_ = other.hmacCtx_;
        valid_ = other.valid_;
        lastError_ = std::move(other.lastError_);
        counterBlock_ = std::move(other.counterBlock_);
        pad_ = std::move(other.pad_);
        padOffset_ = other.padOffset_;
        counter_ = other.counter_;

        other.aesCtx_ = nullptr;
        other.hmacCtx_ = nullptr;
        other.valid_ = false;
    }
    return *this;
}

bool AESEncryptor::generateSalt() {
    return RAND_bytes(salt_.data(), static_cast<int>(salt_.size())) == 1;
}

bool AESEncryptor::deriveKeys(const std::string& password) {
    size_t derivedKeyLen = aesKey_.size() + hmacKey_.size() + WINZIP_AES_PV_SIZE;
    std::vector<uint8_t> derivedKey(derivedKeyLen);

    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        static_cast<int>(password.size()),
        salt_.data(),
        static_cast<int>(salt_.size()),
        PBKDF2_ITERATIONS,
        EVP_sha1(),
        static_cast<int>(derivedKeyLen),
        derivedKey.data()
    );

    if (result != 1) {
        lastError_ = "PBKDF2 key derivation failed";
        return false;
    }

    size_t offset = 0;
    std::copy(derivedKey.begin() + offset,
              derivedKey.begin() + offset + aesKey_.size(),
              aesKey_.begin());
    offset += aesKey_.size();

    std::copy(derivedKey.begin() + offset,
              derivedKey.begin() + offset + hmacKey_.size(),
              hmacKey_.begin());
    offset += hmacKey_.size();

    passwordVerification_ = static_cast<uint16_t>(derivedKey[offset]) |
                            (static_cast<uint16_t>(derivedKey[offset + 1]) << 8);

    std::fill(derivedKey.begin(), derivedKey.end(), 0);
    return true;
}

bool AESEncryptor::initCipher() {
    aesCtx_ = EVP_CIPHER_CTX_new();
    if (!aesCtx_) {
        lastError_ = "Failed to create cipher context";
        return false;
    }

    const EVP_CIPHER* cipher = nullptr;
    switch (method_) {
        case EncryptionMethod::AES128:
            cipher = EVP_aes_128_ecb();
            break;
        case EncryptionMethod::AES192:
            cipher = EVP_aes_192_ecb();
            break;
        case EncryptionMethod::AES256:
            cipher = EVP_aes_256_ecb();
            break;
        default:
            lastError_ = "Unsupported encryption method";
            return false;
    }

    if (EVP_EncryptInit_ex(aesCtx_, cipher, nullptr, aesKey_.data(), nullptr) != 1) {
        lastError_ = "Failed to initialize cipher";
        return false;
    }
    EVP_CIPHER_CTX_set_padding(aesCtx_, 0);

    return true;
}

bool AESEncryptor::initHMAC() {
    hmacCtx_ = HMAC_CTX_new();
    if (!hmacCtx_) {
        lastError_ = "Failed to create HMAC context";
        return false;
    }

    if (HMAC_Init_ex(hmacCtx_, hmacKey_.data(), static_cast<int>(hmacKey_.size()),
                     EVP_sha1(), nullptr) != 1) {
        lastError_ = "Failed to initialize HMAC";
        return false;
    }

    return true;
}

void AESEncryptor::incrementCounter() {
    for (int j = 0; j < 8; j++) {
        counterBlock_[j]++;
        if (counterBlock_[j] != 0) {
            break;
        }
    }
}

bool AESEncryptor::aesCrypt(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (padOffset_ == AES_BLOCK_SIZE) {
            incrementCounter();

            int outLen = 0;
            if (EVP_EncryptUpdate(aesCtx_, pad_.data(), &outLen,
                                  counterBlock_.data(), AES_BLOCK_SIZE) != 1) {
                lastError_ = "AES encryption failed";
                return false;
            }
            padOffset_ = 0;
        }
        data[i] ^= pad_[padOffset_++];
    }
    return true;
}

size_t AESEncryptor::encrypt(const uint8_t* input, size_t inputSize, uint8_t* output) {
    if (!valid_ || !aesCtx_ || !hmacCtx_) {
        return 0;
    }

    std::memcpy(output, input, inputSize);

    if (!aesCrypt(output, inputSize)) {
        return 0;
    }

    if (HMAC_Update(hmacCtx_, output, inputSize) != 1) {
        lastError_ = "HMAC update failed";
        return 0;
    }

    return inputSize;
}

std::vector<uint8_t> AESEncryptor::encrypt(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> output(input.size());
    size_t outLen = encrypt(input.data(), input.size(), output.data());
    if (outLen == 0) {
        return {};
    }
    output.resize(outLen);
    return output;
}

std::array<uint8_t, WINZIP_AES_AUTH_CODE_SIZE> AESEncryptor::authenticationCode() {
    std::array<uint8_t, WINZIP_AES_AUTH_CODE_SIZE> authCode{};

    if (!valid_ || !hmacCtx_) {
        return authCode;
    }

    unsigned int macLen = SHA_DIGEST_LENGTH;
    uint8_t mac[SHA_DIGEST_LENGTH];

    if (HMAC_Final(hmacCtx_, mac, &macLen) != 1) {
        lastError_ = "HMAC final failed";
        return authCode;
    }

    std::copy(mac, mac + WINZIP_AES_AUTH_CODE_SIZE, authCode.begin());

    return authCode;
}

size_t AESEncryptor::encryptedSize(size_t originalSize) const {
    size_t saltSz = encryptionSaltSize(method_);
    return saltSz + WINZIP_AES_PV_SIZE + originalSize + WINZIP_AES_AUTH_CODE_SIZE;
}

//=============================================================================
// AESDecryptor 实现
//=============================================================================

AESDecryptor::AESDecryptor(const std::string& password, EncryptionMethod method,
                           const std::vector<uint8_t>& salt, uint16_t passwordVerification)
    : method_(method)
    , salt_(salt)
    , expectedPasswordVerification_(passwordVerification)
    , aesCtx_(nullptr)
    , hmacCtx_(nullptr)
    , valid_(false)
    , passwordVerified_(false)
    , padOffset_(AES_BLOCK_SIZE)
{
    if (method == EncryptionMethod::None || password.empty() || salt.empty()) {
        lastError_ = "Invalid decryption parameters";
        return;
    }

    size_t keySize = encryptionKeySize(method);
    if (keySize == 0) {
        lastError_ = "Unsupported encryption method";
        return;
    }

    aesKey_.resize(keySize);
    hmacKey_.resize(keySize);
    counterBlock_.resize(AES_BLOCK_SIZE, 0);
    pad_.resize(AES_BLOCK_SIZE, 0);

    if (!deriveKeys(password)) {
        return;
    }

    passwordVerified_ = (actualPasswordVerification_ == expectedPasswordVerification_);
    if (!passwordVerified_) {
        lastError_ = "Wrong password";
        return;
    }

    if (!initCipher()) {
        return;
    }

    hmacCtx_ = HMAC_CTX_new();
    if (!hmacCtx_) {
        lastError_ = "Failed to create HMAC context";
        return;
    }

    if (HMAC_Init_ex(hmacCtx_, hmacKey_.data(), static_cast<int>(hmacKey_.size()),
                     EVP_sha1(), nullptr) != 1) {
        lastError_ = "Failed to initialize HMAC";
        return;
    }

    valid_ = true;
}

AESDecryptor::~AESDecryptor() {
    if (aesCtx_) {
        EVP_CIPHER_CTX_free(aesCtx_);
        aesCtx_ = nullptr;
    }
    if (hmacCtx_) {
        HMAC_CTX_free(hmacCtx_);
        hmacCtx_ = nullptr;
    }
    std::fill(aesKey_.begin(), aesKey_.end(), 0);
    std::fill(hmacKey_.begin(), hmacKey_.end(), 0);
    std::fill(counterBlock_.begin(), counterBlock_.end(), 0);
    std::fill(pad_.begin(), pad_.end(), 0);
}

AESDecryptor::AESDecryptor(AESDecryptor&& other) noexcept
    : method_(other.method_)
    , salt_(std::move(other.salt_))
    , expectedPasswordVerification_(other.expectedPasswordVerification_)
    , actualPasswordVerification_(other.actualPasswordVerification_)
    , aesKey_(std::move(other.aesKey_))
    , hmacKey_(std::move(other.hmacKey_))
    , aesCtx_(other.aesCtx_)
    , hmacCtx_(other.hmacCtx_)
    , valid_(other.valid_)
    , passwordVerified_(other.passwordVerified_)
    , lastError_(std::move(other.lastError_))
    , counterBlock_(std::move(other.counterBlock_))
    , pad_(std::move(other.pad_))
    , padOffset_(other.padOffset_)
{
    other.aesCtx_ = nullptr;
    other.hmacCtx_ = nullptr;
    other.valid_ = false;
}

AESDecryptor& AESDecryptor::operator=(AESDecryptor&& other) noexcept {
    if (this != &other) {
        if (aesCtx_) {
            EVP_CIPHER_CTX_free(aesCtx_);
        }
        if (hmacCtx_) {
            HMAC_CTX_free(hmacCtx_);
        }

        method_ = other.method_;
        salt_ = std::move(other.salt_);
        expectedPasswordVerification_ = other.expectedPasswordVerification_;
        actualPasswordVerification_ = other.actualPasswordVerification_;
        aesKey_ = std::move(other.aesKey_);
        hmacKey_ = std::move(other.hmacKey_);
        aesCtx_ = other.aesCtx_;
        hmacCtx_ = other.hmacCtx_;
        valid_ = other.valid_;
        passwordVerified_ = other.passwordVerified_;
        lastError_ = std::move(other.lastError_);
        counterBlock_ = std::move(other.counterBlock_);
        pad_ = std::move(other.pad_);
        padOffset_ = other.padOffset_;

        other.aesCtx_ = nullptr;
        other.hmacCtx_ = nullptr;
        other.valid_ = false;
    }
    return *this;
}

bool AESDecryptor::deriveKeys(const std::string& password) {
    size_t keySize = aesKey_.size();
    size_t derivedKeyLen = keySize + keySize + WINZIP_AES_PV_SIZE;
    std::vector<uint8_t> derivedKey(derivedKeyLen);

    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        static_cast<int>(password.size()),
        salt_.data(),
        static_cast<int>(salt_.size()),
        PBKDF2_ITERATIONS,
        EVP_sha1(),
        static_cast<int>(derivedKeyLen),
        derivedKey.data()
    );

    if (result != 1) {
        lastError_ = "PBKDF2 key derivation failed";
        return false;
    }

    size_t offset = 0;
    std::copy(derivedKey.begin() + offset,
              derivedKey.begin() + offset + keySize,
              aesKey_.begin());
    offset += keySize;

    std::copy(derivedKey.begin() + offset,
              derivedKey.begin() + offset + keySize,
              hmacKey_.begin());
    offset += keySize;

    actualPasswordVerification_ = static_cast<uint16_t>(derivedKey[offset]) |
                                   (static_cast<uint16_t>(derivedKey[offset + 1]) << 8);

    std::fill(derivedKey.begin(), derivedKey.end(), 0);
    return true;
}

bool AESDecryptor::initCipher() {
    aesCtx_ = EVP_CIPHER_CTX_new();
    if (!aesCtx_) {
        lastError_ = "Failed to create cipher context";
        return false;
    }

    const EVP_CIPHER* cipher = nullptr;
    switch (method_) {
        case EncryptionMethod::AES128:
            cipher = EVP_aes_128_ecb();
            break;
        case EncryptionMethod::AES192:
            cipher = EVP_aes_192_ecb();
            break;
        case EncryptionMethod::AES256:
            cipher = EVP_aes_256_ecb();
            break;
        default:
            lastError_ = "Unsupported encryption method";
            return false;
    }

    if (EVP_EncryptInit_ex(aesCtx_, cipher, nullptr, aesKey_.data(), nullptr) != 1) {
        lastError_ = "Failed to initialize cipher";
        return false;
    }
    EVP_CIPHER_CTX_set_padding(aesCtx_, 0);

    return true;
}

void AESDecryptor::incrementCounter() {
    for (int j = 0; j < 8; j++) {
        counterBlock_[j]++;
        if (counterBlock_[j] != 0) {
            break;
        }
    }
}

bool AESDecryptor::aesCrypt(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (padOffset_ == AES_BLOCK_SIZE) {
            incrementCounter();

            int outLen = 0;
            if (EVP_EncryptUpdate(aesCtx_, pad_.data(), &outLen,
                                  counterBlock_.data(), AES_BLOCK_SIZE) != 1) {
                lastError_ = "AES encryption failed";
                return false;
            }
            padOffset_ = 0;
        }
        data[i] ^= pad_[padOffset_++];
    }
    return true;
}

size_t AESDecryptor::decrypt(const uint8_t* input, size_t inputSize, uint8_t* output) {
    if (!valid_ || !aesCtx_ || !hmacCtx_) {
        return 0;
    }

    if (HMAC_Update(hmacCtx_, input, inputSize) != 1) {
        lastError_ = "HMAC update failed";
        return 0;
    }

    std::memcpy(output, input, inputSize);

    if (!aesCrypt(output, inputSize)) {
        return 0;
    }

    return inputSize;
}

std::vector<uint8_t> AESDecryptor::decrypt(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> output(input.size());
    size_t outLen = decrypt(input.data(), input.size(), output.data());
    if (outLen == 0) {
        return {};
    }
    output.resize(outLen);
    return output;
}

bool AESDecryptor::verifyAuthenticationCode(const std::array<uint8_t, WINZIP_AES_AUTH_CODE_SIZE>& authCode) {
    if (!valid_ || !hmacCtx_) {
        return false;
    }

    unsigned int macLen = SHA_DIGEST_LENGTH;
    uint8_t mac[SHA_DIGEST_LENGTH];

    if (HMAC_Final(hmacCtx_, mac, &macLen) != 1) {
        lastError_ = "HMAC final failed";
        return false;
    }

    uint8_t diff = 0;
    for (size_t i = 0; i < WINZIP_AES_AUTH_CODE_SIZE; ++i) {
        diff |= mac[i] ^ authCode[i];
    }

    return diff == 0;
}

} // namespace pzip
