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


#include "fbProgress.h"

static pthread_t gThreadId;
static bool gThread = false;
static int  gLockFd = -1;

static bool     gEraseOnly = false;
static bool     gDebug     = false;
static string   gMessage;
static string   gMessagePipe;
static string   gProgressPipe;

static stProgressOffset gProgressOffset;
static fbProgress* gProgress = NULL;

fbGraphic fbProgress::m_graphic;
fbImageStorage* fbProgress::m_storage = NULL;

////////////////////////////////////////////
//             fbProgress  class          //
////////////////////////////////////////////

fbProgress::fbProgress()
{
	m_storage = NULL;
	m_message = "";
	Init();
}

fbProgress::~fbProgress()
{
	DeInit();
}

void
fbProgress::Init()
{
	m_storage = new fbImageStorage(C_WORK_DIR);
	OpenPipes();
}

void
fbProgress::DeInit()
{
	ClosePipes();
	delete m_storage;
}

void
fbProgress::Load()
{
	if (m_storage)
	{
		m_storage->CreateIndex();
		m_storage->LoadImages();
	}
}

bool
fbProgress::Cls(e_FrameType frame, uint32_t color)
{
	bool ret = false;
	if (IsInit())
	{
		ret = m_graphic.Cls(frame, color);
	}
	return ret;
}

bool
fbProgress::RefreshScreen()
{
	bool ret = false;
	if (IsInit())
	{
		ret = m_graphic.RefreshScreen();
	}
	return ret;
}

bool
fbProgress::OpenPipe(stPipe* pipe, const char* file)
{
	bool ret = true;
	if (pipe->fd < 0 && file != NULL && strlen(file) > 0)
	{
		if (FileExists(file))
		{
			remove(file);
		}
		if (mkfifo(file, S_IRUSR| S_IWUSR) != -1)
		{
			if ((pipe->fd = open(file, O_RDWR)) != -1)
			{
				pipe->file = file;
			}
			else
			{
				ret = false;
			}
		}
		else
		{
			ret = false;
		}
	}
	return ret;
}

bool
fbProgress::OpenPipe(stPipe& pipe, const string& file)
{
	return OpenPipe(&pipe, file.c_str());
}

bool
fbProgress::ClosePipe(stPipe* pipe)
{
	bool ret = true;
	if (pipe->fd > - 1)
	{
		ret = 0 == close(pipe->fd);
		pipe->fd = -1;
		if (!pipe->file.empty())
		{
			remove(pipe->file.c_str());
			pipe->file.clear();
		}
	}
	return ret;
}

bool
fbProgress::ClosePipe(stPipe& pipe)
{
	return ClosePipe(&pipe);
}

void
fbProgress::OpenPipes()
{
	OpenPipe(m_pipe.progress, gProgressPipe);
	OpenPipe(m_pipe.message, gMessagePipe);
}

void
fbProgress::ClosePipes()
{
	ClosePipe(m_pipe.progress);
	ClosePipe(m_pipe.message);
}

bool
fbProgress::ReadMessage(string& cmd)
{
	bool ret = false;
	cmd.clear();
	if (IsInit() && !Empty())
	{
		if (m_pipe.message.fd > -1)
		{
			const uint16_t bufSz = 64;
			char buffer[bufSz];
			ssize_t sz;
			if ((sz = read(m_pipe.message.fd, buffer, bufSz - 1)) > 0)
			{
				size_t pos;
				buffer[sz] = '\0';
				string message = buffer;
				if ((pos = message.find(C_NL)) != string::npos)
				{
					message.erase(pos);
				}
				if (!message.empty())
				{
					to_lower(message);
					DrawMessage(message);
					cmd = message;
				}
				ret = true;
			}
		}
	}
	return ret;
}

