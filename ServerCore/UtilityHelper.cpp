#include "pch.h"
#include "UtilityHelper.h"
#include <filesystem>
#include <fstream>

Vector<BYTE> UtilityHelper::ReadFile(const WCHAR* path)
{
	Vector<BYTE> data;
	std::filesystem::path filePath{ path };

	const uint32 fileSize = static_cast<uint32>(std::filesystem::file_size(filePath));
	data.resize(fileSize);

	std::basic_ifstream<BYTE> inputfStream{ filePath };
	inputfStream.read(&data[0], fileSize);

	return data;
}

std::wstring UtilityHelper::ConvertUTF8ToUnicode(std::string str)
{
	const int32 strLen = static_cast<int32>(str.size());

	std::wstring wstr;
	if (strLen == 0)
		return wstr;

	const int32 wstrLen = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), strLen, NULL, 0);
	wstr.resize(wstrLen);
	::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), strLen, &wstr[0], wstrLen);

	return wstr;
}

std::string UtilityHelper::ConvertUnicodeToUTF8(std::wstring wstr)
{
	const int32 wstrLen = static_cast<int32>(wstr.size());

	std::string str;
	if (wstrLen == 0)
		return str;

	const int32 strLen = ::WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<WCHAR*>(&wstr[0]), wstrLen, NULL, 0, NULL, NULL);
	str.resize(strLen);
	::WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<WCHAR*>(&wstr[0]), wstrLen, &str[0], strLen, NULL, NULL);

	return str;
}
