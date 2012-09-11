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


#include "fbImageStorage.h"

////////////////////////////////////////////
//             fbIndexItem class          //
////////////////////////////////////////////

fbIndexItem::fbIndexItem()
{
	m_type = E_UNKNOWN;
	m_id = m_name = m_container = m_language = m_country = "";
}

fbIndexItem::fbIndexItem(const stImageProp& prop)
{
	m_type = E_UNKNOWN;
	m_language  = m_country = "";
	SetProperties(prop.id.c_str());
	SetName(prop.name.c_str());
	SetContainer(prop.container.c_str());
}

fbIndexItem::fbIndexItem(const stImageProp* prop)
{
	m_type = E_UNKNOWN;
	m_id = m_name = m_container = m_language = m_country = "";
	if (prop)
	{
		SetProperties(prop->id.c_str());
		SetName(prop->name.c_str());
		SetContainer(prop->container.c_str());
	}
}

void
fbIndexItem::SetProperties(const string& id)
{
	if (!id.empty())
	{
		SetProperties(id.c_str());
	}
}

void
fbIndexItem::SetProperties(const char* id)
{
	if (id && strlen(id))
	{
		m_type = E_UNKNOWN;
		m_id = id;
		string language;
		string country;
		if (0 == m_id.compare(C_ANIM_EXTENSION))
		{
			m_type = E_ANIM;
		}
		else if (true == fbLocaleManager::ParseLocale(m_id, language, country))
		{
			m_type     = E_MSG;
			m_language = language;
			m_country  = country;
		}
	}
}

void
fbIndexItem::SetContainer(const string& container)
{
	if (!container.empty())
	{
		SetContainer(container.c_str());
	}
}

void
fbIndexItem::SetContainer(const char* container)
{
	if (container && strlen(container))
	{
		m_container = container;
	}
}

void
fbIndexItem::SetName(const string& name)
{
	if (!name.empty())
	{
		SetName(name.c_str());
	}
}

void
fbIndexItem::SetName(const char* name)
{
	if (name && strlen(name))
	{
		m_name = name;
	}
}

eImageType
fbIndexItem::GetType()
{
	return m_type;
}

string&
fbIndexItem::GetId()
{
	return m_id;
}

string&
fbIndexItem::GetLanguage()
{
	return m_language;
}

string&
fbIndexItem::GetCountry()
{
	return m_country;
}

string&
fbIndexItem::GetName()
{
	return m_name;
}

string&
fbIndexItem::GetContainer()
{
	return m_container;
}

////////////////////////////////////////////
//              fbIndex class             //
////////////////////////////////////////////

fbIndex::fbIndex(const stImageProp& image_prop)
{
	Insert(image_prop);
}

void
fbIndex::Insert(const stImageProp* image_prop)
{
	if (image_prop)
	{
		Insert(*image_prop);
	}
}

void
fbIndex::Insert(const stImageProp& image_prop)
{
	map<string, fbIndexItem>::iterator it;
	if ((it = m_index.find(image_prop.id)) == m_index.end())
	{
		fbIndexItem item(image_prop);
		if (item.GetType() != E_UNKNOWN)
		{
			m_index.insert(make_pair<string, fbIndexItem>(image_prop.id, item));
			if (item.GetType() == E_MSG)
			{
				m_langIndex.Insert(item.GetLanguage(), item.GetId());
				m_countryIndex.Insert(item.GetCountry(), item.GetId());
			}
			++(m_ref[item.GetType()]);
			m_typeIndex.Insert(item.GetType(), item.GetId());
		}
	}
}

void
fbIndex::Erase(const char* image_id)
{
	if (image_id && strlen(image_id))
	{
		string _image_id = image_id;
		Erase(_image_id);
	}
}

void
fbIndex::Erase(const string& image_id)
{
	map<string, fbIndexItem>::iterator it;
	if ((it = m_index.find(image_id)) != m_index.end())
	{
		map<eImageType,uint16_t>::iterator it_ref;
		if ((it_ref = m_ref.find(it->second.GetType())) != m_ref.end())
		{
			--(it_ref->second);
			if (it_ref->second == 0)
			{
				m_ref.erase(it_ref);
			}
		}
		m_index.erase(it);
	}
}

void
fbIndex::Clear()
{
	m_index.clear();
	m_ref.clear();
	m_langIndex.Clear();
	m_countryIndex.Clear();
	m_typeIndex.Clear();
}

bool
fbIndex::HasItem(const string& image_id)
{
	return m_index.find(image_id) != m_index.end();
}

bool
fbIndex::Empty()
{
	return m_index.empty();
}

uint16_t
fbIndex::Size()
{
	return m_index.size();
}

uint16_t
fbIndex::Size(eImageType type)
{
	uint16_t size = 0;
	map<eImageType,uint16_t>::iterator it;
	if ((it = m_ref.find(type)) != m_ref.end())
	{
		size = it->second;
	}
	return size;
}

map<string, fbIndexItem>::iterator
fbIndex::Begin()
{
	return m_index.begin();
}

map<string, fbIndexItem>::iterator
fbIndex::End()
{
	return m_index.end();
}