bool
fbProgress::ReadProgress(string& cmd)
{
	bool ret = false;
	cmd.clear();
	if (IsInit() && !Empty())
	{
		if (m_pipe.progress.fd > -1)
		{
			gProgressOffset.max    = C_PROGRESS_FRAMES;
			uint16_t progressValue = 0;
			const uint16_t bufSz   = 16;
			char buffer[bufSz];
			ssize_t sz;
			if ((sz = read(m_pipe.progress.fd, buffer, bufSz - 1)) > 0)
			{
				char *p = NULL;
				buffer[sz] = '\0';
				if (sz && (p = strrchr (buffer, C_NL)) != NULL)
				{
					*p = '\0';
				}
				if (sz && strlen(buffer))
				{
					cmd = buffer;
					if ((p = strchr (buffer, ':')) != NULL)
					{
						*p = '\0';
						if (*++p != '\0' && p < &buffer[sz])
						{
							uint16_t max = (uint16_t)abs(atoi(p));
							if (max && gProgressOffset.max > max)
							{
								gProgressOffset.max = max;
							}
						}
					}
					if (IsNumber(buffer))
					{
						int progress = abs(atoi(buffer));
						progressValue = progress <= C_PROGRESS_FRAMES ? progress : C_PROGRESS_FRAMES;
						if (progressValue < gProgressOffset.value)
						{
							progressValue = gProgressOffset.value;
						}
					}
					else
					{
						progressValue = gProgressOffset.value + 1;
					}
					if (progressValue > gProgressOffset.max)
					{
						progressValue = gProgressOffset.max;
					}
					if (progressValue <= gProgressOffset.max && progressValue > gProgressOffset.value)
					{
						for (uint16_t i = gProgressOffset.value; i < progressValue; ++i)
						{
							DrawProgress(i);
							usleep(C_PROGRESS_PAUSE);
						}
						gProgressOffset.value = progressValue;
					}
					ret = true;
				}
			}
		}
	}
	return ret;
}

void
fbProgress::WatchPipes()
{
	if (IsInit() && !Empty())
	{
		if (m_pipe.progress.fd > -1 && m_pipe.message.fd > -1)
		{
			for (;;)
			{
				fd_set set;
				FD_ZERO(&set);
				FD_SET(m_pipe.progress.fd, &set);
				FD_SET(m_pipe.message.fd, &set);
				string cmd;
				switch (select(FD_SETSIZE, &set, NULL, NULL, NULL))
				{
					case -1: // error
					case  0: // timeout
						break;
					default:
						if (FD_ISSET(m_pipe.progress.fd, &set))
						{
							ReadProgress(cmd);
						}
						else if (FD_ISSET(m_pipe.message.fd, &set))
						{
							ReadMessage(cmd);
						}
				}
				if (!cmd.empty())
				{
					to_lower(cmd);
					if (0 == cmd.compare(C_STOP_CMD))
					{
						break;
					}
				}
			}
		}
	}
}

void
fbProgress::DrawMessage(const string& message)
{
	if (IsInit() && !Empty())
	{
		if (message != m_message)
		{
			map<string, fbTga*>::iterator it;
			if ((it = m_storage->Find(message)) != m_storage->End())
			{
				uint32_t x, y;
				m_message  = message;
				if (it->second->GetWidth() >= it->second->GetHeight())		// horizontal images
				{
					x = (m_graphic.GetXResolution() - it->second->GetWidth()) / 2;
					y =  m_graphic.GetYResolution() - it->second->GetHeight();
				}
				else								// rotated images
				{
					x =  m_graphic.GetXResolution() - it->second->GetWidth();
					y = (m_graphic.GetYResolution() - it->second->GetHeight()) / 2;
				}
				it->second->Draw(&m_graphic, x, y, 0);
				m_graphic.RefreshScreen();
			}
		}
	}
}

void
fbProgress::DrawMessage(const char* message)
{
	if (message && strlen(message))
	{
		string _message = message;
		DrawMessage(message);
	}
}

void
fbProgress::DrawBackground()
{
	if (IsInit() && !Empty())
	{
		map<string, fbTga*>::iterator it;
		string bg = C_IMAGE_BACKGROUND;
		if ((it = m_storage->Find(bg)) != m_storage->End())
		{
			uint32_t x = (m_graphic.GetXResolution() - it->second->GetWidth()) / 2;
			uint32_t y = (m_graphic.GetYResolution() - it->second->GetHeight()) /2;
			if (true == it->second->Draw(&m_graphic, x, y))
			{
				m_graphic.RefreshScreen();
			}
		}
	}
}

void
fbProgress::DrawFrame(fbTga* image, const uint32_t x, const uint32_t y, const uint16_t frameOffset, const bool refreshScreen)
{
	if (image && !image->Empty())
	{
		if (true == image->Draw(&m_graphic, x, y, frameOffset))
		{
			if (refreshScreen)
			{
				m_graphic.RefreshScreen();
			}
		}
	}
}

