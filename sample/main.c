/*
 *	This file is a part of Light PNG Loader library.
 *	Copyright (c) 2009-2011 Alex Pankratov. All rights reserved.
 *
 *	http://swapped.cc/lpng
 */

/*
 *	The program is distributed under terms of BSD license. 
 *	You can obtain the copy of the license by visiting:
 *
 *	http://www.opensource.org/licenses/bsd-license.php
 */

#include <windows.h>
#include "../lpng.h"

#include "resource.h"

HBITMAP png1 = NULL;
int     png1_w = 0;
int     png1_h = 0;

HBITMAP png2 = NULL;
int     png2_w = 0;
int     png2_h = 0;

LRESULT __stdcall WndProc(HWND hwnd, UINT id, WPARAM wp, LPARAM lp)
{
	if (id == WM_CREATE)
	{
		/*
		 *	load PNG from a file
		 */
		wchar_t name[256];
		BITMAP  info;

		#pragma warning (disable: 4996)

		GetModuleFileName(NULL, name, sizeof(name)/2 - 12);
		wcscpy(wcsrchr(name, L'\\'), L"\\image1.png");

		png1 = LoadPng(name, NULL, NULL, TRUE);
		if (png1)
		{
			GetObject(png1, sizeof(info), &info);
			png1_w = info.bmWidth;
			png1_h = info.bmHeight;
		}

		/*
		 *	load PNG from a resource
		 */
		png2 = LoadPng(MAKEINTRESOURCE(IDR_IMAGE2), MAKEINTRESOURCE(12345), NULL, TRUE);
		if (png2)
		{
			GetObject(png2, sizeof(info), &info);
			png2_w = info.bmWidth;
			png2_h = info.bmHeight;
		}
	}

	if (id == WM_PAINT)
	{
		PAINTSTRUCT ps;
		HDC dc_win;
		HDC dc_mem;
		
		dc_win = BeginPaint(hwnd, &ps);
		dc_mem = CreateCompatibleDC(dc_win);

		{
			BLENDFUNCTION bf = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };

			SelectObject(dc_mem, png1);
			AlphaBlend(dc_win, 0, 0, png1_w, png1_h, 
			           dc_mem, 0, 0, png1_w, png1_h, bf);

			SelectObject(dc_mem, png2);
			AlphaBlend(dc_win, 0, png1_h + 10, png2_w, png2_h, 
			           dc_mem, 0, 0, png2_w, png2_h, bf);
		}

		DeleteDC(dc_mem);
		EndPaint(hwnd, &ps);
	}

	if (id == WM_CLOSE)
	{
		PostQuitMessage(0);
	}

	return DefWindowProc(hwnd, id, wp, lp);
}

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE prev_instance, 
                       LPWSTR cmd_line, int show_cmd)
{
	WNDCLASS wc = { 0 };
	MSG msg;

	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = NULL;
	wc.lpszClassName = L"LpngSampleClass";
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1); //NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

	if (! RegisterClass(&wc))
		return 1;

	if (! CreateWindowEx(0, 
			L"LpngSampleClass", 
			L"LpngSample",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
			CW_USEDEFAULT, CW_USEDEFAULT, 
			CW_USEDEFAULT, CW_USEDEFAULT, 
			NULL, NULL, instance, NULL))
		return 2;

	while ( GetMessage(&msg, NULL, 0, 0) )
	{
		if (msg.message == WM_QUIT)
			break;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

