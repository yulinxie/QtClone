// Internal helper: include GDI+ in a way that works with the Windows SDK 7.0A
// headers (which still rely on the min/max macros) without polluting the rest
// of the framework with those macros.
#ifndef QTC_GDIPLUS_INTERNAL_H
#define QTC_GDIPLUS_INTERNAL_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#include <gdiplus.h>
#undef min
#undef max

#endif // QTC_GDIPLUS_INTERNAL_H