void*
fbProgress::DrawAnimation(void* arg)
{
	if (m_graphic.IsInit() && m_storage && !m_storage->Empty())
	{
		map<string, fbTga*>::iterator it;
		string animation = C_IMAGE_ANIMATION;
		if ((it = m_storage->Find(animation)) != m_storage->End())
		{
			uint32_t x = (m_graphic.GetXResolution() - it->second->GetFrameWidth()) / 2;
			uint32_t y = (m_graphic.GetYResolution() - it->second->GetFrameHeight()) /2;
			uint16_t frameoffset = it->second->GetFrameFirst();
			int16_t step = 0;
			if (it->second->GetFrameFirst() < it->second->GetFrameLast())
			{
				++step;
			}
			else if (it->second->GetFrameFirst() > it->second->GetFrameLast())
			{
				--step;
			}
			for(;;)
			{
				DrawFrame(it->second, x, y, frameoffset, true);
				if (step < 0 && frameoffset == 0)
				{
					frameoffset = C_ANIMATION_FRAMES - 1;
				}
				else if (step > 0 && frameoffset >= C_ANIMATION_FRAMES - 1)
				{
					frameoffset = 0;
				}
				else
				{
					frameoffset += step;
				}
				usleep(C_ANIMATION_PAUSE);
			}
		}
	}
	return 0;
}

void
fbProgress::DrawProgress(const uint16_t offset)
{
	if (IsInit() && !Empty())
	{
		map<string, fbTga*>::iterator it;
		string progress = C_IMAGE_PROGRESS;
		if ((it = m_storage->Find(progress)) != m_storage->End())
		{
			uint32_t x = (m_graphic.GetXResolution() - it->second->GetFrameWidth()) / 2;
			uint32_t y = (m_graphic.GetYResolution() - it->second->GetFrameHeight()) /2;
			uint16_t frameoffset = it->second->GetFrameFirst();
			if (it->second->GetFrameFirst() < it->second->GetFrameLast())
			{
				frameoffset += offset;
				frameoffset %= (C_PROGRESS_FRAMES);
			}
			else if (it->second->GetFrameFirst() > it->second->GetFrameLast())
			{
				frameoffset -= (offset % C_PROGRESS_FRAMES);
			}
			DrawFrame(it->second, x, y, frameoffset, false);
		}
	}
}

bool
fbProgress::IsInit()
{
	return m_graphic.IsInit();
}

bool
fbProgress::Empty()
{
	return !m_storage || m_storage->Empty();
}

////////////////////////////////////////////
//               Lock()                   //
////////////////////////////////////////////

bool
Lock()
{
	bool res = false;
	if((gLockFd = open(C_LOCK_FILE, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH)) > -1)
	{
		struct flock fl;
		fl.l_type   = F_WRLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start  = 0;
		fl.l_len    = 0;
		fl.l_pid    = getpid();
		if(fcntl(gLockFd, F_SETLK, &fl) == -1)
		{
			close(gLockFd);
			gLockFd = -1;
		}
		else
		{
			res = true;
		}
	}
	else
	{
		fprintf(stderr, "ERROR: Cannot open lock file %s\n", C_LOCK_FILE);
	}
	return res;
}

////////////////////////////////////////////
//              IsLocked()                //
////////////////////////////////////////////

bool
IsLocked()
{
	return gLockFd > -1;
}

////////////////////////////////////////////
//               Unlock()                 //
////////////////////////////////////////////

bool
Unlock()
{
	bool res = true;
	if (IsLocked())
	{
		struct flock fl;
		fl.l_type   = F_UNLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start  = 0;
		fl.l_len    = 0;
		fl.l_pid    = getpid();
		if(fcntl(gLockFd, F_SETLK, &fl) != -1)
		{
			close(gLockFd);
			gLockFd = -1;
			remove(C_LOCK_FILE);
		}
		else
		{
			res = false;
			fprintf(stderr, "ERROR: Cannot unlock file %s\n", C_LOCK_FILE);
		}
	}
	return res;
}

////////////////////////////////////////////
//             WriteToPipe()              //
////////////////////////////////////////////

