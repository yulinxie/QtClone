// qtcmoc.cpp - the meta object compiler of QtClone.
//
// Reads a C++ header and writes moc_<header>.cpp containing:
//   * the class' static meta object table (methods, properties)
//   * metaObject() / qt_metacast() / qt_metacall() / qt_static_metacall()
//   * bodies for every signal (they call QMetaObject::activate)
//
// Supported subset of the Qt syntax (documented in docs/MOC.md):
//   Q_OBJECT, Q_PROPERTY(type name READ g [WRITE s] [NOTIFY n]
//                        [DESIGNABLE bool] [CONSTANT]),
//   signals:, slots:, Q_SIGNALS:, Q_SLOTS:, Q_INVOKABLE
//
// The tool is itself built with the VS2010 toolchain (C++03), so it only uses
// conservative language features.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

namespace {

// ---------------------------------------------------------------------------
// string helpers
// ---------------------------------------------------------------------------
// property flag values, mirrored by QMetaObject::PropertyFlags
const int MetaReadable = 0x1;
const int MetaWritable = 0x2;
const int MetaDesignable = 0x8;
const int MetaConstant = 0x400;
const int MetaNotify = 0x80;

std::string numToStr(int v) {
    char b[32];
    _snprintf(b, 32, "%d", v);
    b[31] = 0;
    return std::string(b);
}

std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) ++b;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) --e;
    return s.substr(b, e - b);
}

bool startsWith(const std::string& s, const char* p) {
    size_t n = strlen(p);
    return s.size() >= n && s.compare(0, n, p) == 0;
}

bool endsWith(const std::string& s, const char* p) {
    size_t n = strlen(p);
    return s.size() >= n && s.compare(s.size() - n, n, p) == 0;
}

bool isIdentChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

bool isIdentStart(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

std::string normalizeType(const std::string& in) {
    // collapse whitespace, glue '*' and '&' to the preceding type name
    std::string out;
    bool pendingSpace = false;
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { pendingSpace = true; continue; }
        if (c == '*' || c == '&') {
            if (!out.empty() && out[out.size() - 1] == ' ') out.erase(out.size() - 1);
            out += c;
            pendingSpace = false;
            continue;
        }
        if (c == '<' || c == '>' || c == ',' || c == '(' || c == ')') {
            if (!out.empty() && out[out.size() - 1] == ' ' && c != '<') out.erase(out.size() - 1);
            out += c;
            pendingSpace = false;
            continue;
        }
        if (pendingSpace && !out.empty()) {
            char prev = out[out.size() - 1];
            if ((isIdentChar(prev) || prev == '>' || prev == '*') && isIdentChar(c)) out += ' ';
            else if (prev == ',') out += ' ';
        }
        pendingSpace = false;
        out += c;
    }
    return out;
}

// split on top level commas ('<' '>' nesting and parentheses)
std::vector<std::string> splitTopLevel(const std::string& s, char sep) {
    std::vector<std::string> out;
    int angle = 0, paren = 0;
    std::string cur;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '<') ++angle;
        else if (c == '>') --angle;
        else if (c == '(') ++paren;
        else if (c == ')') --paren;
        if (c == sep && angle == 0 && paren == 0) {
            out.push_back(trim(cur));
            cur.clear();
        } else {
            cur += c;
        }
    }
    if (!trim(cur).empty() || !out.empty()) out.push_back(trim(cur));
    return out;
}

// remove "= defaultvalue" at top level
std::string stripDefault(const std::string& s) {
    int angle = 0, paren = 0, bracket = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '<') ++angle;
        else if (c == '>') --angle;
        else if (c == '(') ++paren;
        else if (c == ')') --paren;
        else if (c == '[') ++bracket;
        else if (c == ']') --bracket;
        else if (c == '=' && angle == 0 && paren == 0 && bracket == 0) return trim(s.substr(0, i));
    }
    return trim(s);
}

bool isBuiltinTypeWord(const std::string& w) {
    static const char* kWords[] = {
        "void", "bool", "char", "short", "int", "long", "float", "double",
        "unsigned", "signed", "wchar_t", "size_t", "const", "volatile", 0
    };
    for (int i = 0; kWords[i]; ++i) if (w == kWords[i]) return true;
    return false;
}

