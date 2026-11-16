// QtClone - uicgen.h : .ui XML -> C++ header generator (shared by qtcuic and the designer).
#ifndef QTC_UICGEN_H
#define QTC_UICGEN_H

#include <qtc/QString.h>

// Generates the contents of ui_<class>.h from a Qt Designer style .ui document.
// Returns an empty string and fills `error` when the document cannot be parsed.
QString uicGenerateHeader(const QString& uiXml, const QString& className, QString* error = 0);

// Writes UTF-8 text to a file (adds a UTF-8 BOM by default so that MSVC 2010
// reads non-ASCII string literals correctly).
bool uicWriteFile(const QString& path, const QString& text, bool withBom = true);

// Reads a file as UTF-8 (BOM aware).
QString uicReadFile(const QString& path, bool* ok = 0);

#endif // QTC_UICGEN_H
