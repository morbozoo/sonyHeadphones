/*
Thomas Sanchez Lengeling
Help functions to convert from utf16 to utf8
*/

#pragma once

#include <codecvt>
#include <string>

namespace utf8{

	static std::string Utf16ToUtf8(wchar_t chUtf16)
	{
		// From RFC 3629
		// 0000 0000-0000 007F   0xxxxxxx
		// 0000 0080-0000 07FF   110xxxxx 10xxxxxx
		// 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx

		// max output length is 3 bytes (plus one for Nul)
		unsigned char szUtf8[4] = "";

		if (chUtf16 < 0x80)
		{
			szUtf8[0] = static_cast<unsigned char>(chUtf16);
		}
		else if (chUtf16 < 0x7FF)
		{
			szUtf8[0] = static_cast<unsigned char>(0xC0 | ((chUtf16 >> 6) & 0x1F));
			szUtf8[1] = static_cast<unsigned char>(0x80 | (chUtf16 & 0x3F));
		}
		else
		{
			szUtf8[0] = static_cast<unsigned char>(0xE0 | ((chUtf16 >> 12) & 0xF));
			szUtf8[1] = static_cast<unsigned char>(0x80 | ((chUtf16 >> 6) & 0x3F));
			szUtf8[2] = static_cast<unsigned char>(0x80 | (chUtf16 & 0x3F));
		}

		return reinterpret_cast<char *>(szUtf8);
	}

	static std::string Utf16ToUtf8(const std::wstring &sUtf16)
	{
		std::string sUtf8;
		std::wstring::const_iterator itr;

		for (itr = sUtf16.begin(); itr != sUtf16.end(); ++itr)
			sUtf8 += Utf16ToUtf8(*itr);
		return sUtf8;
	}


	static std::wstring stringToWString(const std::string & str){

			std::wstring result;
			result.resize(str.length());
			mbstowcs(&result[0], &str[0], str.length());
			return result;
	}
}