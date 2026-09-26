#pragma once

#include <tchar.h>
#include <string>
#include <optional>

class SecureFile {
	using TString = std::basic_string<TCHAR>;
public:
	bool Encrypt(TString const& plainFile, TString const& encryptedFile);

	std::optional<std::string> Decrypt(TString const& encryptedFile);

	std::optional<std::string> EncryptLicense(std::string const& id, int days);
	bool DecryptLicense(std::string const& license, std::string& out_id, int& out_days);


	bool EncryptString(std::string const& content, std::string const& password, std::string& outEncrypted);
	bool DecryptString(std::string const& encryptedContent, std::string const& password, std::string& outDecrypted);
};
