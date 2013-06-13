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


#include "fbTga.h"

////////////////////////////////////////////
//              fbTga  class              //
////////////////////////////////////////////

fbTga::fbTga()
{
	m_data        = NULL;
	m_header      = NULL;
	m_size        = 0;
	m_workdir     = m_file = "";
}


fbTga::fbTga(const string& workdir)
{
	m_data        = NULL;
	m_header      = NULL;
	m_size        = 0;
	m_workdir     = workdir;
	m_file        = "";
}

fbTga::fbTga(const string& workdir, const string& file)
{
	m_data        = NULL;
	m_header      = NULL;
	m_size        = 0;
	m_workdir     = workdir;
	m_file        = file;
	Load();
}

fbTga::fbTga(const char* workdir)
{
	m_data        = NULL;
	m_header      = NULL;
	m_size        = 0;
	m_workdir     = workdir ? workdir : "";
	m_file        = "";
}

fbTga::fbTga(const char* workdir, const char* file)
{
	m_data        = NULL;
	m_header      = NULL;
	m_size        = 0;
	m_workdir     = workdir ? workdir : "";
	m_file        = file ? file : "";
	Load();
}

fbTga::~fbTga()
{
	Clear();
}

bool
fbTga::Load(const char* file, const uint16_t frames)
{
	bool ret = false;
	if (file && strlen(file))
	{
		string _file = file;
		ret = Load(_file, frames);
	}
	return ret;
}

bool
fbTga::Load(const string& file, const uint16_t frames)
{
	bool ret = false;
	if (!file.empty())
	{
		if (!m_data || m_file.compare(file) != 0)
		{
			m_file = file;
			if (!m_workdir.empty())
			{
				ret = Load(frames);
			}
		}
		else
		{
			ret = true;
		}
	}
	return ret;
}

bool
fbTga::Load(const uint16_t frames)
{
	bool ret = false;
	if (!m_workdir.empty() && !m_file.empty())
	{
		string file = m_workdir;
		file.append(C_DIR_SEPARATOR);
		file.append(m_file);
		FILE* fp;
		if (NULL != (fp = fopen(file.c_str(), "rb")))
		{
			fseek (fp, 0 , SEEK_END);
			int32_t size = ftell(fp);
			if (size > 0)
			{
				m_size = size;
				Clear();
				m_data = new uint8_t[m_size];
				rewind(fp);
				if(fread(m_data, 1, m_size, fp)); /* avoid warning about ignored return value */
				m_header = (stTgaHeader*)m_data;
				SetFrames(frames);
				ret = true;
			}
			fclose(fp);
		}
	}
	return ret;
}

void
fbTga::Clear()
{
	if (m_data != NULL)
	{
		delete [] m_data;
		m_header = NULL;
		m_data   = NULL;
	}
}

bool
fbTga::Empty()
{
	return m_data == NULL;
}

void
fbTga::SetWorkdir(const string& workdir)
{
	if (!workdir.empty())
	{
		m_workdir = workdir;
	}
}

void
fbTga::SetWorkdir(const char* workdir)
{
	if (workdir && strlen(workdir))
	{
		m_workdir = workdir;
	}
}

void
fbTga::SetFrames(const uint16_t frames)
{
	if (frames && m_header)
	{
		if ((m_header->imagespec.descriptor & (1 << 5)) == 0)
		{
			m_frame.first = frames - 1;
			m_frame.last  = 0;
		}
		else
		{
			m_frame.first = 0;
			m_frame.last  = frames - 1;
		}
		m_frame.count  = frames;
		m_frame.width  = m_header->imagespec.width;
		m_frame.height = m_header->imagespec.height / frames;
		m_frame.size   = m_frame.width * m_frame.height * (m_header->imagespec.bpp >> 3);
	}
}

void
fbTga::SetFile(const string& file)
{
	if (!file.empty())
	{
		m_file = file;
	}
}

void
fbTga::SetFile(const char* file)
{
	if (file && strlen(file))
	{
		m_file = file;
	}
}

string&
fbTga::GetFile()
{
	return m_file;
}

string&
fbTga::GetWorkdir()
{
	return m_workdir;
}

uint16_t
fbTga::GetWidth()
{
	return m_header ? m_header->imagespec.width : 0;
}

