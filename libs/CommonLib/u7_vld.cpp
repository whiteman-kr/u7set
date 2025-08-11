#include "./include/CommonLib/u7_vld.h"

#ifdef VLD_IS_INCLUDED

	#include <windows.h>

namespace
{
	void GetCounterFilePath(WCHAR* path, DWORD size)
	{
		GetTempPathW(size, path);
		lstrcatW(path, L"vld_counters.tmp");
	}

	void GetLeakFilePath(WCHAR* path, DWORD size)
	{
		GetTempPathW(size, path);
		lstrcatW(path, L"vld_currentleak.tmp");
	}

	void AppendToFile(LPCWSTR path, const wchar_t* msg)
	{
		HANDLE h = CreateFileW(path, FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			DWORD written;
			WriteFile(h, msg, (DWORD)(wcslen(msg) * sizeof(wchar_t)), &written, NULL);
			CloseHandle(h);
		}
	}

	void ResetFile(LPCWSTR path)
	{
		HANDLE h = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			CloseHandle(h);
		}
	}

	bool ContainsProjectDir(const WCHAR* buf, size_t len)
	{
		const WCHAR needle[] = VLD_PROJECT_ROOT_PATH;
		const size_t needleLen = (sizeof(needle) / sizeof(WCHAR)) - 1;

		for (size_t i = 0; i + needleLen <= len; ++i)
		{
			size_t j = 0;
			while (j < needleLen)
			{
				WCHAR c1 = buf[i + j];
				WCHAR c2 = needle[j];
				if (c1 >= L'A' && c1 <= L'Z')
					c1 += 32;
				if (c2 >= L'A' && c2 <= L'Z')
					c2 += 32;
				if (c1 != c2)
					break;
				++j;
			}
			if (j == needleLen)
				return true;
		}
		return false;
	}

	bool FileContainsProjectLeak(LPCWSTR path)
	{
		HANDLE h = CreateFileW(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (h == INVALID_HANDLE_VALUE)
			return false;

		WCHAR buf[1024];
		DWORD readBytes;
		bool found = false;
		while (ReadFile(h, buf, sizeof(buf), &readBytes, NULL) && readBytes)
		{
			size_t chars = readBytes / sizeof(WCHAR);
			if (ContainsProjectDir(buf, chars))
			{
				found = true;
				break;
			}
		}
		CloseHandle(h);
		return found;
	}

	void OutputFile(LPCWSTR path)
	{
		HANDLE h = CreateFileW(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (h == INVALID_HANDLE_VALUE)
			return;

		WCHAR buf[512];
		DWORD readBytes;
		while (ReadFile(h, buf, sizeof(buf) - sizeof(WCHAR), &readBytes, NULL) && readBytes)
		{
			buf[readBytes / sizeof(WCHAR)] = 0;
			OutputDebugStringW(buf);
		}
		CloseHandle(h);
	}

	void UpdateCounters(bool projectLeak)
	{
		WCHAR path[MAX_PATH];
		GetCounterFilePath(path, MAX_PATH);

		DWORD projectLeaks = 0;
		DWORD otherLeaks = 0;

		HANDLE h = CreateFileW(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			DWORD readBytes = 0;
			DWORD counters[2] = {0, 0};
			if (ReadFile(h, counters, sizeof(counters), &readBytes, NULL) && readBytes == sizeof(counters))
			{
				projectLeaks = counters[0];
				otherLeaks = counters[1];
			}
			CloseHandle(h);
		}

		if (projectLeak == true)
		{
			projectLeaks++;
		}
		else
		{
			otherLeaks++;
		}

		h = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			DWORD counters[2] = {projectLeaks, otherLeaks};
			DWORD written;
			WriteFile(h, counters, sizeof(counters), &written, NULL);
			CloseHandle(h);
		}
	}

	void ReadCounters(DWORD& projectLeaks, DWORD& otherLeaks)
	{
		projectLeaks = 0;
		otherLeaks = 0;

		WCHAR path[MAX_PATH];
		GetCounterFilePath(path, MAX_PATH);

		HANDLE h = CreateFileW(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			DWORD counters[2];
			DWORD readBytes;
			if (ReadFile(h, counters, sizeof(counters), &readBytes, NULL) && readBytes == sizeof(counters))
			{
				projectLeaks = counters[0];
				otherLeaks = counters[1];
			}
			CloseHandle(h);
		}
	}

	void ResetCounterFile()
	{
		WCHAR path[MAX_PATH];
		GetCounterFilePath(path, MAX_PATH);

		HANDLE h = CreateFileW(path, GENERIC_READ, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			CloseHandle(h);
		}

		return;
	}

	extern "C" int __cdecl vldReportHook([[maybe_unused]] int reportType, wchar_t* message, [[maybe_unused]] int* retVal)
	{
		if (message == nullptr)
		{
			return 1; // suppress VLD
		}

		WCHAR leakFile[MAX_PATH];
		GetLeakFilePath(leakFile, MAX_PATH);

		// Summary line replacement
		if (wcsncmp(message, L"Visual Leak Detector detected", 29) == 0)
		{
			DWORD projectLeaks, otherLeaks;
			ReadCounters(projectLeaks, otherLeaks);
			WCHAR buf[256];
			if (projectLeaks == 0)
			{
				wsprintfW(buf,
						  L"VLD: Visual Leak Detector detected %lu project leaks and %lu other memory leaks\n\n",
						  projectLeaks,
						  otherLeaks);
			}
			else
			{
				wsprintfW(buf,
						  L"VLD: VISUAL LEAK DETECTOR DETECTED %lu project leaks and %lu other memory leaks\n\n",
						  projectLeaks,
						  otherLeaks);
			}

			OutputDebugStringW(L"\n");
			OutputDebugStringW(buf);

			ResetCounterFile();
			return 1;
		}

		// Start of a leak block
		//
		if (wcsncmp(message, L"---------- Block ", 17) == 0)
		{
			ResetFile(leakFile);
		}

		// Append every line to current leak file
		//
		AppendToFile(leakFile, message);

		// End of leak block
		//
		if (wcscmp(message, L"\n\n") == 0)
		{
			bool projectLeak = FileContainsProjectLeak(leakFile);

			UpdateCounters(projectLeak);

			if (projectLeak == true)
			{
				OutputFile(leakFile);
			}

			ResetFile(leakFile);
			return 1;
		}

		return 1; // suppress VLD output
	}
} // namespace
#endif

namespace Vld
{
	void setVldReportFilterHook()
	{
#ifdef VLD_IS_INCLUDED
		::VLDSetReportHook(VLD_RPTHOOK_INSTALL, vldReportHook);
#endif
	}
} // namespace Vld
