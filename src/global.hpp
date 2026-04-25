#pragma once

#define _GLIBC_USE_DEPRECATED 0
#define _GLIBCXX_USE_DEPRECATED 0
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1

#include "sor/adapt_std.hpp"

#include "sor/core.hpp"

#include "sor/adapt_json.hpp"
#include "sor/adapt_nfd.hpp"

using namespace JanSordid::Core;
