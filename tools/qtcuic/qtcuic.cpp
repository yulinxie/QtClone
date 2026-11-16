// qtcuic - command line .ui to C++ header compiler (QtClone).
// usage: qtcuic <input.ui> [-o <output.h>] [--class <FormName>] [--quiet]
#include "uicgen.h"
#include <qtc/QString.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    QString input;
    QString output;
    QString className;
    bool quiet = false;

    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromUtf8(argv[i]);
        if (arg == L"-o" && i + 1 < argc) { output = QString::fromUtf8(argv[++i]); continue; }
        if (arg == L"--class" && i + 1 < argc) { className = QString::fromUtf8(argv[++i]); continue; }
        if (arg == L"-q" || arg == L"--quiet") { quiet = true; continue; }
        if (arg == L"-h" || arg == L"--help") {
            printf("usage: qtcuic <input.ui> [-o <output.h>] [--class <FormName>] [-q]\n");
            return 0;
        }
        if (input.isEmpty()) input = arg;
    }
    if (input.isEmpty()) {
        fprintf(stderr, "qtcuic: no input file\n");
        return 1;
    }

    bool ok = false;
    QString xml = uicReadFile(input, &ok);
    if (!ok) {
        fprintf(stderr, "qtcuic: cannot read %s\n", input.toUtf8().c_str());
        return 1;
    }

    QString error;
    QString header = uicGenerateHeader(xml, className, &error);
    if (header.isEmpty()) {
        fprintf(stderr, "qtcuic: %s: %s\n", input.toUtf8().c_str(), error.toUtf8().c_str());
        return 1;
    }

    if (output.isEmpty()) {
        QString dir;
        QString base = input;
        int slash = base.find_last_of(L"/\\");
        if (slash >= 0) {
            dir = base.left(slash + 1);
            base = base.mid(slash + 1);
        }
        int dot = base.find_last_of(L'.');
        if (dot >= 0) base = base.left(dot);
        output = dir + base + L"_ui.h";
    }

    if (!uicWriteFile(output, header)) {
        fprintf(stderr, "qtcuic: cannot write %s\n", output.toUtf8().c_str());
        return 1;
    }
    if (!quiet) printf("qtcuic: %s -> %s\n", input.toUtf8().c_str(), output.toUtf8().c_str());
    return 0;
}
