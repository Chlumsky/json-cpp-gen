
#pragma once

#include <string>
#include <vector>

bool isWhitespace(char c);
bool isNameChar(char c);
bool isInitialNameChar(char c);

/// Only consists of letters, digits, and underscores
class UnqualifiedName {

public:
    static bool validate(const std::string &string);

    UnqualifiedName();
    UnqualifiedName(const std::string &string);
    UnqualifiedName(std::string &&string);
    const std::string &string() const;
    operator const std::string &() const;
    explicit operator bool() const;

private:
    std::string str;

};

/// Sequence of qualified names separated by ::
class QualifiedName {

public:
    class Ref {
    public:
        Ref();
        Ref(const QualifiedName &name);
        explicit Ref(const UnqualifiedName &name);
        bool isAbsolute() const;
        const UnqualifiedName &prefix() const;
        const UnqualifiedName &suffix() const;
        Ref exceptAbsolute() const;
        Ref exceptPrefix() const;
        Ref exceptSuffix() const;
        std::string string() const;
        bool isUnqualified() const;
        explicit operator bool() const;
        explicit operator QualifiedName() const;
    private:
        const UnqualifiedName *start;
        int n;
        bool absolute;
    };

    static bool validate(const std::string &string);

    QualifiedName();
    QualifiedName(const std::string &string);
    explicit QualifiedName(const UnqualifiedName &unqualifiedName);
    void setAbsolute(bool absolute);
    void append(const UnqualifiedName &unqualifiedName);
    void append(UnqualifiedName &&unqualifiedName);

    bool isAbsolute() const;
    const UnqualifiedName &prefix() const;
    const UnqualifiedName &suffix() const;
    Ref exceptAbsolute() const;
    Ref exceptPrefix() const;
    Ref exceptSuffix() const;
    std::string string() const;
    bool isUnqualified() const;
    explicit operator bool() const;
    bool operator==(const QualifiedName &other) const;

private:
    std::vector<UnqualifiedName> names;
    bool absolute;

};

QualifiedName operator+(QualifiedName::Ref a, QualifiedName::Ref b);
