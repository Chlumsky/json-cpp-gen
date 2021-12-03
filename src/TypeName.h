
#pragma once

#include <string>

class TypeName {

public:
    TypeName() = default;
    inline TypeName(const std::string &body, const std::string &suffix) : nameBody(body), nameSuffix(suffix) { }
    inline TypeName(std::string &&body, const std::string &suffix) : nameBody((std::string &&) body), nameSuffix(suffix) { }
    inline explicit TypeName(const std::string &body, std::string &&suffix = std::string()) : nameBody(body), nameSuffix((std::string &&) suffix) { }
    inline explicit TypeName(std::string &&body, std::string &&suffix = std::string()) : nameBody((std::string &&) body), nameSuffix((std::string &&) suffix) { }
    inline const std::string & body() const { return nameBody; }
    inline const std::string & suffix() const { return nameSuffix; }
    std::string fullName() const;
    std::string variableDeclaration(const std::string &variableName) const;
    std::string refArgDeclaration(const std::string &argName) const;
    std::string constRefArgDeclaration(const std::string &argName) const;

private:
    std::string nameBody, nameSuffix;

};
