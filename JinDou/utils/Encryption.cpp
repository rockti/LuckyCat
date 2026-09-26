#pragma once
#include "Encryption.h"
#include <windows.h>
#include <sstream>
#include <fmt/format.h>
#include <plusaes.hpp>
#include <picohash.h>
#include <process.h>
#include <time.h>
#include "String.h"

using namespace std;
static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string gen_random(const int len) {

	std::string tmp_s;

	srand((unsigned)time(NULL) * _getpid());

	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i)
		tmp_s += base64_chars[rand() % (sizeof(base64_chars) - 1)];


	return tmp_s;

}
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}
std::string base64_decode(std::string const& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

struct file_closer {
	file_closer(HANDLE& h) :h_(h) {}
	~file_closer() {
		if (h_ != INVALID_HANDLE_VALUE) CloseHandle(h_);
	}
	HANDLE& h_;
};

#define DFile(name)  HANDLE name = INVALID_HANDLE_VALUE; file_closer a##_##name{ name };

inline void mix(char* buffer, DWORD len) {
	for (size_t i = 0; i < len / 2; ++i) {
		char tmp = buffer[i];
		buffer[i] = buffer[len - i - 1];
		buffer[len - i - 1] = tmp;
	}
}

bool SecureFile::EncryptString(std::string const& content, std::string const& password, std::string& outEncrypted) {
	outEncrypted.clear();
	picohash_ctx_t ctx;
	char digest[PICOHASH_MD5_DIGEST_LENGTH + 1];
	picohash_init_md5(&ctx);
	picohash_update(&ctx, password.c_str(), password.size());
	picohash_final(&ctx, digest);

	const std::vector<unsigned char> key = plusaes::key_from_string(&digest); // 16-char = 128-bit

	const unsigned char iv[16] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	};

	const unsigned long encrypted_size = plusaes::get_padded_encrypted_size(content.size());
	std::vector<unsigned char> encrypted(encrypted_size);

	auto error = plusaes::encrypt_cbc((unsigned char*)content.data(), content.size(), &key[0], key.size(), &iv, &encrypted[0], encrypted.size(), true);
	for (auto c : encrypted) {
		outEncrypted.push_back((char)c);
	}
	return error == 0;
}

bool SecureFile::DecryptString(std::string const& encryptedContent, std::string const& password, std::string& outDecrypted) {
	picohash_ctx_t ctx;
	char digest[PICOHASH_MD5_DIGEST_LENGTH + 1];

	picohash_init_md5(&ctx);
	picohash_update(&ctx, password.c_str(), password.size());
	picohash_final(&ctx, digest);
	const std::vector<unsigned char> key = plusaes::key_from_string(&digest); // 16-char = 128-bit
	const unsigned char iv[16] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	};


	unsigned long padded_size = 0;
	std::vector<unsigned char> decrypted(encryptedContent.size());

	auto error = plusaes::decrypt_cbc((const unsigned char*)encryptedContent.data(), encryptedContent.size(), &key[0], key.size(), &iv, &decrypted[0], decrypted.size(), &padded_size);
	outDecrypted.clear();
	for (auto c : decrypted) {
		outDecrypted.push_back((char)c);
	}
	return error == 0;
}