// Extract the type from a parameter declaration, dropping the parameter name.
// "const QString& text" -> "const QString&" ; "int" -> "int"
std::string paramType(const std::string& p) {
    std::string s = normalizeType(stripDefault(p));
    if (s.empty()) return s;
    // locate word tokens (with their end offsets)
    std::vector<std::pair<std::string, size_t> > words;
    size_t i = 0;
    while (i < s.size()) {
        if (isIdentStart(s[i])) {
            size_t b = i;
            while (i < s.size() && isIdentChar(s[i])) ++i;
            words.push_back(std::make_pair(s.substr(b, i - b), b));
        } else {
            ++i;
        }
    }
    if (words.size() < 2) return s;
    const std::string& last = words[words.size() - 1].first;
    if (isBuiltinTypeWord(last)) return s;
    size_t lastStart = words[words.size() - 1].second;
    std::string prefix = trim(s.substr(0, lastStart));
    if (prefix.empty()) return s;
    std::string prefixLastWord;
    for (size_t k = 0; k < words.size() - 1; ++k) prefixLastWord = words[k].first;
    if (isBuiltinTypeWord(prefixLastWord) && (prefixLastWord == "const" || prefixLastWord == "volatile")) return s;
    return prefix;
}

// ---------------------------------------------------------------------------
// source preprocessing
// ---------------------------------------------------------------------------
std::string stripComments(const std::string& src) {
    std::string out = src;
    bool lineComment = false, blockComment = false, inString = false, inChar = false;
    for (size_t i = 0; i < out.size(); ++i) {
        char c = out[i];
        char n = (i + 1 < out.size()) ? out[i + 1] : 0;
        if (lineComment) {
            if (c == '\n') lineComment = false;
            else out[i] = ' ';
            continue;
        }
        if (blockComment) {
            if (c == '*' && n == '/') { out[i] = ' '; out[i + 1] = ' '; ++i; blockComment = false; }
            else if (c != '\n') out[i] = ' ';
            continue;
        }
        if (inString) {
            if (c == '\\') { ++i; continue; }
            if (c == '"') inString = false;
            continue;
        }
        if (inChar) {
            if (c == '\\') { ++i; continue; }
            if (c == '\'') inChar = false;
            continue;
        }
        if (c == '/' && n == '/') { out[i] = ' '; out[i + 1] = ' '; ++i; lineComment = true; continue; }
        if (c == '/' && n == '*') { out[i] = ' '; out[i + 1] = ' '; ++i; blockComment = true; continue; }
        if (c == '"') { inString = true; continue; }
        if (c == '\'') { inChar = true; continue; }
    }
    return out;
}

std::string stripPreprocessor(const std::string& src) {
    std::string out = src;
    size_t i = 0;
    while (i < out.size()) {
        size_t lineStart = i;
        size_t lineEnd = out.find('\n', i);
        if (lineEnd == std::string::npos) lineEnd = out.size();
        size_t p = lineStart;
        while (p < lineEnd && (out[p] == ' ' || out[p] == '\t')) ++p;
        if (p < lineEnd && out[p] == '#') {
            for (size_t k = lineStart; k < lineEnd; ++k) out[k] = ' ';
            // continuation lines
            while (lineEnd > 0 && out[lineEnd - 1] == '\\') {
                out[lineEnd - 1] = ' ';
                size_t next = out.find('\n', lineEnd);
                if (next == std::string::npos) { lineEnd = out.size(); break; }
                for (size_t k = lineEnd; k < next; ++k) out[k] = ' ';
                lineEnd = next;
            }
        }
        i = lineEnd + 1;
        if (i > out.size()) break;
    }
    return out;
}

// ---------------------------------------------------------------------------
// parsed model
// ---------------------------------------------------------------------------
struct MethodInfo {
    std::string name;
    std::string ret;
    int type;                  // Qt::Signal / Qt::Slot / Qt::Method
    int access;                // 0 public, 1 protected, 2 private
    std::vector<std::string> params;
    bool isConst;
    bool pureVirtual;
    MethodInfo() : type(0), access(0), isConst(false), pureVirtual(false) {}
};

struct PropInfo {
    std::string type;
    std::string name;
    std::string getter;
    std::string setter;
    std::string notify;
    bool designable;
    bool constant;
    PropInfo() : designable(true), constant(false) {}
};