uint16_t
fbTga::GetHeight()
{
	return m_header ? m_header->imagespec.height : 0;
}

uint32_t
fbTga::Size()
{
	return m_size;
}

uint16_t
fbTga::GetFrameFirst()
{
	return m_frame.first;
}

uint16_t
fbTga::GetFrameLast()
{
	return m_frame.last;
}

uint16_t
fbTga::GetFrameCount()
{
	return m_frame.count;
}

uint16_t
fbTga::GetFrameWidth()
{
	return m_frame.width;
}

uint16_t
fbTga::GetFrameHeight()
{
	return m_frame.height;
}

uint32_t
fbTga::GetFrameSize()
{
	return m_frame.size;
}

void
fbTga::PutPixel(const fbGraphic* pGraphic, uint32_t x, uint32_t y, uint32_t color)
{
	uint32_t bpp = const_cast<fbGraphic*>(pGraphic)->GetBytesPerPixel();
	uint32_t bpr = const_cast<fbGraphic*>(pGraphic)->GetLineSize();
	if (bpr > 0)
	{
		if (bpp == 2)
		{
			uint16_t* fb_p = (uint16_t*)const_cast<fbGraphic*>(pGraphic)->GetFb();
			if (fb_p)
			{
				fb_p += x + (y * bpr / 2);
				uint16_t out = (color >> 3) & 0x1f;		// b
				out |= ((color >> 10) & 0x3f) << 5;		// g
				out |= ((color >> 19) & 0x1f) << 11;		// r
				*fb_p = out;
			}
		}
		else if (bpp == 3 || bpp == 4)
		{
			uint8_t* fb_p = (uint8_t*)const_cast<fbGraphic*>(pGraphic)->GetFb();
			if (fb_p)
			{
				fb_p += (x * bpp) + (y * bpr);
				*(fb_p++) = color & 0xff;		// b
				*(fb_p++) = (color >> 8) & 0xff;	// g
				*(fb_p++) = (color >> 16) & 0xff;	// r
				if (bpp == 4)
				{
					*(fb_p++) = (0xff );		// a
				}
			}
		}
	}
}

uint32_t
fbTga::DecodeByte(const uint8_t* pByte)
{
	uint32_t res = 0;
	if (pByte)
	{
		const uint8_t* in = pByte;
		if (m_header->imagespec.bpp == 32)
		{
			if (in[3] != 0)
			{
				res = (in[3] << 24 | in[2] << 16 | in[1] << 8 | in[0]);
			}
		}
		else if (m_header->imagespec.bpp == 24)
		{
			res = (0xff000000 | in[2] << 16 | in[1] << 8 | in[0]);
		}
	}
	return res;
}

bool
fbTga::Draw(const fbGraphic* pGraphic, uint32_t x, uint32_t y, uint16_t frameOffset)
{
	bool res = false;
	if (pGraphic)
	{
		if (m_data && m_header)
		{
			/* do some sanity checks */
			if ((m_header->imagetype == C_IMAGE_TYPE_UNCOMPRESSED_TRUECOLOR || m_header->imagetype == C_IMAGE_TYPE_RLE_TRUECOLOR) && 
			   (m_header->imagespec.bpp == 32 || m_header->imagespec.bpp == 24) && m_header->colormaptype == 0)
			{
				uint32_t pos    = 0;
				uint32_t step   = (m_header->imagespec.bpp >> 3);
				uint16_t height = m_frame.height;
				uint16_t offset = frameOffset < m_frame.count ? frameOffset : frameOffset % m_frame.count;
				const uint8_t* imagestart = ((const uint8_t*)m_data + sizeof(stTgaHeader) + m_header->idlength + offset * m_frame.size);
				if (m_header->imagetype == C_IMAGE_TYPE_UNCOMPRESSED_TRUECOLOR)
				{
					/* no RLE */
					for (uint32_t srcY = 0; srcY < height; ++srcY)
					{
						uint32_t dstY;
						if ((m_header->imagespec.descriptor & (1 << 5)) == 0)
						{
							dstY = (height - 1) - srcY;
						}
						else
						{
							dstY = srcY;
						}
						for (uint32_t srcX = 0; srcX < m_header->imagespec.width; ++srcX)
						{
							PutPixel(pGraphic, x + srcX, y + dstY, DecodeByte((const uint8_t*)imagestart + pos));
							pos += step;
						}
					}
				}
				else if (m_header->imagetype == C_IMAGE_TYPE_RLE_TRUECOLOR)
				{
					/* RLE compression */
					uint32_t count = 0;
					uint32_t srcX  = 0;
					uint32_t srcY  = ((m_header->imagespec.descriptor & (1 << 5)) == 0) ? height - 1 : 0;
					uint32_t totalPixels = height * m_header->imagespec.width;
					while (count < totalPixels)
					{
						uint8_t  run = *((const uint8_t*)imagestart + pos);
						uint32_t runlen = (run & 0x7f) + 1;
						// consume the run byte
						++pos;
						// start of a run
						for (uint32_t runpos = 0; runpos < runlen; ++runpos)
						{
							PutPixel(pGraphic, x + srcX, y + srcY, DecodeByte((const uint8_t*)imagestart + pos));
							++count;
							++srcX;
							if (srcX == m_header->imagespec.width)
							{
								if ((m_header->imagespec.descriptor & (1 << 5)) == 0)
								{
									--srcY;
								}
								else
								{
									++srcY;
								}
								srcX = 0;
							}
							//if a run of raw pixels, consume an input pixel
							if ((run & 0x80) == 0)
							{
								pos += step;
							}
						}
						// if a run of repeated pixels, consume pixel we repeated
						if (run & 0x80)
						{
							pos += step;
						}
					}
				}
				res = true;
			}
		}
	}
	return res;
}

