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

#ifndef __FBGRAPHIC_H
#define __FBGRAPHIC_H

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "fbCommon.h"

#ifndef PAGE_SHIFT
	#define PAGE_SHIFT 12
#endif
#ifndef PAGE_SIZE
	#define PAGE_SIZE (1UL << PAGE_SHIFT)
#endif
#ifndef PAGE_MASK
	#define PAGE_MASK (~(PAGE_SIZE - 1))
#endif

using namespace std;

class fbGraphic;

enum e_FrameType { E_FIRST = 0, E_USED, E_CURRENT, E_HIDDEN};

static const char* const C_DISPLAY_FILENAME = "/dev/fb0";
static const uint32_t C_MAX_INTERLACE       = 5;

class fbGraphic
{
private:
	bool                     m_init;
	struct fb_fix_screeninfo m_fb_fix;
	struct fb_var_screeninfo m_fb_var;
	struct stScreenInfo {
		uint32_t  lineSize;
		uint32_t  screenSize;
		uint32_t  bpp;
		stScreenInfo() : lineSize(0), screenSize(0), bpp(0) {}
	};
	struct stFramebuffer {
		int32_t   fd;
		uint8_t*  p;
		uint32_t  current;
		uint32_t  x_ratio;
		uint32_t  y_ratio;
		stFramebuffer() : fd(-1), p(NULL), current(0), x_ratio(1), y_ratio(1) {}
	};
	stScreenInfo             m_screen;
	stFramebuffer            m_fb;
	void Init();
	void DeInit();
	int32_t OpenFb();
	int32_t CloseFb();
	int32_t AdjustScreen(uint32_t xoffset = 0, uint32_t yoffset = 0);
	int32_t GetFixScreenInfo();
	int32_t GetVarScreenInfo();
public:
	fbGraphic();
	~fbGraphic();
	bool IsFbOpen();
	bool IsInit();
	bool Cls(e_FrameType frame = E_HIDDEN, uint32_t color = 0xFF000000);
	bool RefreshScreen();
	uint32_t GetXResolution();
	uint32_t GetYResolution();
	uint32_t GetBytesPerPixel();
	uint32_t GetLineSize();
	uint32_t GetScreenSize();
	uint32_t GetFrameNumber(e_FrameType frameType = E_CURRENT);
	uint8_t* GetFb(bool current = true);
};

#endif
