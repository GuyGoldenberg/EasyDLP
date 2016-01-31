#pragma once

#include "targetver.h"

// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <iostream>
#include <string.h>
#include <iomanip>


using namespace std;

/// <summary>
/// Encodes the specified WSTR to a desired encoding.
/// </summary>
/// <param name="wstr">whcar_t string</param>
/// <param name="codePage">codePage represents the encoding type (CP_*)</param>
/// <returns>Encoded string</returns>
char* Encode(const wchar_t* wstr, unsigned int codePage);

