#pragma once

#include "targetver.h"

// Windows Header Files:
#define _WINSOCKAPI_    // stops windows.h including winsock.h

#include <Windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <iostream>
#include <string.h>
#include <iomanip>
#include <WinSock2.h>

using namespace std;

/// <summary>
/// Encodes the specified WSTR to a desired encoding.
/// </summary>
/// <param name="wstr">whcar_t string</param>
/// <param name="codePage">codePage represents the encoding type (CP_*)</param>
/// <returns>Encoded string</returns>
char* Encode(const wchar_t* wstr, unsigned int codePage);