struct ClassInfo {
    std::string name;
    std::string base;
    std::vector<MethodInfo> methods;
    std::vector<PropInfo> props;
    ClassInfo() {}
};

bool isSectionHeader(const std::string& t, int* kind) {
    // kind: 0 methods, 1 signals, 2 slots
    std::string s = t;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == '\n' || s[i] == '\t') s[i] = ' ';
    if (s == "public" || s == "private" || s == "protected" || s == "public:" ||
        s == "private:" || s == "protected:") { *kind = 0; return true; }
    if (s == "signals" || s == "Q_SIGNALS" || s == "signals:" || s == "Q_SIGNALS:" ||
        s == "public Q_SIGNALS" || s == "Q_SIGNALS public") { *kind = 1; return true; }
    if (s == "slots" || s == "Q_SLOTS" || s == "slots:" || s == "Q_SLOTS:" ||
        s == "public slots" || s == "private slots" || s == "protected slots" ||
        s == "public Q_SLOTS" || s == "private Q_SLOTS" || s == "protected Q_SLOTS" ||
        s == "public slots:" || s == "private slots:" || s == "protected slots:" ||
        s == "public Q_SLOTS:" || s == "private Q_SLOTS:" || s == "protected Q_SLOTS:") {
        *kind = 2;
        return true;
    }
    return false;
}

int accessFromSection(int kind, const std::string& t, int prevAccess) {
    if (t.find("private") != std::string::npos) return 2;
    if (t.find("protected") != std::string::npos) return 1;
    if (t.find("public") != std::string::npos) return 0;
    return prevAccess;
}

std::string stripMethodPrefixes(std::string t) {
    bool changed = true;
    while (changed) {
        changed = false;
        static const char* kPrefixes[] = {
            "virtual ", "explicit ", "inline ", "static ", "Q_INVOKABLE ",
            "QTC_EXPORT ", "Q_DECL_OVERRIDE", "Q_DECL_FINAL", 0
        };
        for (int i = 0; kPrefixes[i]; ++i) {
            size_t n = strlen(kPrefixes[i]);
            if (t.size() >= n && t.compare(0, n, kPrefixes[i]) == 0) {
                t = trim(t.substr(n));
                changed = true;
            }
        }
    }
    return t;
}

bool parseMethod(const std::string& declIn, const std::string& className,
                 int kind, int access, MethodInfo* out) {
    std::string decl = stripMethodPrefixes(declIn);
    if (decl.empty()) return false;
    size_t open = decl.find('(');
    if (open == std::string::npos) return false;

    std::string head = trim(decl.substr(0, open));
    // method name: last identifier of head
    size_t nameEnd = head.size();
    size_t nameStart = nameEnd;
    while (nameStart > 0 && isIdentChar(head[nameStart - 1])) --nameStart;
    if (nameStart == nameEnd) return false;
    std::string name = head.substr(nameStart, nameEnd - nameStart);
    std::string ret = trim(head.substr(0, nameStart));

    if (name == className || (!name.empty() && name[0] == '~')) return false;
    if (ret.find('=') != std::string::npos) return false;      // e.g. function pointer typedef
    if (ret.empty()) return false;                             // constructors handled above

    size_t depth = 0, close = std::string::npos;
    for (size_t i = open; i < decl.size(); ++i) {
        if (decl[i] == '(') ++depth;
        else if (decl[i] == ')') { if (--depth == 0) { close = i; break; } }
    }
    if (close == std::string::npos) return false;
    std::string paramText = decl.substr(open + 1, close - open - 1);
    std::string tail = trim(decl.substr(close + 1));

    MethodInfo m;
    m.name = name;
    m.ret = normalizeType(ret);
    if (m.ret == "virtual") m.ret = "void";
    m.type = (kind == 1) ? 1 : (kind == 2 ? 2 : 0);
    m.access = access;
    m.isConst = (tail.find("const") != std::string::npos);
    m.pureVirtual = (tail.find("= 0") != std::string::npos || tail.find("=0") != std::string::npos);

    if (!trim(paramText).empty() && trim(paramText) != "void") {
        std::vector<std::string> parts = splitTopLevel(paramText, ',');
        for (size_t i = 0; i < parts.size(); ++i) {
            std::string pt = paramType(parts[i]);
            if (pt.empty()) continue;
            m.params.push_back(pt);
        }
    }
    *out = m;
    return true;
}