uint8_t
fbTga::GetImageType()
{
	uint8_t res = C_IMAGE_TYPE_NODATA;
	if (m_data && m_header)
	{
		res = m_header->imagetype;
	}
	return res;
}

bool
fbTga::DecodeRLE()
{
	bool ret = true;
	if (m_data && m_header)
	{
		/* do some sanity checks */
		if (m_header->imagetype == C_IMAGE_TYPE_RLE_TRUECOLOR)
		{
			if ((m_header->imagespec.bpp == 32 || m_header->imagespec.bpp == 24) && m_header->colormaptype == 0)
			{
				uint8_t  runlen, runpos;
				uint32_t count = 0;
				uint8_t  pixelDepth  = (m_header->imagespec.bpp >> 3);
				uint32_t headerSize  = sizeof(stTgaHeader) + m_header->idlength;
				uint32_t rawDataSize = m_header->imagespec.width * m_header->imagespec.height * pixelDepth;
				uint8_t* pRawBuffer  = new uint8_t[headerSize + rawDataSize];
				uint8_t* p           = pRawBuffer + headerSize;
				memcpy(pRawBuffer, (const uint8_t*)m_data, headerSize);
				const uint8_t* pRleData = (const uint8_t*)m_data + headerSize;
				while (count < rawDataSize)
				{
					if(*pRleData & 0x80)				// Run length chunk (High bit = 1)
					{
						runlen = (*pRleData & 0x7f) + 1;	// Get run length
						++pRleData;				// Move to pixel data
						// Repeat the next pixel runlen times
						for(runpos = 0; runpos < runlen; ++runpos, count += pixelDepth)
						{
							memcpy(&p[count], pRleData, pixelDepth);
						}
						pRleData += pixelDepth;			// Move to the next descriptor chunk
					}
					else						// Raw chunk
					{
						runlen = *pRleData + 1;			// Get run length
						++pRleData;				// Move to pixel data
						// Write the next runlen pixels directly
						for(runpos = 0; runpos < runlen; ++runpos, count += pixelDepth, pRleData += pixelDepth)
						{
							memcpy(&p[count], pRleData, pixelDepth);
						}
					}
				}
				((stTgaHeader*)pRawBuffer)->imagetype = C_IMAGE_TYPE_UNCOMPRESSED_TRUECOLOR;
				delete [] m_data;
				m_data   = pRawBuffer;
				m_header = (stTgaHeader*)m_data;
				m_size   = headerSize + rawDataSize;
			}
			else
			{
				ret = false;
			}
		}
		else
		{
			ret = m_header->imagetype == C_IMAGE_TYPE_UNCOMPRESSED_TRUECOLOR;
		}
	}
	else
	{
		ret = false;
	}
	return ret;
}