bool SecureFile::Encrypt(TString const& plainFile, TString const& encryptedFile) {
	DFile(hSourceFile);
	DFile(hDestinationFile);


	hSourceFile = CreateFile(plainFile.c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hSourceFile) {
		return false;
	}


	hDestinationFile = CreateFile(encryptedFile.c_str(), FILE_WRITE_DATA, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hDestinationFile) {
		return false;
	}
	const int MAX_SIZE = 1024;
	char buffer[MAX_SIZE + 1] = "";
	DWORD dwBlockLen = 66;
	DWORD dwCount = 0;


	std::string randomStr = gen_random(40);
	char key[41] = "";
	char enKey[41] = "";
	for (int i = 0; i < 40; ++i) {
		enKey[i] = randomStr[i] - 30;
		key[i] = randomStr[i];
	}
	if (!WriteFile(hDestinationFile, enKey, 40, &dwCount, NULL)) {
		return false;
	}
	mix(key, 40);
	std::string password = key;

	for (bool eof = false; !eof; ) {
		memset(buffer, 0, MAX_SIZE + 1);
		if (!ReadFile(hSourceFile, buffer, dwBlockLen, &dwCount, NULL)) {
			return false;
		}
		eof = dwCount < dwBlockLen;
		std::string source = buffer;
		std::string encrpted;
		EncryptString(source, key, encrpted);
		int size = encrpted.size() / 16;
		if (!WriteFile(hDestinationFile, &size, sizeof(size), &dwCount, NULL)) {
			return false;
		}

		if (!WriteFile(hDestinationFile, encrpted.data(), encrpted.size(), &dwCount, NULL)) {
			return false;
		}
		dwBlockLen += 22;
		if (dwBlockLen > MAX_SIZE) {
			dwBlockLen = MAX_SIZE;
		}
	}

	return true;
}

std::optional<std::string> SecureFile::Decrypt(TString const& encryptedFile) {
	DFile(hSourceFile);

	hSourceFile = CreateFile(encryptedFile.c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hSourceFile) {
		return {};
	}

	const int MAX_SIZE = 1024;
	char buffer[MAX_SIZE + 1] = "";
	DWORD dwCount = 0;


	char key[41] = "";
	if (!ReadFile(hSourceFile, key, 40, &dwCount, NULL)) {
		return {};
	}
	for (int i = 0; i < 40; ++i) {
		key[i] = key[i] + 30;
	}

	mix(key, 40);
	std::string password = key;

	std::stringstream ss;
	for (bool eof = false; !eof; ) {
		memset(buffer, 0, MAX_SIZE + 1);
		int size = 0;
		if (!ReadFile(hSourceFile, buffer, sizeof(size), &dwCount, NULL)) {
			return {};
		}
		memcpy(&size, buffer, sizeof(size));
		auto error = GetLastError();
		size *= 16;
		if (dwCount == 0) {
			eof = true;
			continue;
		}
		if (!ReadFile(hSourceFile, buffer, size, &dwCount, NULL)) {
			return {};
		}
		std::string source;
		std::string decrpted;
		for (int i = 0; i < dwCount; ++i) {
			source.push_back(buffer[i]);
		}
		auto eee = DecryptString(source, password, decrpted);
		ss << decrpted.c_str();
	}
	return ss.str();
}

std::optional<std::string> SecureFile::EncryptLicense(std::string const& id, int days) {

	int year = days / 100 + 2020;
	int month = (days / 10) % 10;
	int day = days % 10;
	auto license = fmt::format("{}={}-{:02d}-{:02d}", id, year, month, day);
	SecureFile sf;
	std::string encryptedLicense;
	auto result = sf.EncryptString(license, id, encryptedLicense);
	if (result) {
		auto base64 = base64_encode((const unsigned char*)encryptedLicense.data(), encryptedLicense.size());

		return fmt::format("{}:{}", id, base64);
	}
	return {};
}

bool SecureFile::DecryptLicense(std::string const& license, std::string& out_id, int& out_days) {
	auto strs = split(license, ":");
	if (strs.size() != 2) {
		return false;
	}
	SecureFile sf;
	std::string out;
	auto encryptedLicense = base64_decode(strs[1]);
	bool result = sf.DecryptString(encryptedLicense, strs[0], out);
	if (result) {
		auto ll = split(out, "=");
		if (ll.size() != 2) {
			return false;
		}
		if (strs[0] != ll[0]) {
			return false;
		}
		auto date = split(ll[1], "-");
		int year = stoi(date[0]);
		int month = stoi(date[1]);
		int day = stoi(date[2]);
		out_id = ll[0];
		out_days = (year - 2020) * 100 + month * 10 + day;
		return true;
	}
	return false;
}