void parseProperty(const std::string& decl, ClassInfo* ci) {
    size_t open = decl.find('(');
    size_t close = decl.rfind(')');
    if (open == std::string::npos || close == std::string::npos || close <= open) return;
    std::string inner = trim(decl.substr(open + 1, close - open - 1));
    std::vector<std::string> toks;
    std::string cur;
    for (size_t i = 0; i < inner.size(); ++i) {
        char c = inner[i];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!cur.empty()) { toks.push_back(cur); cur.clear(); }
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) toks.push_back(cur);
    if (toks.size() < 2) return;
    PropInfo p;
    p.type = normalizeType(toks[0]);
    p.name = toks[1];
    for (size_t i = 2; i < toks.size(); ++i) {
        const std::string& k = toks[i];
        if (k == "READ" && i + 1 < toks.size()) p.getter = toks[++i];
        else if (k == "WRITE" && i + 1 < toks.size()) p.setter = toks[++i];
        else if (k == "MEMBER" && i + 1 < toks.size()) p.setter = std::string(); // read-only accessor
        else if (k == "NOTIFY" && i + 1 < toks.size()) p.notify = toks[++i];
        else if (k == "DESIGNABLE" && i + 1 < toks.size()) {
            std::string v = toks[++i];
            p.designable = (v != "false" && v != "0");
        } else if (k == "CONSTANT") p.constant = true;
        else if (k == "RESET" && i + 1 < toks.size()) ++i;
        else if (k == "STORED" || k == "SCRIPTABLE" || k == "USER" || k == "FINAL") {
            if (i + 1 < toks.size() && (toks[i + 1] == "true" || toks[i + 1] == "false" ||
                                        toks[i + 1] == "0" || toks[i + 1] == "1")) ++i;
        }
    }
    if (!p.getter.empty()) ci->props.push_back(p);
}

// ---------------------------------------------------------------------------
// class body parser
// ---------------------------------------------------------------------------
void parseClassBody(const std::string& body, ClassInfo* ci) {
    size_t i = 0;
    std::string decl;
    int parenDepth = 0;
    int kind = 0;      // 0 methods, 1 signals, 2 slots
    int access = 0;
    bool sawQObject = false;

    while (i < body.size()) {
        char c = body[i];

        if (parenDepth > 0) {
            if (c == '(') ++parenDepth;
            else if (c == ')') --parenDepth;
            decl += c;
            ++i;
            if (parenDepth == 0) {
                std::string t = trim(decl);
                if (startsWith(t, "Q_PROPERTY")) { parseProperty(t, ci); decl.clear(); }
                else if (startsWith(t, "Q_ENUMS") || startsWith(t, "Q_FLAGS") ||
                         startsWith(t, "Q_CLASSINFO") || startsWith(t, "Q_ENUM") ||
                         startsWith(t, "Q_FLAG")) {
                    decl.clear();
                }
            }
            continue;
        }

        if (c == '(') { parenDepth = 1; decl += c; ++i; continue; }

        if (c == ':') {
            std::string t = trim(decl);
            decl.clear();
            ++i;
            if (t.size() >= 8 && t.compare(0, 8, "Q_OBJECT") == 0) t = trim(t.substr(8));
            if (isSectionHeader(t, &kind)) {
                access = accessFromSection(kind, t, access);
                continue;
            }
            // something like "enum Foo :" - ignore
            continue;
        }

        if (c == ';') {
            std::string t = trim(decl);
            decl.clear();
            ++i;
            if (t.find("Q_OBJECT") != std::string::npos) { sawQObject = true; continue; }
            if (t.empty()) continue;
            if (startsWith(t, "Q_PROPERTY")) { parseProperty(t, ci); continue; }
            if (startsWith(t, "Q_ENUM") || startsWith(t, "Q_FLAG") || startsWith(t, "Q_DECLARE")) continue;
            if (startsWith(t, "typedef") || startsWith(t, "using") || startsWith(t, "friend") ||
                startsWith(t, "enum") || startsWith(t, "class ") || startsWith(t, "struct ") ||
                startsWith(t, "static const") || startsWith(t, "static constexpr")) continue;
            MethodInfo m;
            bool invokable = t.find("Q_INVOKABLE") != std::string::npos;
            if (parseMethod(t, ci->name, kind, access, &m)) {
                if (m.pureVirtual) continue;
                if (kind == 0 && !invokable) continue;   // plain member functions are not in the meta object
                ci->methods.push_back(m);
            }
            continue;
        }

        if (c == '{') {
            // inline function body or nested type: skip to matching brace
            int depth = 0;
            while (i < body.size()) {
                if (body[i] == '{') ++depth;
                else if (body[i] == '}') {
                    --depth;
                    if (depth == 0) { ++i; break; }
                }
                ++i;
            }
            decl.clear();
            continue;
        }

        if (c == '}') { ++i; break; }

        decl += c;
        ++i;
        std::string t = trim(decl);
        if (t == "Q_OBJECT" || t == "Q_OBJECT;") { sawQObject = true; decl.clear(); }
    }
    (void)sawQObject;
}