map<string, fbIndexItem>::iterator
fbIndex::GetItemById(const string& image_id)
{
	map<string, fbIndexItem>::iterator it = m_index.end();
	if (!image_id.empty())
	{
		it = m_index.find(image_id);
	}
	return it;
}

map<string, fbIndexItem>::iterator
fbIndex::GetItemById(const char* image_id)
{
	map<string, fbIndexItem>::iterator it = m_index.end();
	if (image_id && strlen(image_id))
	{
		string _image_id = image_id;
		it = GetItemById(_image_id);
	}
	return it;
}

map<string, fbIndexItem>::iterator
fbIndex::GetItemByLanguage(const string& language)
{
	map<string, fbIndexItem>::iterator it = m_index.end();
	if (m_langIndex.Size() && !language.empty())
	{
		set<string>::iterator it_ind;
		if ((it_ind = m_langIndex.GetItem(language)) != m_langIndex.End())
		{
			it = m_index.find(*it_ind);
		}
	}
	return it;
}

map<string, fbIndexItem>::iterator
fbIndex::GetItemByLanguage(const char* language)
{
	map<string, fbIndexItem>::iterator it = m_index.end();
	if (language && strlen(language))
	{
		string _language = language;
		it = GetItemByLanguage(_language);
	}
	return it;
}

map<string, fbIndexItem>::iterator
fbIndex::GetItemByCountry(const string& country)
{
	map<string, fbIndexItem>::iterator it = m_index.end();
	if (m_countryIndex.Size() && !country.empty())
	{
		set<string>::iterator it_ind;
		if ((it_ind = m_countryIndex.GetItem(country)) != m_countryIndex.End())
		{
			it = m_index.find(*it_ind);
		}
	}
	return it;
}

map<string, fbIndexItem>::iterator
fbIndex::GetItemByCountry(const char* country)
{
	map<string, fbIndexItem>::iterator it = m_index.end();
	if (country && strlen(country))
	{
		string _country = country;
		it = GetItemByLanguage(_country);
	}
	return it;
}

map<string, fbIndexItem>::iterator
fbIndex::GetItemByType(const eImageType type)
{
	map<string, fbIndexItem>::iterator it = m_index.end();
	if (m_typeIndex.Size())
	{
		set<string>::iterator it_ind;
		if ((it_ind = m_typeIndex.GetItem(type)) != m_typeIndex.End())
		{
			it = m_index.find(*it_ind);
		}
	}
	return it;
}

////////////////////////////////////////////
//         fbImageStorage class           //
////////////////////////////////////////////

fbImageStorage::fbImageStorage()
{
	m_workdir = "";
}

fbImageStorage::fbImageStorage(const string& workdir)
{
	m_workdir = workdir;
}

fbImageStorage::fbImageStorage(const char* workdir)
{
	m_workdir = workdir ? workdir : "";
}

fbImageStorage::~fbImageStorage()
{
	Clear();
}

void
fbImageStorage::CreateIndex()
{
	if (m_index.empty())
	{
		string images[]= {C_IMAGES_TGZ, C_COMMON_TGZ};
		int sz = sizeof(images)/sizeof(images[0]);
		for (int i = 0; i < sz; ++i)
		{
			string tgz = C_IMAGES_FOLDER;
			tgz.append(images[i]);
			if(FileExists(tgz.c_str()))
			{
				FILE *fp;
				const int buffSize = 128;
				char buffer[buffSize];
				string cmd = C_LIST_CMD;
				cmd.append(tgz);
				if((fp= popen(cmd.c_str(), "r")) != NULL)
				{
					while (fgets(buffer, buffSize, fp) != NULL)
					{
						char* p = strtok(buffer, "\n");
						while (p != NULL)
						{
							string index_id, image_id;
							stImageProp image_prop;
							string image = p;
							map<string, fbIndex>::iterator it;
							size_t pos1 = image.find(C_TGA_ARGS_DELIMITER);
							size_t pos2 = image.rfind(C_TGA_ARGS_DELIMITER);
							index_id    = image.substr(0, pos1);
							image_id    = image.substr(pos1 + 1, pos1 == pos2 ? string::npos : pos2 - 1);
							to_lower(index_id);
							to_lower(image_id);
							if ((it = m_index.find(index_id)) == m_index.end())
							{
								fbIndex index;
								pair<map<string, fbIndex>::iterator, bool> ret;
								ret = m_index.insert(make_pair<string,fbIndex>(index_id,index));
								it = ret.first;
							}
							image_prop.name      = image;
							image_prop.id        = image_id;
							image_prop.container = images[i];
							it->second.Insert(image_prop);
							p = strtok(NULL, "\n");
						}
					}
					pclose(fp);
				}
			}
		}
	}
}

bool
fbImageStorage::LoadImage(const stImageProp* prop, const eImageType type)
{
	bool res = false;
	if (prop)
	{
		res = LoadImage(*prop, type);
	}
	return res;
}

