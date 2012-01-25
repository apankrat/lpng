+----------------------------------------------------------+
|                                                          |
|        Ultra-light PNG loader library for Windows        |
|                                                          |
|                  http://swapped.cc/lpng                  |
|                                                          |
+----------------------------------------------------------+


  lpng implements loading of truecolor PNG images (with or 
  without an alpha channel) into 32-bit (RGB+A) Windows 
  bitmaps that can be used with AlphaBlend function.

  The .exe of a sample application is only 16KB in size,
  with 3KB taken by the actual PNG stored as resources.

+----------------------------------------------------------+

  License

  The PNG loading code (lpng.c) is written by me, Alex 
  Pankratov, and it is distributed under BSD license. The 
  deflate decoder (puff.c) is written by Mark Adler and 
  distributed under zlib license.

+----------------------------------------------------------+

  The API consists of just one function:

    HBITMAP LoadPng(const wchar_t * resName,
                    const wchar_t * resType,
                    HMODULE         resInst,
                    BOOL   premultiplyAlpha);

  It can be used to load a PNG from a disk file or from 
  a resource. Sample application demonstrating the use of 
  the API can be found in ./sample directory. The project 
  file is for Visual Studio 2005.

  The "premultiply" parameter controls whether the pixels'
  RGB values are 'pre-multiplied' at respective alpha value 
  if the alpha channel is present. This is a pre-processing
  step requied by the AlphaBlend function, so the parameter
  needs to be TRUE if the bitmap is going to be painted on
  a target DC using this function. Otherwise, e.g. if the
  bitmap is going to be used for an ImageList, it needs to
  be FALSE.

+----------------------------------------------------------+