bool
WriteToPipe(const char* file, const char* msg)
{
	bool ret = false;
	if (file != NULL && strlen(file) > 0 && msg != NULL && strlen(msg) > 0)
	{
		if (FileExists(file))
		{
			int32_t fd;
			if ((fd = open(file, O_WRONLY | O_NONBLOCK)) != -1)
			{
				string buffer = msg;
				if (buffer[buffer.length() - 1] != C_NL)
				{
					buffer += C_NL;
				}
				ssize_t sz = write(fd, buffer.c_str(), buffer.length());
				if (sz > - 1 && buffer.length() == static_cast<size_t>(sz))
				{
					ret = true;
				}
				close(fd);
			}
		}
	}
	return ret;
}

////////////////////////////////////////////
//                MkDir()                 //
////////////////////////////////////////////

int
MkDir(const char *dirname, mode_t mode)
{
	int ret=0;
	if (dirname != NULL && strlen(dirname))
	{
		string dir = dirname;
		string path;
		struct stat st;
		size_t pos = 0;
		while (ret == 0 && path != dir)
		{
			pos  = dir.find(C_DIR_SEPARATOR, ++pos);
			path = pos != string::npos && pos < dir.length() - 1 ? dir.substr(0, pos) : dir;
			if (stat(path.c_str(), &st) != 0)
			{
				ret = mkdir(path.c_str(), mode);
			}
			else if (!S_ISDIR(st.st_mode))
			{
				ret = -1;
			}
		}
	}
	else
	{
		--ret;
	}
	return ret;
}

////////////////////////////////////////////
//                RmDir()                 //
////////////////////////////////////////////

int
RmDir(const char* dirname)
{
	int ret = 0;
	if (dirname != NULL && strlen(dirname))
	{
		DIR* dir;
		if ((dir = opendir(dirname)) != NULL)
		{
			struct dirent *entry;
			while ((entry = readdir(dir)) != NULL)
			{
				if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
				{
					string path = dirname; path += C_DIR_SEPARATOR; path += entry->d_name;
					if (entry->d_type == DT_DIR)
					{
						if (RmDir(path.c_str()) != 0)
						{
							ret = -1;
						}
					}
					if (remove(path.c_str()) != 0)
					{
						ret = -1;
					}
				}
			}
			closedir(dir);
			remove(dirname);
		}
		else
		{
			--ret;
		}
	}
	else
	{
		--ret;
	}
	return ret;
}

////////////////////////////////////////////
//                IsNumber()               //
////////////////////////////////////////////

bool
IsNumber(const char* str)
{
	bool isnum = false;
	if (str != NULL)
	{
		size_t len;
		if ((len = strlen(str)) > 0)
		{
			isnum = true;
			for (uint16_t i = 0; i < len; ++i)
			{
				if (!isdigit(str[i]))
				{
					isnum = false;
					break;
				}
			}
		}
	}
	return isnum;
}

////////////////////////////////////////////
//               Usage()                  //
////////////////////////////////////////////

void
Usage()
{
	fprintf(stderr, "\nUtility to draw progress directly to frame buffer\n\n" \
			"\t-e (erase display and exit)\n" \
			"\t-f <base name of string image file>\n\t\tbattery-only | saving | installing | restoring | erasing\n" \
			"\t-p <named progress pipe> (/tmp/progress by default)\n\t\tIf the progess pipe does not exist, display indeterminate progress\n" \
			"\t-m <named message pipe> (/tmp/message by default)\n\t\tIf the message pipe does not exist, statically display a file passed by -f argument\n" \
			"\t-s <starting point for progress bar> (0 ... %d, default 0)\n" \
			"\t-d debug mode\n\n", C_PROGRESS_FRAMES);
}

////////////////////////////////////////////
//             ParseInput()               //
////////////////////////////////////////////

