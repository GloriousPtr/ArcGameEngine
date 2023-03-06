#pragma once

#include <Arc/Core/Base.h>

#define ThrowIfFailed(fn, msg)	{\
									HRESULT hr = fn;\
									ARC_CORE_ASSERT(SUCCEEDED(hr), msg); \
								}
