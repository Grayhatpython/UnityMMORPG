#pragma once

#define NOMINMAX

#include "Types.h"
#include "CoreMacro.h"
#include "ThreadLocal.h"
#include "CoreGlobal.h"
#include "Container.h"

#include <windows.h>
#include <iostream>
using namespace std;

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "Lock.h"
#include "ObjectPool.h"
#include "Memory.h"
#include "SendBuffer.h"
#include "Session.h"
#include "JobQueueBasedObject.h"
#include "Log.h"
