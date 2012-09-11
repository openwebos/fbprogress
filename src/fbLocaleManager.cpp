// @@@LICENSE
//
//      Copyright (c) 2007-2012 Hewlett-Packard Development Company, L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LICENSE@@@


#include "fbLocaleManager.h"

////////////////////////////////////////////
//         fbLocaleManager class          //
////////////////////////////////////////////

fbLocaleManager::fbLocaleManager()
{
	Init();
}

void
fbLocaleManager::Init()
{
	m_language = C_DEFAULT_LANGUAGE;
	m_country  = C_DEFAULT_COUNTRY;
	to_lower(m_language);
	to_lower(m_country);
	SetLocale(NULL, NULL);
	m_default = m_locale;
	ReadSystemSettings();
}

void
fbLocaleManager::ReadSystemSettings()
{
	string systemSettings = C_SYSTEM_SETTINGS;
	if(!FileExists(systemSettings.c_str()))
	{
		systemSettings  = C_ROOTFS;
		systemSettings.append(C_SYSTEM_SETTINGS);
		if(!FileExists(systemSettings.c_str()))
		{
			systemSettings.erase();
		}
	}
	if(!systemSettings.empty())
	{
		FILE* fp;
		if ((fp = fopen(systemSettings.c_str(), "r")) != NULL)
		{
			string language;
			string country;
			uint16_t bufSize = 80;
			char buffer[bufSize];
			while (fgets(buffer, bufSize, fp) != NULL && (country.empty() || language.empty()))
			{
				char* p = strrchr(buffer, '\n');
				if (p)
				{
					*p = '\0';
				}
				if (strlen(buffer))
				{
					if (language.empty() && strstr(buffer, C_LOCALE_LANGUAGE_PATTERN) == buffer)
					{
						language = buffer + strlen(C_LOCALE_LANGUAGE_PATTERN);
						language = trim(language);
					}
					else if (country.empty() && strstr(buffer, C_LOCALE_COUNTRY_PATTERN) == buffer)
					{
						country = buffer + strlen(C_LOCALE_COUNTRY_PATTERN);
						country = trim(country);
					}
				}
			}
			if (!language.empty())
			{
				m_language = to_lower(language);
			}
			if (!country.empty())
			{
				m_country = to_lower(country);
			}
			SetLocale(NULL, NULL);
			fclose(fp);
		}
	}
}

void
fbLocaleManager::SetLanguage(string &language)
{
	SetLanguage(language.c_str());
}

void
fbLocaleManager::SetLanguage(const char* language)
{
	if (language && strlen(language))
	{
		SetLocale(language, NULL);
	}
}

const char*
fbLocaleManager::GetDefaultLanguage()
{
	return C_DEFAULT_LANGUAGE;
}

const char*
fbLocaleManager::GetLanguage()
{
	return m_language.c_str();
}

void
fbLocaleManager::SetCountry(string &country)
{
	SetCountry(country.c_str());
}

void
fbLocaleManager::SetCountry(const char* country)
{
	if (country && strlen(country))
	{
		SetLocale(NULL, country);
	}
}

const char*
fbLocaleManager::GetDefaultCountry()
{
	return C_DEFAULT_COUNTRY;
}

const char*
fbLocaleManager::GetCountry()
{
	return m_country.c_str();
}

void
fbLocaleManager::SetLocale(string& language, string& country)
{
	SetLocale(language.c_str(), country.c_str());
}

void
fbLocaleManager::SetLocale(const char* language, const char* country)
{
	if (language && strlen(language))
	{
		m_language = language;
	}
	if (country && strlen(country))
	{
		m_country  = country;
	}
	m_locale = m_language;
	m_locale.append(C_LOCALE_DELIMITER);
	m_locale.append(m_country);
}

const char*
fbLocaleManager::GetDefaultLocale()
{
	return m_default.c_str();
}

const char*
fbLocaleManager::GetLocale()
{
	return m_locale.c_str();
}

bool
fbLocaleManager::ParseLocale(const string& locale, string& language, string& country)
{
	bool ret = false;
	size_t pos = locale.find(C_LOCALE_DELIMITER);
	if (pos != string::npos && pos > 0 && pos < locale.size() - 1)
	{
		string _language = locale.substr(0, pos);
		string _country  = locale.substr(pos + 1);
		if (!_language.empty() && !_country.empty())
		{
			language = to_lower(_language);
			country  = to_lower(_country);
			ret = true;
		}
	}
	return ret;
}
