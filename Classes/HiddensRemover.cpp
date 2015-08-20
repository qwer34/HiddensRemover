// HiddensRemover.cpp : 定义控制台应用程序的入口点。
//

#if defined(WIN32)

// Exclude rarely-used stuff from Windows headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif	//WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <io.h>

#ifdef _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#else

#include <unistd.h>
#include <dirent.h>
#include <ftw.h>
#include <sys/param.h>

#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

#ifndef R_OK
#define R_OK		(4)		/* Test for read permission.  */
#endif

#ifndef W_OK
#define W_OK		(2)		/* Test for write permission.  */
#endif

#ifndef F_OK
#define F_OK		(0)		/* Test for existence.  */
#endif

#ifdef WIN32
#define PATH_SEPARATOR_CHAR				'\\'
#define PATH_SEPARATOR_STRING			"\\"
#else
#define PATH_SEPARATOR_CHAR				'/'
#define PATH_SEPARATOR_STRING			"/"
#endif

using namespace std;

//////////////////////////////////////////////////////////////////////////

#define EXEC_BINARY_NAME				"HiddensRemover"
#define EXEC_BINARY_VERSION				"0.0.1"

//////////////////////////////////////////////////////////////////////////

static void PrintInfo(void)
{
	printf("************************************\r\n");
	printf("* %s  Ver. %s\r\n", EXEC_BINARY_NAME, EXEC_BINARY_VERSION);
	printf("* Powered by Xin Zhang\r\n");
	printf("* %s %s\r\n", __TIME__, __DATE__);
	printf("*\r\n");
	printf("* Usage:\r\n");
	printf("* ------\r\n");
	printf("* %s [<Dir>]\r\n", EXEC_BINARY_NAME);
	printf("************************************\r\n");
}

#ifndef WIN32

static int MacUnlinkCallback(const char * szPath, const struct stat * pStat, int nTypeflag, struct FTW * pFtw)
{
	return remove(szPath);
}

#endif

static void CleanDirectory(const string& strPath, bool bRemoveAll = false)
{
#ifdef WIN32

	string strToScan = strPath + "*.*";
	WIN32_FIND_DATAA FindFileData = { 0 };
	HANDLE hFind = ::FindFirstFileA(strToScan.c_str(), &FindFileData);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		printf("Cannot be accessed: %s\r\n", strPath.c_str());
		return;
	}

	while (TRUE)
	{
		string strFileName = FindFileData.cFileName;

		if ("." == strFileName || ".." == strFileName || ".svn" == strFileName || ".git" == strFileName)
		{
			//
		}
		else
		{
			bool bIsDir = (0 != (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
			bool bStartingWithDot = ('.' == strFileName[0]);

			if (bIsDir)
			{
				const string strSubDir = strPath + strFileName + PATH_SEPARATOR_STRING;
				CleanDirectory(strSubDir, bRemoveAll || bStartingWithDot);
			}
			else if (bRemoveAll || bStartingWithDot)
			{
				const string strSubFile = strPath + strFileName;

				if (-1 == remove(strSubFile.c_str()))
				{
					printf("Remove file failed: %s\r\n", strSubFile.c_str());
				}
				else
				{
					printf("File removed: %s\r\n", strSubFile.c_str());
				}
			}
		}

		if (!FindNextFileA(hFind, &FindFileData))
		{
			break;
		}
	}

	FindClose(hFind);

	if (bRemoveAll)
	{
		if (-1 == remove(strPath.c_str()))
		{
			std::string strCmd = "cmd /c rd /s /q \"" + strPath + "\"";

			if (WinExec(strCmd.c_str(), SW_HIDE) > 31)
			{
				printf("Directory removed: %s\r\n", strPath.c_str());
			}
			else
			{
				printf("Remove directory failed: %s\r\n", strPath.c_str());
			}
		}
		else
		{
			printf("Directory removed: %s\r\n", strPath.c_str());
		}
	}

#else

	DIR * dir;
	struct dirent * entry;
	struct stat statbuf;

	if (!(dir = opendir(strPath.c_str())))
	{
		printf("Cannot be accessed: %s\r\n", strPath.c_str());
		return;
	}

	while ((entry = readdir(dir)))
	{
		string strFileName = entry->d_name;

		if ("." == strFileName || ".." == strFileName || ".svn" == strFileName || ".git" == strFileName)
		{
			//
		}
		else
		{
			string strSub = strPath + entry->d_name;

			lstat(strSub.c_str(), &statbuf);

			bool bIsDir = S_ISDIR(statbuf.st_mode);
			bool bStartingWithDot = ('.' == strFileName[0]);

			if (bIsDir)
			{
				strSub += PATH_SEPARATOR_STRING;
				CleanDirectory(strSub, bRemoveAll || bStartingWithDot);
			}
			else if (bRemoveAll || bStartingWithDot)
			{
				if (-1 == remove(strSub.c_str()))
				{
					printf("Remove file failed: %s\r\n", strSub.c_str());
				}
				else
				{
					printf("File removed: %s\r\n", strSub.c_str());
				}
			}
		}
	}

	closedir(dir);

	if (bRemoveAll)
	{
		if (-1 == remove(strPath.c_str()))
		{
			if (nftw(strPath.c_str(), MacUnlinkCallback, 64, FTW_DEPTH | FTW_PHYS))
			{
				printf("Remove directory failed: %s\r\n", strPath.c_str());
			}
			else
			{
				printf("Directory removed: %s\r\n", strPath.c_str());
			}
		}
		else
		{
			printf("Directory removed: %s\r\n", strPath.c_str());
		}
	}

#endif
}

int main(int argc, char * argv[])
{
	PrintInfo();

	string strRoot = "." PATH_SEPARATOR_STRING;

	if (argc >= 2)
	{
		strRoot = argv[1];

		char cLastChar = strRoot[strRoot.length() - 1];

		if (PATH_SEPARATOR_CHAR != cLastChar && '/' != cLastChar)
		{
			strRoot += PATH_SEPARATOR_STRING;
		}
	}

	CleanDirectory(strRoot);

#if defined(WIN32) && (defined(DEBUG) || defined(_DEBUG))
	getchar();
#endif

	return 0;
}