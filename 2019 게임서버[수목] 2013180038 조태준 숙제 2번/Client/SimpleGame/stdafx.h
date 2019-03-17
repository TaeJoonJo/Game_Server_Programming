#pragma once
#pragma comment(lib, "ws2_32")

#include "targetver.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <Windows.h>

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <ctime>
#include <random>

#include <vector>
#include <iterator>

#include "Defines.h"
#include "Protocols.h"

using namespace std;