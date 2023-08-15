/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "Utils.h"
#include "SslCredentials.h"

#include <unordered_map>
#include "../ThirdParty/HmacSha256/hmac_sha256.h"


std::string Inworld::Utils::GetSslRootCerts()
{
    std::string SslRootCerts;

	for (auto& Str : SslRootsFileContents)
	{
        SslRootCerts += Str;
	}

    return SslRootCerts;
}

std::string Inworld::Utils::PhonemeToViseme(const std::string& Phoneme)
{
    static std::unordered_map<std::string, std::string> PhonemeToVisemeMap = {
        // Consonants
        {"b" , "PP"}, {"d" , "DD"}, {"d\u0361\u0292" , "CH"}, {"\xF0" , "TH"}, {"f" , "FF"}, {"\u0261" , "Kk"},
        {"h" , "Kk"}, {"j" , "I"}, {"k" , "Kk"}, {"" , "Nn"}, {"m" , "PP"}, {"n" , "Nn"},
        {"\u014B" , "Kk"}, {"p" , "PP"}, {"\u0279" , "RR"}, {"s" , "SS"}, {"\u0283" , "CH"}, {"t" , "DD"},
        {"t\u0361\u0283" , "CH"}, {"\u03B8" , "TH"}, {"v" , "FF"}, {"w" , "U"}, {"z" , "SS"}, {"\u0292" , "CH"},
        // Vowels
        {"\u0259" , "E"}, {"\u025A" , "E"}, {"\xE6" , "Aa"}, {"a\u026A" , "Aa"}, {"a\u028A" , "Aa"}, {"\u0251" , "Aa"},
        {"e\u026A" , "E"}, {"\u025D" , "E"}, {"\u025B" , "E"}, {"i" , "I"}, {"\u026A" , "I"}, {"o\u028A" , "O"},
        {"\u0254" , "O"}, {"\u0254\u026A", "O"}, {"u" , "U"}, {"\u028A" , "U"}, {"\u028C" , "E"},
        //Additional Symbols
        {"\u02C8" , "STOP"}, {"\u02CC" , "STOP"}, {"." , "STOP"},
    };
    if (PhonemeToVisemeMap.find(Phoneme) != PhonemeToVisemeMap.end())
    {
        return PhonemeToVisemeMap[Phoneme];
    }
    return "";
}

std::vector<uint8_t> Inworld::Utils::HmacSha256(const std::vector<uint8_t>& Data, const std::vector<uint8_t>& Key)
{
	std::vector<uint8_t> Res(32);
	hmac_sha256(Key.data(), Key.size(), Data.data(), Data.size(), Res.data(), Res.size());
	return Res;
}

std::string Inworld::Utils::ToHex(const std::vector<uint8_t>& Data)
{
	std::string Res(Data.size() * 2, '0');
	for (int i = 0; i < Data.size(); i++)
	{
		sprintf((char*)(Res.data()) + (i * 2), "%02x", Data[i]);
	}

	return Res;
}

