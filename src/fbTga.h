// @@@LICENSE
//
//      Copyright (c) 2007-2013 LG Electronics, Inc.
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


#ifndef __TGA_H__
#define __TGA_H__

#include "fbCommon.h"
#include "fbGraphic.h"

static const uint8_t C_IMAGE_TYPE_NODATA                   = 0;
static const uint8_t C_IMAGE_TYPE_UNCOMPRESSED_COLORMAPPED = 1;
static const uint8_t C_IMAGE_TYPE_UNCOMPRESSED_TRUECOLOR   = 2;
static const uint8_t C_IMAGE_TYPE_UNCOMPRESSED_BLACKWHITE  = 3;
static const uint8_t C_IMAGE_TYPE_RLE_COLORMAPPED          = 9;
static const uint8_t C_IMAGE_TYPE_RLE_TRUECOLOR            = 10;
static const uint8_t C_IMAGE_TYPE_RLE_BLACKWHITE           = 11;

struct stColorMapSpec
{
	uint16_t offset;
	uint16_t length;
	uint8_t  bpp;
	stColorMapSpec(): offset(0), length(0), bpp(0) {}
} __attribute__((packed));

struct stImageSpec
{
	uint16_t x_origin;
	uint16_t y_origin;
	uint16_t width;
	uint16_t height;
	uint8_t  bpp;
	uint8_t  descriptor;
	stImageSpec(): x_origin(0), y_origin(0), width(0), height(0), bpp(0), descriptor(0) {}
} __attribute__((packed));

struct stTgaHeader
{
	uint8_t        idlength;
	uint8_t        colormaptype;
	uint8_t        imagetype;
	stColorMapSpec colormapspec;
	stImageSpec    imagespec;
	stTgaHeader(): idlength(0), colormaptype(0), imagetype(0) {}
} __attribute__((packed));

struct stFrameInfo
{
	uint16_t first;
	uint16_t last;
	uint16_t count;
	uint16_t width;
	uint16_t height;
	uint32_t size;
	stFrameInfo(): first(0), last(0), count(1), width(0), height(0), size(0) {}
};

class fbTga
{
private:
	string       m_workdir;
	string       m_file;
	stTgaHeader* m_header;
	stFrameInfo  m_frame;
	uint32_t     m_size;
	uint8_t*     m_data;
	void         PutPixel(const fbGraphic* pGraphic, uint32_t x, uint32_t y, uint32_t color);
	uint32_t     DecodeByte(const uint8_t* pByte);
public:
	fbTga();
	fbTga(const string& workdir);
	fbTga(const string& workdir, const string& file);
	fbTga(const char* workdir);
	fbTga(const char* workdir, const char* file);
	~fbTga();
	void SetFile(const string& file);
	void SetFile(const char* file);
	void SetWorkdir(const string& workdir);
	void SetWorkdir(const char* workdir);
	void SetFrames(const uint16_t frames);
	void Clear();
	bool Load(const uint16_t frames = 1);
	bool Load(const string& file, const uint16_t frames = 1);
	bool Load(const char* file, const uint16_t frames = 1);
	bool Draw(const fbGraphic* pGraphic, uint32_t x, uint32_t y, uint16_t frameOffset = 0);
	bool DecodeRLE();
	bool Empty();
	string&  GetWorkdir();
	string&  GetFile();
	uint8_t  GetImageType();
	uint16_t GetWidth();
	uint16_t GetHeight();
	uint16_t GetFrameFirst();
	uint16_t GetFrameLast();
	uint16_t GetFrameCount();
	uint16_t GetFrameWidth();
	uint16_t GetFrameHeight();
	uint32_t GetFrameSize();
	uint32_t Size();
};

#endif