// find classes with Q_OBJECT at file scope
void parseTranslationUnit(const std::string& src, std::vector<ClassInfo>& out) {
    size_t i = 0;
    int braceDepth = 0;
    while (i < src.size()) {
        char c = src[i];
        if (c == '{') { ++braceDepth; ++i; continue; }
        if (c == '}') { if (braceDepth > 0) --braceDepth; ++i; continue; }

        if (braceDepth == 0 && isIdentStart(c)) {
            size_t b = i;
            while (i < src.size() && isIdentChar(src[i])) ++i;
            std::string word = src.substr(b, i - b);
            if (word == "class" || word == "struct") {
                // read up to '{' or ';'
                size_t j = i;
                std::string header;
                int angle = 0;
                while (j < src.size()) {
                    char d = src[j];
                    if (d == '<') ++angle;
                    else if (d == '>') --angle;
                    else if (d == '{' && angle == 0) break;
                    else if (d == ';' && angle == 0) break;
                    header += d;
                    ++j;
                }
                if (j < src.size() && src[j] == ';') { i = j + 1; continue; }

                ClassInfo ci;
                // class name = last identifier before ':' or end
                std::string head = header;
                size_t colon = head.find(':');
                std::string namePart = (colon == std::string::npos) ? head : head.substr(0, colon);
                size_t k = namePart.size();
                while (k > 0 && !isIdentChar(namePart[k - 1])) --k;
                size_t k2 = k;
                while (k2 > 0 && isIdentChar(namePart[k2 - 1])) --k2;
                ci.name = namePart.substr(k2, k - k2);

                if (colon != std::string::npos) {
                    std::string bases = head.substr(colon + 1);
                    std::vector<std::string> parts = splitTopLevel(bases, ',');
                    for (size_t p = 0; p < parts.size(); ++p) {
                        std::string b2 = trim(parts[p]);
                        if (b2.size() > 7 && b2.compare(0, 7, "public ") == 0) b2 = trim(b2.substr(7));
                        if (b2.size() > 8 && b2.compare(0, 8, "virtual ") == 0) b2 = trim(b2.substr(8));
                        if (!b2.empty()) { ci.base = b2; break; }
                    }
                }

                // body
                if (j < src.size() && src[j] == '{') {
                    size_t bodyStart = j + 1;
                    int depth = 1;
                    size_t bo = bodyStart;
                    while (bo < src.size() && depth > 0) {
                        if (src[bo] == '{') ++depth;
                        else if (src[bo] == '}') --depth;
                        if (depth == 0) break;
                        ++bo;
                    }
                    std::string body = src.substr(bodyStart, bo - bodyStart);
                    if (body.find("Q_OBJECT") != std::string::npos && !ci.name.empty()) {
                        parseClassBody(body, &ci);
                        if (!ci.methods.empty() || !ci.props.empty() || ci.base == "QObject")
                            out.push_back(ci);
                        else
                            out.push_back(ci);
                    }
                    i = (bo < src.size()) ? bo + 1 : src.size();
                    continue;
                }
                i = j;
                continue;
            }
            continue;
        }
        ++i;
    }
}

// ---------------------------------------------------------------------------
// code generation
// ---------------------------------------------------------------------------
const char* typeNameOf(int t) {
    if (t == 1) return "Qt::Signal";
    if (t == 2) return "Qt::Slot";
    return "Qt::Method";
}

