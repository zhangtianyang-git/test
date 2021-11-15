#ifndef MYCOMMON_H
#define MYCOMMON_H
#if WIN32
#include "sqlite3.h"
#else
#include "sqlite3.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#define SQLITE_PATH_DCS "/config/dcs.db"
#define SQLITE_PATH_TOOL "/config/tool.db"

using namespace std;
#endif // MYCOMMON_H
