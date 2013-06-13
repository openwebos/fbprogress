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


#ifndef __FBPROGRESS_H
#define __FBPROGRESS_H

#include <csignal>
#include <sys/select.h>
#include <dirent.h>
#include "fbCommon.h"
#include "fbImageStorage.h"
#include "fbGraphic.h"

using namespace std;

class fbProgress;

struct stPipe {
	int32_t fd;
	string  file;
	stPipe(): fd(-1) {}
};

struct stPipes {
	stPipe progress;
	stPipe message;
};

struct stProgressOffset {
	string   arg;
	uint16_t value;
	uint16_t max;
	stProgressOffset(): value(0), max(C_PROGRESS_FRAMES) {}
};

const char* const C_LOCK_FILE     = C_WORK_DIR"/fbprogress.lock";
const char* const C_STOP_CMD      = "stop";

static int  ParseInput(int argc, char *argv[]);
static int  MkDir(const char *dirname, mode_t mode = 0777);
static int  RmDir(const char* dirname);
static void SigHandler(int signo);
static void Usage();
static bool Lock();
static bool IsLocked();
static bool Unlock();
static bool IsNumber(const char* str);
static bool WriteToPipe(const char* file, const char* msg);

class fbProgress
{
private:
	string                 m_message;
	stPipes                m_pipe;
	static fbGraphic       m_graphic;
	static fbImageStorage* m_storage;
	static void DrawFrame(fbTga* image, const uint32_t x, const uint32_t y, const uint16_t frameOffset, const bool refreshScreen = true);
	void Init();
	void DeInit();
	bool OpenPipe(stPipe* pipe, const char* file);
	bool OpenPipe(stPipe& pipe, const string& file);
	bool ClosePipe(stPipe* pipe);
	bool ClosePipe(stPipe& pipe);
	void OpenPipes();
	void ClosePipes();
	bool ReadMessage(string& cmd);
	bool ReadProgress(string& cmd);
public:
	fbProgress();
	~fbProgress();
	void Load();
	bool Cls(e_FrameType frame = E_HIDDEN, uint32_t color = 0xFF000000);
	bool RefreshScreen();
	bool IsInit();
	bool Empty();
	void DrawMessage(const string& message);
	void DrawMessage(const char* message);
	void DrawBackground();
	void DrawProgress(const uint16_t offset);
	void WatchPipes();
	static void* DrawAnimation(void* arg = NULL);
};

#endif