std::string simpleName(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (isIdentChar(c)) out += c;
        else out += '_';
    }
    return out;
}

std::string cppStringLiteral(const std::string& s) {
    std::string out = "\"";
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '\\') out += "\\\\";
        else if (c == '"') out += "\\\"";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else out += c;
    }
    out += "\"";
    return out;
}

// The argument slot of a reference-to-const parameter holds a value of the
// underlying type, so casts are generated against that storage type.
std::string storageType(const std::string& t) {
    if (t.find('&') == std::string::npos) return t;
    std::string s = t;
    for (;;) {
        size_t p = s.find('&');
        if (p == std::string::npos) break;
        s.erase(p, 1);
    }
    s = trim(s);
    while (startsWith(s, "const ")) s = trim(s.substr(6));
    return s;
}

std::string argCast(const std::string& type, int index) {
    std::string cast = "reinterpret_cast<" + storageType(type) + "(*)>(argv[" + numToStr(index) + "])";
    return "(*" + cast + ")";
}

std::string indent(int n) { return std::string(n * 4, ' '); }

void generateClass(FILE* f, const ClassInfo& ci) {
    const std::string& C = ci.name;
    bool root = ci.base.empty() || ci.base == ci.name;
    std::string base = root ? std::string("QObject") : ci.base;
    std::string prefix = "qtc_meta_" + simpleName(C);

    fprintf(f, "// generated by qtcmoc from %s - do not edit\n\n", C.c_str());

    // parameter type tables
    for (size_t mi = 0; mi < ci.methods.size(); ++mi) {
        const MethodInfo& m = ci.methods[mi];
        if (m.params.empty()) continue;
        fprintf(f, "static const char* const %s_pt_%d[] = {", prefix.c_str(), (int)mi);
        for (size_t p = 0; p < m.params.size(); ++p) {
            if (p) fprintf(f, ", ");
            fprintf(f, "%s", cppStringLiteral(storageType(m.params[p])).c_str());
        }
        fprintf(f, " };\n");
    }
    fprintf(f, "\n");

    if (!ci.methods.empty()) {
        fprintf(f, "static const QMetaObject::MethodData %s_methods[] = {\n", prefix.c_str());
        for (size_t mi = 0; mi < ci.methods.size(); ++mi) {
            const MethodInfo& m = ci.methods[mi];
            std::string sig = m.name + "(";
            for (size_t p = 0; p < m.params.size(); ++p) {
                if (p) sig += ",";
                sig += storageType(m.params[p]);
            }
            sig += ")";
            fprintf(f, "    { %s, %s, %s, %d, %s, %s, %d },\n",
                    cppStringLiteral(m.name).c_str(),
                    cppStringLiteral(sig).c_str(),
                    cppStringLiteral(m.ret).c_str(),
                    (int)m.params.size(),
                    m.params.empty() ? "0" : ("qtc_meta_" + simpleName(C) + "_pt_" + numToStr((int)mi)).c_str(),
                    typeNameOf(m.type),
                    m.access);
        }
        fprintf(f, "};\n\n");
    }

    if (!ci.props.empty()) {
        fprintf(f, "static const QMetaObject::PropertyData %s_props[] = {\n", prefix.c_str());
        for (size_t pi = 0; pi < ci.props.size(); ++pi) {
            const PropInfo& p = ci.props[pi];
            int flags = MetaReadable;
            if (!p.setter.empty()) flags |= MetaWritable;
            if (p.designable) flags |= MetaDesignable;
            if (p.constant) flags |= MetaConstant;
            if (!p.notify.empty()) flags |= MetaNotify;
            fprintf(f, "    { %s, %s, 0x%x, %s, %s, %s, 0 },\n",
                    cppStringLiteral(p.name).c_str(),
                    cppStringLiteral(p.type).c_str(),
                    flags,
                    cppStringLiteral(p.getter).c_str(),
                    p.setter.empty() ? "0" : cppStringLiteral(p.setter).c_str(),
                    p.notify.empty() ? "0" : cppStringLiteral(p.notify).c_str());
        }
        fprintf(f, "};\n\n");
    }

    fprintf(f, "const QMetaObject %s::staticMetaObject = {\n", C.c_str());
    fprintf(f, "    %s, %s,\n", root ? "0" : ("&" + base + "::staticMetaObject").c_str(),
            cppStringLiteral(C).c_str());
    fprintf(f, "    %s, %d, %s, %d\n",
            ci.methods.empty() ? "0" : (prefix + "_methods").c_str(), (int)ci.methods.size(),
            ci.props.empty() ? "0" : (prefix + "_props").c_str(), (int)ci.props.size());
    fprintf(f, "};\n\n");

    fprintf(f, "const QMetaObject* %s::metaObject() const { return &%s::staticMetaObject; }\n\n", C.c_str(), C.c_str());

    fprintf(f, "void* %s::qt_metacast(const char* name) {\n", C.c_str());
    fprintf(f, "    if (!name) return 0;\n");
    fprintf(f, "    if (strcmp(name, %s) == 0) return (void*)this;\n", cppStringLiteral(C).c_str());
    if (root) fprintf(f, "    return 0;\n");
    else fprintf(f, "    return %s::qt_metacast(name);\n", base.c_str());
    fprintf(f, "}\n\n");

    fprintf(f, "int %s::qt_metacall(QMetaObject::Call call, int id, void** argv) {\n", C.c_str());
    if (!root) {
        fprintf(f, "    id = %s::qt_metacall(call, id, argv);\n", base.c_str());
        fprintf(f, "    if (id < 0) return id;\n");
    }
    fprintf(f, "    if (call == QMetaObject::InvokeMetaMethod) {\n");
    if (!ci.methods.empty()) {
        fprintf(f, "        if (id < %d) qt_static_metacall(this, call, id, argv);\n", (int)ci.methods.size());
        fprintf(f, "        id -= %d;\n", (int)ci.methods.size());
    }
    fprintf(f, "    } else if (call == QMetaObject::ReadProperty || call == QMetaObject::WriteProperty ||\n");
    fprintf(f, "               call == QMetaObject::ResetProperty) {\n");
    if (!ci.props.empty()) {
        fprintf(f, "        if (id < %d) qt_static_metacall(this, call, id, argv);\n", (int)ci.props.size());
        fprintf(f, "        id -= %d;\n", (int)ci.props.size());
    }
    fprintf(f, "    }\n");
    fprintf(f, "    return id;\n");
    fprintf(f, "}\n\n");

    // qt_static_metacall
    fprintf(f, "void %s::qt_static_metacall(QObject* obj, QMetaObject::Call call, int id, void** argv) {\n", C.c_str());
    fprintf(f, "    %s* self = static_cast<%s*>(obj);\n", C.c_str(), C.c_str());
    fprintf(f, "    (void)self;\n");
    fprintf(f, "    if (call == QMetaObject::InvokeMetaMethod) {\n");
    if (!ci.methods.empty()) {
        fprintf(f, "        switch (id) {\n");
        for (size_t mi = 0; mi < ci.methods.size(); ++mi) {
            const MethodInfo& m = ci.methods[mi];
            fprintf(f, "        case %d:\n", (int)mi);
            std::string callExpr = "self->" + m.name + "(";
            for (size_t p = 0; p < m.params.size(); ++p) {
                if (p) callExpr += ", ";
                callExpr += argCast(m.params[p], (int)p + 1);
            }
            callExpr += ")";
            if (m.ret == "void") {
                fprintf(f, "            %s; break;\n", callExpr.c_str());
            } else {
                fprintf(f, "            { %s __r = %s; if (argv[0]) *reinterpret_cast<%s*>(argv[0]) = __r; } break;\n",
                        m.ret.c_str(), callExpr.c_str(), m.ret.c_str());
            }
        }
        fprintf(f, "        default: break;\n");
        fprintf(f, "        }\n");
    }
    fprintf(f, "    } else if (call == QMetaObject::ReadProperty) {\n");
    if (!ci.props.empty()) {
        fprintf(f, "        switch (id) {\n");
        for (size_t pi = 0; pi < ci.props.size(); ++pi) {
            const PropInfo& p = ci.props[pi];
            fprintf(f, "        case %d: *reinterpret_cast<%s*>(argv[0]) = self->%s(); break;\n",
                    (int)pi, storageType(p.type).c_str(), p.getter.c_str());
        }
        fprintf(f, "        default: break;\n        }\n");
    }
    fprintf(f, "    } else if (call == QMetaObject::WriteProperty) {\n");
    {
        bool any = false;
        for (size_t pi = 0; pi < ci.props.size(); ++pi) if (!ci.props[pi].setter.empty()) any = true;
        if (any) {
            fprintf(f, "        switch (id) {\n");
            for (size_t pi = 0; pi < ci.props.size(); ++pi) {
                const PropInfo& p = ci.props[pi];
                if (p.setter.empty()) continue;
                fprintf(f, "        case %d: self->%s(*reinterpret_cast<%s*>(argv[0])); break;\n",
                        (int)pi, p.setter.c_str(), storageType(p.type).c_str());
            }
            fprintf(f, "        default: break;\n        }\n");
        }
    }
    fprintf(f, "    }\n");
    fprintf(f, "}\n\n");

    // signal bodies
    for (size_t mi = 0; mi < ci.methods.size(); ++mi) {
        const MethodInfo& m = ci.methods[mi];
        if (m.type != 1) continue;
        std::string params, argvDecl, argvList = "0";
        for (size_t p = 0; p < m.params.size(); ++p) {
            if (p) params += ", ";
            params += m.params[p] + " _t" + numToStr((int)p + 1);
            argvList += ", const_cast<void*>(reinterpret_cast<const void*>(&_t" +
                        numToStr((int)p + 1) + "))";
        }
        fprintf(f, "void %s::%s(%s) {\n", C.c_str(), m.name.c_str(), params.c_str());
        fprintf(f, "    void* _a[] = { %s };\n", argvList.c_str());
        fprintf(f, "    QMetaObject::activate(this, &staticMetaObject, %d, _a);\n", (int)mi);
        fprintf(f, "}\n\n");
    }
}

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> inputs;
    std::string outFile;
    bool quiet = false;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "-o" && i + 1 < argc) { outFile = argv[++i]; continue; }
        if (a == "-q") { quiet = true; continue; }
        if (a == "--version") { printf("qtcmoc 1.0 (QtClone)\n"); return 0; }
        if (a == "--help" || a == "-h") {
            printf("usage: qtcmoc [-o outfile] [-q] header.h [more.h ...]\n");
            return 0;
        }
        inputs.push_back(a);
    }
    if (inputs.empty()) {
        fprintf(stderr, "qtcmoc: no input files\n");
        return 1;
    }

    int failures = 0;
    for (size_t fi = 0; fi < inputs.size(); ++fi) {
        const std::string& in = inputs[fi];
        FILE* fp = fopen(in.c_str(), "rb");
        if (!fp) {
            fprintf(stderr, "qtcmoc: cannot open %s\n", in.c_str());
            ++failures;
            continue;
        }
        std::string src;
        char buf[8192];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) src.append(buf, n);
        fclose(fp);

        std::string cleaned = stripPreprocessor(stripComments(src));
        std::vector<ClassInfo> classes;
        parseTranslationUnit(cleaned, classes);

        std::string baseName = in;
        size_t slash = baseName.find_last_of("/\\");
        if (slash != std::string::npos) baseName = baseName.substr(slash + 1);
        size_t dot = baseName.rfind('.');
        if (dot != std::string::npos) baseName = baseName.substr(0, dot);

        std::string output = outFile;
        if (output.empty()) {
            std::string dir;
            if (slash != std::string::npos) dir = in.substr(0, slash + 1);
            output = dir + "moc_" + baseName + ".cpp";
        }

        FILE* of = fopen(output.c_str(), "wb");
        if (!of) {
            fprintf(stderr, "qtcmoc: cannot write %s\n", output.c_str());
            ++failures;
            continue;
        }
        std::string includePath = in;
        for (size_t k = 0; k < includePath.size(); ++k) if (includePath[k] == '\\') includePath[k] = '/';
        fprintf(of, "#include \"%s\"\n#include <string.h>\n\n", includePath.c_str());
        for (size_t c = 0; c < classes.size(); ++c) generateClass(of, classes[c]);
        fclose(of);

        if (!quiet) {
            printf("qtcmoc: %s -> %s (%d class%s)\n", in.c_str(), output.c_str(),
                   (int)classes.size(), classes.size() == 1 ? "" : "es");
        }
    }
    return failures ? 1 : 0;
}
