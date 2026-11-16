// QtClone - QtGlobal.cpp
#include <qtc/QtGlobal.h>
#include <windows.h>
#include <stdio.h>

void qtcDebugMessage(const wchar_t* msg) {
    if (!msg) return;
    OutputDebugStringW(msg);
    OutputDebugStringW(L"\n");
    fwprintf_s(stderr, L"%s\n", msg);
    fflush(stderr);
}

qtcDebugStream::qtcDebugStream() {}

qtcDebugStream::~qtcDebugStream() {
    qtcDebugMessage(buf_.c_str());
}

qtcDebugStream& qtcDebugStream::operator<<(const wchar_t* s) {
    if (s) buf_ += s;
    return *this;
}

qtcDebugStream& qtcDebugStream::operator<<(const std::wstring& s) {
    buf_ += s;
    return *this;
}

qtcDebugStream& qtcDebugStream::operator<<(const char* s) {
    if (s) { size_t n = strlen(s); for (size_t i = 0; i < n; ++i) buf_ += (wchar_t)(unsigned char)s[i]; }
    return *this;
}

qtcDebugStream& qtcDebugStream::operator<<(int v) {
    wchar_t b[32]; _snwprintf_s(b, 32, _TRUNCATE, L"%d", v); buf_ += b; return *this;
}

qtcDebugStream& qtcDebugStream::operator<<(unsigned int v) {
    wchar_t b[32]; _snwprintf_s(b, 32, _TRUNCATE, L"%u", v); buf_ += b; return *this;
}

qtcDebugStream& qtcDebugStream::operator<<(qint64 v) {
    wchar_t b[40]; _snwprintf_s(b, 40, _TRUNCATE, L"%I64d", v); buf_ += b; return *this;
}

qtcDebugStream& qtcDebugStream::operator<<(double v) {
    wchar_t b[64]; _snwprintf_s(b, 64, _TRUNCATE, L"%g", v); buf_ += b; return *this;
}
