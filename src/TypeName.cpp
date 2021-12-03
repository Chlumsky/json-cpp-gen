
#include "TypeName.h"

std::string TypeName::fullName() const {
    return nameBody+nameSuffix;
}

std::string TypeName::variableDeclaration(const std::string &variableName) const {
    return nameBody+' '+variableName+nameSuffix;
}

std::string TypeName::refArgDeclaration(const std::string &argName) const {
    return nameBody+(nameSuffix.empty() ? " &" : " ")+argName+nameSuffix;
}

std::string TypeName::constRefArgDeclaration(const std::string &argName) const {
    return nameBody+(nameSuffix.empty() ? " const &" : " const ")+argName+nameSuffix;
}
