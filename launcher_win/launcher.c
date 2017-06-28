#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <inttypes.h>
#include <locale.h>
#include <share.h>

typedef struct
{
	int alive;
	int count;
	HANDLE process;
} child_set_t;

child_set_t *sets;
int n = 4;
int64_t loops = 2500000;
HANDLE *wait_handles;
FILE *fp;

int ok = 0, ng = 0, others = 0, total = 0;

void printlog(const WCHAR *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	SYSTEMTIME tm;
	WCHAR msg[1024];

	GetLocalTime(&tm);
	_vsnwprintf(msg, 1024, fmt, list);
	fwprintf(fp, L"%04d/%02d/%02d %02d:%02d:%02d %s\n", 
		tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, msg);
}

void printerr(const WCHAR *msg1)
{
	DWORD err = GetLastError();
	WCHAR msg[256];
	FormatMessage(
		/*FORMAT_MESSAGE_ALLOCATE_BUFFER |*/
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		msg,
		256,
		NULL
	);
	wprintf(L"%s: %s\n", msg1, msg);
	printlog(L"%s: %s\n", msg1, msg);
}

void init()
{
	int i;

	wprintf(L"concurrency(e.g. 8) : ");
	wscanf_s(L"%d", &n);
	wprintf(L"loops(e.g. 2500000) : ");
	wscanf_s(L"%I64d", &loops);

	wprintf(L"concurrency=%d loops=%I64d\n", n, loops);

	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	sets = malloc(sizeof(child_set_t)*n);
	wait_handles = malloc(sizeof(HANDLE)*n);
	fp = _wfsopen(L"log.txt", L"a, ccs=UTF-8", _SH_DENYWR);
	for (i = 0; i < n; i++) {
		sets[i].alive = 0;
	}
}

void exec()
{
	int i;
	WCHAR cmdline[256];
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags |= STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_MINIMIZE;

	for (i = 0; i < n; ) {
		if (sets[i].alive) {
			i++;
			continue;
		}
		wsprintf(cmdline, L"ryzen_segv_test.exe %I64d", loops);
		if (CreateProcess(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
			sets[i].alive = 1;
			sets[i].process = pi.hProcess;
			sets[i].count = total++;
		} else {
			printerr(L"CreateProcess");
			others++;
			Sleep(10);
		}
	}
}

void check()
{
	int i, j = 0, exitchild;
	DWORD ret, childret;
	for (i = 0; i < n; i++) {
		if (!sets[i].alive) {
			continue;
		}
		wait_handles[j] = sets[i].process;
		j++;
	}
	ret = WaitForMultipleObjects(j, wait_handles, FALSE, 1000);
	if (ret == WAIT_FAILED) {
		printerr(L"WaitForMultipleObjects");
	} else if(ret >= WAIT_OBJECT_0 && ret < WAIT_OBJECT_0 + j) {
		exitchild = ret - WAIT_OBJECT_0;
		GetExitCodeProcess(sets[exitchild].process, &childret);
		if (childret == 0) {
			ok++;
			printlog(L"%d: OK", sets[exitchild].count);
		} else {
			ng++;
			printlog(L"%d: NG(%d)", sets[exitchild].count, childret);
		}
		sets[exitchild].alive = 0;
	}
}

int main()
{
	WCHAR *loc = _wsetlocale(LC_ALL, L"");
	_wsetlocale(LC_ALL, loc);
	init();
	while (1) {
		exec();
		check();
		printf("\rok:%d ng:%d others:%d", ok, ng, others);
	}
}