bool
fbImageStorage::LoadImage(const stImageProp& prop, const eImageType type)
{
	bool ret = false;
	if (!prop.container.empty() && !prop.name.empty() && !prop.id.empty())
	{
		if (m_data.find(prop.id) == m_data.end())
		{
			string s = C_EXTRACT_CMD;
			s.append(C_IMAGES_FOLDER), s.append(prop.container), s.append(" "), s.append(prop.name), s.append(" 1>/dev/null 2>&1");
			if(system(s.c_str())); /* avoid warning about ignoring return value */
			uint16_t frames = 1;
			fbTga* pImage   = new fbTga(m_workdir, prop.id);
			if (type == E_ANIM)
			{
				if (prop.id.compare(C_IMAGE_ANIMATION) == 0)
				{
					frames = C_ANIMATION_FRAMES;
				}
				else if (prop.id.compare(C_IMAGE_PROGRESS) == 0)
				{
					frames = C_PROGRESS_FRAMES;
				}
			}
			if ((ret = pImage->Load(prop.name, frames)) == true)
			{
				if (type == E_ANIM && pImage->GetImageType() == C_IMAGE_TYPE_RLE_TRUECOLOR)
				{
					ret = pImage->DecodeRLE();
				}
			}
			if (ret)
			{
				m_data.insert(make_pair<string,fbTga*>(prop.id, pImage));
			}
			else
			{
				delete pImage;
			}
			s = m_workdir, s.append(C_DIR_SEPARATOR), s.append(prop.name);
			remove(s.c_str());
		}
	}
	return ret;
}

void
fbImageStorage::LoadImages()
{
	stLoadSpec spec;
	LoadImages(spec);
}

void
fbImageStorage::LoadImages(const stLoadSpec* spec)
{
	if (spec)
	{
		LoadImages(*spec);
	}
	else
	{
		LoadImages();
	}
}

void
fbImageStorage::LoadImages(const stLoadSpec& spec)
{
	if (!m_index.empty())
	{
		map<string, fbIndex>::iterator it;
		for (it = m_index.begin(); it != m_index.end(); ++it)
		{
			map<string, fbIndexItem>::iterator it_image;
			if (spec.loadprogress || it->first.compare(C_IMAGE_PROGRESS) != 0)
			{
				if (it->second.Size(E_ANIM))
				{
					if ((it_image = it->second.GetItemByType(E_ANIM)) != it->second.End())
					{
						stImageProp prop;
						prop.name      = it_image->second.GetName();
						prop.container = it_image->second.GetContainer();
						prop.id        = it->first;
						if (LoadImage(prop, E_ANIM) == true)
						{
							continue;
						}
					}
				}
			}
			if (spec.loadmessage && (spec.message == NULL || it->first.compare(spec.message) == 0))
			{
				if (it->second.Size(E_MSG))
				{
					if ((it_image = it->second.GetItemById(m_locale.GetLocale())) == it->second.End())
					{
						if ((it_image = it->second.GetItemByLanguage(m_locale.GetLanguage())) == it->second.End() && 0 != strcmp(m_locale.GetLocale(),m_locale.GetDefaultLocale()))
						{
							if ((it_image = it->second.GetItemById(m_locale.GetDefaultLocale())) == it->second.End() && 0 != strcmp(m_locale.GetLanguage(),m_locale.GetDefaultLanguage()))
							{
								it_image = it->second.GetItemByLanguage(m_locale.GetDefaultLanguage());
							}
						}
					}
					if (it_image != it->second.End())
					{
						stImageProp prop;
						prop.name      = it_image->second.GetName();
						prop.container = it_image->second.GetContainer();
						prop.id        = it->first;
						LoadImage(prop, E_MSG);
					}
				}
			}
		}
	}
}

void
fbImageStorage::SetWorkdir(const string& workdir)
{
	if (!workdir.empty())
	{
		m_workdir = workdir;
	}
}

void
fbImageStorage::SetWorkdir(const char* workdir)
{
	if (workdir && strlen(workdir))
	{
		m_workdir = workdir;
	}
}

void
fbImageStorage::Erase(const string& item)
{
	map<string, fbTga*>::iterator it;
	if ((it = m_data.find(item)) != m_data.end())
	{
		delete it->second;
		m_data.erase(it);
	}
}

bool
fbImageStorage::HasItem(const string& item)
{
	return m_data.find(item) != m_data.end();
}

void
fbImageStorage::Clear()
{
	map<string, fbTga*>::iterator it;
	for (it = m_data.begin(); it != m_data.end(); ++it)
	{
		delete it->second;
	}
	m_data.clear();
	m_index.clear();
}

bool
fbImageStorage::Empty()
{
	return m_data.empty();
}

uint16_t
fbImageStorage::Size()
{
	return m_data.size();
}

map<string, fbTga*>::iterator
fbImageStorage::Begin()
{
	return m_data.begin();
}

map<string, fbTga*>::iterator
fbImageStorage::End()
{
	return m_data.end();
}

map<string, fbTga*>::iterator
fbImageStorage::Find(const char* image)
{
	map<string, fbTga*>::iterator it = m_data.end();
	if (image && strlen(image))
	{
		string _image = image;
		it = m_data.find(_image);
	}
	return it;
}

map<string, fbTga*>::iterator
fbImageStorage::Find(const string& image)
{
	return m_data.find(image);
}

