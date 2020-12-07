//Разработайте программы для Linux и Windows, используя API операционных систем. У обычных файлов (regular file), у которых имя
//является целым неотрицательным числом прибавить к этому числу
//1000. Предполагается, что изначально файлы имеют названия, числовое значение которых не превышает 999.


#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdbool.h>
#include <stdbool.h>
#include <locale.h>
#include <direct.h>
#include <malloc.h>

void ErrorExit(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
        StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}

bool _is_dot_or_dotdot(const TCHAR* str) {
    if (str[0] != '.') {
        return false;
    }

    const TCHAR _Second_tchar = str[1];
    if (_Second_tchar == 0) {
        return true;
    }

    if (_Second_tchar != '.') {
        return false;
    }

    return str[2] == 0;
}


DWORD WINAPI CheckDir(LPVOID lpParam) {
    WIN32_FIND_DATA FindBuf;

    struct Titem {
        int data; //поле структуры
        struct Titem* next;
        HANDLE current_thread;
    };

    struct Titem* first = 0, *m; //структура с указателями на списки и области памяти
    int res = 0, res1 = 1, rv = 1, a = 0, i = 0;
    TCHAR ch[526], way[526];
    TCHAR newfilename[512], namebuf[256];
    HANDLE resdir;// current_thread;
    TCHAR* pointer;

    TCHAR* name = (TCHAR*)lpParam;

    _stprintf_s(ch, 100, _T("%s\\%s"), name, _T("*"));
    resdir = FindFirstFile(ch, &FindBuf);
    _tprintf(_T("Open dir: %s\n"), name);
    if (resdir != INVALID_HANDLE_VALUE) {
        while (FindNextFile(resdir, &FindBuf) != 0) {
            //TCHAR buffer[256];
            if (!(_is_dot_or_dotdot(FindBuf.cFileName))) {
                memset(way, '\0', 526);
                memset(newfilename, '\0', 512);
                _stprintf_s(way, 150, _T("%s\\%s"), name, FindBuf.cFileName);
                _stprintf_s(newfilename, 150, _T("%s\\"), name);
                _tprintf(_T("Now file is: %s\n"), way);
                if (FindBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    TCHAR* arg = (TCHAR*)malloc(500 * sizeof(TCHAR));
                    _tcscpy(arg, name);
                    HANDLE threadHandle = CreateThread(NULL, 0, CheckDir, arg, 0, NULL);
                    if (threadHandle != INVALID_HANDLE_VALUE) {
                        struct Titem *m = malloc(sizeof(struct Titem));
                        if (m) {
                            m->current_thread = threadHandle;
                            m->next = first;
                            first = m;
                        }
                        //CheckDir(way);
                    }
                }
                else {
                    if (!(FindBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        res1 = 1;
                        pointer = (&FindBuf.cFileName[0]);
                        while (*pointer != '\0') {
                            if ((*pointer < '0') || (*pointer > '9')) {
                                res1 = 0;
                            }
                            pointer++;
                        }
                        if (res1 == 1) {
                            a = _tstoi(FindBuf.cFileName);
                            if (a < 0) {
                                ErrorExit((LPTSTR)_T("Error of _wtoi\n"));
                                res = 1;
                            }
                            else {
                                if ((a > 0) && (a <= 999)) {
                                    a = a + 1000;
                                    if (_sntprintf(newfilename, 5, L"%d\n", a) < 1) {
                                        ErrorExit((LPTSTR)_T("Error of _sntprintf\n"));
                                        res = 1;
                                    }
                                    else {
                                        TCHAR tmpbuf[256];
                                        _stprintf(namebuf, (_T("%s\\%s")), name, FindBuf.cFileName);
                                        _stprintf(tmpbuf, (_T("%s\\%s")), name, newfilename);
                                        if (MoveFile(namebuf, tmpbuf) == 0) {
                                            ErrorExit((LPTSTR)_T("Error of rename\n"));
                                            _stprintf(namebuf, (_T("%s/%s")), (TCHAR*)resdir, FindBuf.cFileName);
                                            res = 1;
                                        }
                                        else {
                                            _tprintf(("The tename is access: %s", newfilename));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        _tprintf(_T("Program completed\n"));
    }
    else {
        ErrorExit((LPTSTR)_T("Error of reading directory\n"));
    }
    return res;
}

int main() {
    setlocale(LC_ALL, "");
    TCHAR name[256] = _T(".");
    CheckDir(name);
}
