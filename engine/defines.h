#pragma once

#ifndef LINUX
#define API __declspec(dllexport)
#else
#define API
#endif
