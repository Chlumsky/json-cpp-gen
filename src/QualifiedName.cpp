
#include "QualifiedName.h"

#include <cctype>
#include <cassert>

static const UnqualifiedName emptyUnqualifiedName;

bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool isNameChar(char c) {
    return (c&0x80) || c == '_' || isalnum(c);
}

bool isInitialNameChar(char c) {
    return (c&0x80) || c == '_' || isalpha(c);
}

bool UnqualifiedName::validate(const std::string &string) {
    if (string.empty())
        return false;
    if (!isInitialNameChar(string[0]))
        return false;
    for (char c : string) {
        if (!(isWhitespace(c) || isNameChar(c)))
            return false;
    }
    return true;
}

UnqualifiedName::UnqualifiedName() = default;

UnqualifiedName::UnqualifiedName(const std::string &string) : str(string) {
    assert(validate(str));
}

UnqualifiedName::UnqualifiedName(std::string &&string) : str((std::string &&) string) {
    assert(validate(str));
}

const std::string &UnqualifiedName::string() const {
    return str;
}

UnqualifiedName::operator const std::string &() const {
    return str;
}

UnqualifiedName::operator bool() const {
    return !str.empty();
}

QualifiedName::Ref::Ref() : start(nullptr), n(0), absolute(false) { }

QualifiedName::Ref::Ref(const QualifiedName &name) : start(name.names.data()), n(int(name.names.size())), absolute(name.absolute) { }

QualifiedName::Ref::Ref(const UnqualifiedName &name) : start(&name), n(1), absolute(false) { }

bool QualifiedName::Ref::isAbsolute() const {
    return absolute;
}

const UnqualifiedName &QualifiedName::Ref::prefix() const {
    return n > 0 ? *start : emptyUnqualifiedName;
}

const UnqualifiedName &QualifiedName::Ref::suffix() const {
    return n > 0 ? start[n-1] : emptyUnqualifiedName;
}

QualifiedName::Ref QualifiedName::Ref::exceptAbsolute() const {
    Ref ref(*this);
    ref.absolute = false;
    return ref;
}

QualifiedName::Ref QualifiedName::Ref::exceptPrefix() const {
    Ref suffixRef;
    suffixRef.start = start+1;
    suffixRef.n = n-1;
    suffixRef.absolute = false;
    return suffixRef;
}

QualifiedName::Ref QualifiedName::Ref::exceptSuffix() const {
    Ref prefixRef;
    prefixRef.start = start;
    prefixRef.n = n-1;
    prefixRef.absolute = absolute;
    return prefixRef;
}

std::string QualifiedName::Ref::string() const {
    std::string str;
    if (n > 0) {
        if (absolute)
            str += "::";
        str += start->string();
        for (const UnqualifiedName *subName = start+1, *end = start+n; subName < end; ++subName) {
            str += "::";
            str += subName->string();
        }
    }
    return str;
}

std::string QualifiedName::Ref::stringPrefix() const {
    std::string str;
    if (absolute)
        str += "::";
    for (const UnqualifiedName *subName = start, *end = start+n; subName < end; ++subName) {
        str += subName->string();
        str += "::";
    }
    return str;
}

bool QualifiedName::Ref::isUnqualified() const {
    return !absolute && n == 1;
}

QualifiedName::Ref::operator bool() const {
    return n > 0;
}

QualifiedName::Ref::operator QualifiedName() const {
    QualifiedName name;
    name.names = std::vector<UnqualifiedName>(start, start+n);
    name.absolute = absolute;
    return name;
}

bool QualifiedName::validate(const std::string &string) {
    if (string.empty())
        return false;
    int colons = 0;
    bool prevAlnum = false;
    for (char c : string) {
        if (isWhitespace(c)) {
            if (colons == 1) // valid if prevAlnum because of unsigned char etc.
                return false;
        } else if (c == ':') {
            if (colons++ >= 2)
                return false;
            prevAlnum = false;
        } else if (isInitialNameChar(c)) {
            if (colons == 1)
                return false;
            colons = 0;
            prevAlnum = true;
        } else if (isNameChar(c)) {
            if (!prevAlnum)
                return false;
        } else
            return false;
    }
    return prevAlnum;
}

QualifiedName::QualifiedName() : absolute(false) { }

QualifiedName::QualifiedName(const std::string &string) : absolute(false) {
    assert(validate(string));
    if (!string.empty()) {
        std::string subName;
        bool prevSpace = false;
        for (char c : string) {
            prevSpace = isWhitespace(c);
            if (c == ':') {
                if (!subName.empty()) {
                    names.push_back((std::string &&) subName);
                    subName = std::string();
                } else if (names.empty())
                    absolute = true;
            } else if (isNameChar(c)) {
                if (prevSpace && !subName.empty())
                    subName.push_back(' ');
                subName.push_back(c);
            }
        }
        if (!subName.empty())
            names.push_back((std::string &&) subName);
    }
}

QualifiedName::QualifiedName(const UnqualifiedName &unqualifiedName) : absolute(false) {
    names.push_back(unqualifiedName);
}

void QualifiedName::setAbsolute(bool absolute) {
    this->absolute = absolute;
}

void QualifiedName::append(const UnqualifiedName &unqualifiedName) {
    if (unqualifiedName)
        names.push_back(unqualifiedName);
}

void QualifiedName::append(UnqualifiedName &&unqualifiedName) {
    if (unqualifiedName)
        names.push_back((UnqualifiedName &&) unqualifiedName);
}

bool QualifiedName::isAbsolute() const {
    return absolute;
}

const UnqualifiedName &QualifiedName::prefix() const {
    return Ref(*this).prefix();
}

const UnqualifiedName &QualifiedName::suffix() const {
    return Ref(*this).suffix();
}

QualifiedName::Ref QualifiedName::exceptAbsolute() const {
    return Ref(*this).exceptAbsolute();
}

QualifiedName::Ref QualifiedName::exceptPrefix() const {
    return Ref(*this).exceptPrefix();
}

QualifiedName::Ref QualifiedName::exceptSuffix() const {
    return Ref(*this).exceptSuffix();
}

std::string QualifiedName::string() const {
    return Ref(*this).string();
}

std::string QualifiedName::stringPrefix() const {
    return Ref(*this).stringPrefix();
}

bool QualifiedName::isUnqualified() const {
    return Ref(*this).isUnqualified();
}

QualifiedName::operator bool() const {
    return !names.empty();
}

bool QualifiedName::operator==(const QualifiedName &other) const {
    if (!(absolute == other.absolute && names.size() == other.names.size()))
        return false;
    for (size_t i = 0; i < names.size(); ++i) {
        if (names[i].string() != other.names[i].string())
            return false;
    }
    return true;
}

QualifiedName operator+(QualifiedName::Ref a, QualifiedName::Ref b) {
    if (b.isAbsolute())
        return QualifiedName(b);
    QualifiedName total(a);
    for (; b; b = b.exceptPrefix())
        total.append(b.prefix());
    return total;
}
