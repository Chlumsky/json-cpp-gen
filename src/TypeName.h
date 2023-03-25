
#pragma once

#include <string>

class TypeName {

public:
    enum Substance {
        ACTUAL,
        VIRTUAL
    };

    inline TypeName() : nameSubstance(VIRTUAL) { }
    inline TypeName(const std::string &body, const std::string &suffix, Substance substance = ACTUAL) : nameBody(body), nameSuffix(suffix), nameSubstance(substance) { }
    inline TypeName(std::string &&body, const std::string &suffix, Substance substance = ACTUAL) : nameBody((std::string &&) body), nameSuffix(suffix), nameSubstance(substance) { }
    inline explicit TypeName(const std::string &body, std::string &&suffix = std::string(), Substance substance = ACTUAL) : nameBody(body), nameSuffix((std::string &&) suffix), nameSubstance(substance) { }
    inline explicit TypeName(std::string &&body, std::string &&suffix = std::string(), Substance substance = ACTUAL) : nameBody((std::string &&) body), nameSuffix((std::string &&) suffix), nameSubstance(substance) { }
    inline TypeName(const std::string &body, Substance substance) : nameBody(body), nameSubstance(substance) { }
    inline TypeName(std::string &&body, Substance substance) : nameBody((std::string &&) body), nameSubstance(substance) { }
    inline const std::string &body() const { return nameBody; }
    inline const std::string &suffix() const { return nameSuffix; }
    inline Substance substance() const { return nameSubstance; }
    std::string fullName() const;
    std::string variableDeclaration(const std::string &variableName) const;
    std::string refArgDeclaration(const std::string &argName) const;
    std::string constRefArgDeclaration(const std::string &argName) const;

private:
    std::string nameBody, nameSuffix;
    Substance nameSubstance;

};
