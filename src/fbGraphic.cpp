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


#include "fbGraphic.h"

////////////////////////////////////////////
//            fbGraphic class             //
////////////////////////////////////////////

fbGraphic::fbGraphic()
{
	m_init           = false;
	if (OpenFb() > -1)
	{
		Init();
	}
}

fbGraphic::~fbGraphic()
{
	if (IsFbOpen())
	{
		DeInit();
		CloseFb();
	}
}

void
fbGraphic::Init()
{
	if (IsFbOpen() && !m_init)
	{
		if (GetFixScreenInfo() == 0)
		{
			if (m_fb_fix.type == FB_TYPE_PACKED_PIXELS)
			{
				if (GetVarScreenInfo() == 0)
				{
					m_init = true;
					uint8_t* p;
					uint32_t mem_offset = (uint32_t)(m_fb_fix.smem_start) & (~PAGE_MASK);
					if (m_fb_var.xres && m_fb_var.xres_virtual > m_fb_var.xres)
					{
						m_fb.x_ratio = m_fb_var.xres_virtual / m_fb_var.xres;
					}
					if (m_fb_var.yres && m_fb_var.yres_virtual > m_fb_var.yres)
					{
						m_fb.y_ratio = m_fb_var.yres_virtual / m_fb_var.yres;
					}
					if ((p = (uint8_t*)mmap(NULL, m_fb_fix.smem_len + mem_offset, PROT_READ | PROT_WRITE, MAP_SHARED, m_fb.fd, 0)) != MAP_FAILED)
					{
						m_fb.p = p;
#ifdef MACHINE_PIXIE
						// Enable automatic DMA of fb to display.  This setting is not
						// persistent across reboots, so it does not need to be disabled.
						(void) ioctl(m_fb.fd, MSMFB_AUTO_UPDATE, 1);
#endif
					}
				}
			}
		}
	}
}

int32_t
fbGraphic::GetFixScreenInfo()
{
	int32_t ret = -1;
	if (IsFbOpen())
	{
		ret = ioctl(m_fb.fd, FBIOGET_FSCREENINFO, &m_fb_fix);
	}
	return ret;
}

int32_t
fbGraphic::GetVarScreenInfo()
{
	int32_t ret = -1;
	if (IsFbOpen())
	{
		ret = ioctl(m_fb.fd, FBIOGET_VSCREENINFO, &m_fb_var);
	}
	return ret;
}

int32_t
fbGraphic::AdjustScreen(uint32_t xoffset, uint32_t yoffset)
{
	int32_t ret = -1;
	if (IsFbOpen())
	{
		m_fb_var.xoffset = xoffset;
		m_fb_var.yoffset = yoffset;
		if ((ret = ioctl(m_fb.fd, FBIOPAN_DISPLAY, &m_fb_var)) != 0)
		{
			GetVarScreenInfo();
		}
		m_fb.current = m_fb_var.yoffset / m_fb_var.yres;
	}
	return ret;
}

void
fbGraphic::DeInit()
{
	int32_t ret = 0;
	if (IsFbOpen() && m_init)
	{
		if (m_fb.p)
		{
			if ((ret = munmap(m_fb.p, GetScreenSize())) == 0)
			{
				m_fb.p = NULL;
			}
		}
		m_init = ret != 0;
	}
}

int32_t
fbGraphic::OpenFb()
{
	if (!IsFbOpen())
	{
		m_fb.fd = open(C_DISPLAY_FILENAME, O_RDWR);
	}
	return m_fb.fd;
}

int32_t
fbGraphic::CloseFb()
{
	int32_t ret = 0;
	if (IsFbOpen())
	{
		if ((ret = close(m_fb.fd)) > -1)
		{
			m_fb.fd = -1;
		}
	}
	return ret;
}

bool
fbGraphic::IsFbOpen()
{
	return m_fb.fd < 0 ? false : true;
}

bool
fbGraphic::IsInit()
{
	return m_init;
}

uint32_t
fbGraphic::GetXResolution()
{
	uint32_t res = 0;
	if (m_init)
	{
		res = m_fb_var.xres;
	}
	return res;
}

uint32_t
fbGraphic::GetYResolution()
{
	uint32_t res = 0;
	if (m_init)
	{
		res = m_fb_var.yres;
	}
	return res;
}

uint32_t
fbGraphic::GetBytesPerPixel()
{
	uint32_t res = 0;
	if (m_init)
	{
		if (!m_screen.bpp)
		{
			m_screen.bpp = m_fb_var.bits_per_pixel >> 3;
		}
		res = m_screen.bpp;
	}
	return res;
}

uint32_t
fbGraphic::GetLineSize()
{
	uint32_t res = 0;
	if (m_init)
	{
		if (!m_screen.lineSize)
		{
			m_screen.lineSize = m_fb_var.xres * GetBytesPerPixel();
		}
		res = m_screen.lineSize;
	}
	return res;
}

uint32_t
fbGraphic::GetScreenSize()
{
	uint32_t res = 0;
	if (m_init)
	{
		if (!m_screen.screenSize)
		{
			m_screen.screenSize = GetLineSize() * GetYResolution();
		}
		res = m_screen.screenSize;
	}
	return res;
}

uint32_t
fbGraphic::GetFrameNumber(e_FrameType frameType)
{
	uint32_t res = 0;
	switch (frameType)
	{
		case E_CURRENT:
			if (m_fb.y_ratio > 1 && GetVarScreenInfo() == 0 && m_fb_var.yres)
			{
				res = m_fb_var.yoffset / m_fb_var.yres;
			}
			break;
		case E_HIDDEN:
			res = GetFrameNumber(E_CURRENT);
			if (m_fb.y_ratio > 1)
			{
				if (++res >= m_fb.y_ratio)
				{
					res = 0;
				}
			}
			break;
		case E_USED:
			res = m_fb.current;
			break;
		case E_FIRST:
			res = 0;
			break;
		default:
			res = 0;
	}
	return res;
}

uint8_t*
fbGraphic::GetFb(bool current)
{
	uint8_t* p = m_fb.p;
	if (current && p)
	{
		p += m_fb.current * GetScreenSize();
	}
	return p;
}

bool
fbGraphic::Cls(e_FrameType frame, uint32_t color)
{
	bool ret = false;
	if (m_init && m_fb.p)
	{
		uint32_t* p;
		uint32_t count  = GetScreenSize() / GetBytesPerPixel();
		uint32_t fb_num = GetFrameNumber(frame);
		p = (uint32_t*)(m_fb.p + fb_num * GetScreenSize());
		while (count)
		{
			*p++ = color;
			--count;
		}
		ret = AdjustScreen(0, fb_num * m_fb_var.yres) == 0;
	}
	return ret;
}

bool
fbGraphic::RefreshScreen()
{
	bool ret = false;
	if (m_init && m_fb.p)
	{
		ret = AdjustScreen(0, m_fb.current * m_fb_var.yres) == 0;
	}
	return ret;
}

