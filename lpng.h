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

/*
 *	lpng.h, version 16-01-2009
 */

#ifndef _LOAD_PNG_H_
#define _LOAD_PNG_H_

/*
 *	To load PNG image from a disk file use:
 *
 *		LoadPng(L"c:\\sample.png", NULL, NULL);
 *
 *	To load from a resource in process's own .exe use:
 *
 *		LoadPng(MAKEINTRESOURCE(IDR_SAMPLE), 
 *		        MAKEINTRESOURCE(1001),
 *		        NULL,
 *		        FALSE);
 *
 *	To load from a resource in some other .exe or .dll use:
 *
 *		module = LoadLibrary(L"c:\\sample.dll");
 *
 *		LoadPng(MAKEINTRESOURCE(IDR_SAMPLE), 
 *		        MAKEINTRESOURCE(1001),
 *		        module, 
 *		        FALSE);
 *
 *	where "1001" is a type of the resource. It is selected,
 *	when the .png image is imported into the resource file
 *	and it can be whatever.
 */

#ifdef __cplusplus
extern "C" {
#endif

HBITMAP LoadPng(const wchar_t * resName,
                const wchar_t * resType,
                HMODULE         resInst,
		BOOL   premultiplyAlpha);

#ifdef __cplusplus
}
#endif

#endif
