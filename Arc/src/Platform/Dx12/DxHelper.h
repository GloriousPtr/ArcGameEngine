#pragma once

#include <comutil.h>
#include <Arc/Core/Base.h>

#define ThrowIfFailed(fn, msg)	{\
									HRESULT hr = fn;\
									ARC_CORE_ASSERT(SUCCEEDED(hr), msg); \
								}

#define NameResource(pResource, cstr)	{\
											_bstr_t wcstr(cstr);\
											(pResource)->SetName(wcstr);\
										}

#define ToWCSTR(cstr) _bstr_t(cstr)
#define ToCSTR(wcstr) static_cast<const char*>(_bstr_t(wcstr))