int
ParseInput(int argc, char *argv[])
{
	bool ret = 0;
	int c, prev_ind;
	while(prev_ind = optind, (c =  getopt(argc, argv, "def:m:p:s:")) != EOF)
	{
		if ( optind == prev_ind + 2 && *optarg == '-' )
		{
			c = ':';
			--optind;
		}
		switch (c)
		{
			case 'd':
				gDebug              = true;
				break;
			case 'e':
				gEraseOnly          = true;
				break;
			case 'f':
				gMessage            = optarg;
				to_lower(gMessage);
				break;
			case 'm':
				gMessagePipe        = optarg;
				break;
			case 'p':
				gProgressPipe       = optarg;
				break;
			case 's':
				gProgressOffset.arg = optarg;
				to_lower(gProgressOffset.arg);
				break;
			default:
				Usage();
				ret = 1;
		}
		if (ret != 0)
		{
			break;
		}
	}
	if (!gProgressOffset.arg.empty())
	{
		size_t pos;
		string offset;
		string max;
		if ((pos = gProgressOffset.arg.find(':')) != string::npos)
		{
			offset = gProgressOffset.arg.substr(0, pos);
			max    = gProgressOffset.arg.substr(++pos);
		}
		else
		{
			offset = gProgressOffset.arg;
		}
		if (!max.empty())
		{
			if (IsNumber(max.c_str()))
			{
				int i = abs(atoi(max.c_str()));
				gProgressOffset.max = i <= C_PROGRESS_FRAMES ? i : C_PROGRESS_FRAMES;
			}
		}
		if (!offset.empty())
		{
			int i;
			if (IsNumber(offset.c_str()))
			{
				i = abs(atoi(offset.c_str()));
			}
			else
			{
				i = gProgressOffset.value + 1;
			}
			gProgressOffset.value = i <= gProgressOffset.max ? i : gProgressOffset.max;
		}
	}
	return ret;
}

////////////////////////////////////////////
//               SigHandler               //
////////////////////////////////////////////

void
SigHandler(int signo)
{
	if (gProgress)
	{
		if (gProgress->IsInit())
		{
			if (gThread)
			{
				pthread_cancel(gThreadId);
				gThread = false;
			}
			if (gProgress->Cls(E_HIDDEN, gDebug ? 0xFFFF0000 : 0xFF000000))
			{
				gProgress->RefreshScreen();
			}
		}
		delete gProgress;
		gProgress = NULL;
	}
	if (IsLocked())
	{
		Unlock();
		RmDir(C_WORK_DIR);
	}
	switch (signo)
	{
		case SIGINT:
			fprintf(stderr, "SIGINT signal");
			break;
		case SIGTERM:
			fprintf(stderr, "SIGTERM signal");
			break;
		default:
			fprintf(stderr, "Signal [%d]", signo);
			break;
	}
	fprintf(stderr, " received. Bailing out...\n");
	exit(1);
}

////////////////////////////////////////////
//                main()                  //
////////////////////////////////////////////

int
main(int argc, char *argv[])
{
	if (argc > 1)
	{
		if (0 != ParseInput(argc, argv))
		{
			exit(1);
		}
	}
	if (gMessagePipe.empty())
	{
		gMessagePipe = C_MESSAGE_PIPE;
	}
	if (gProgressPipe.empty())
	{
		gProgressPipe = C_PROGRESS_PIPE;
	}
	MkDir(C_WORK_DIR);
	signal(SIGINT,  SigHandler);
	signal(SIGTERM, SigHandler);
	Lock();
	if (!IsLocked())
	{
		bool ret = true;
		if (ret && !gMessage.empty())
		{
			ret = WriteToPipe(gMessagePipe.c_str(), gMessage.c_str());
		}
		if (ret && !gProgressOffset.arg.empty())
		{
			ret = WriteToPipe(gProgressPipe.c_str(), gProgressOffset.arg.c_str());
		}
		if (!ret)
		{
			Lock();			// is reader down?
		}
	}
	if (IsLocked())				// singleton
	{
		if (gMessage.empty() || 0 != gMessage.compare(C_STOP_CMD))
		{
			gProgress = new fbProgress;
			if (gProgress->IsInit())
			{
				gProgress->Cls(E_HIDDEN, gDebug ? 0xFF00FF00 : 0xFF000000);
				if (!gEraseOnly)
				{
					gProgress->Load();
					if (!gProgress->Empty())
					{
						gProgress->DrawBackground();
						if (0 == pthread_create(&gThreadId, NULL, gProgress->DrawAnimation, NULL))
						{
							gThread = true;
						}
						if (!gMessage.empty())
						{
							gProgress->DrawMessage(gMessage);
						}
						if (gProgressOffset.value)
						{
							for (uint16_t i = 0; i < gProgressOffset.value; ++i)
							{
								gProgress->DrawProgress(i);
								usleep(C_PROGRESS_PAUSE);
							}
						}
						gProgress->WatchPipes();
						if (gThread)
						{
							pthread_cancel(gThreadId);
							gThread = false;
						}
						gProgress->Cls(E_HIDDEN, gDebug ? 0xFF0000FF : 0xFF000000);
					}
				}
			}
			delete gProgress;
		}
		Unlock();
		RmDir(C_WORK_DIR);
	}
	return 0;
}
