
#include "TypeName.h"

static bool isRefOrPtr(const std::string &nameBody) {
    return !nameBody.empty() && (nameBody.back() == '&' || nameBody.back() == '*');
}

std::string TypeName::fullName() const {
    return nameBody+nameSuffix;
}

std::string TypeName::variableDeclaration(const std::string &variableName) const {
    return nameBody+(isRefOrPtr(nameBody) ? "" : " ")+variableName+nameSuffix;
}

std::string TypeName::refArgDeclaration(const std::string &argName) const {
    return nameBody+(isRefOrPtr(nameBody) ? "" : " ")+(nameSuffix.empty() ? "&" : "")+argName+nameSuffix;
}

std::string TypeName::constRefArgDeclaration(const std::string &argName) const {
    if (isRefOrPtr(nameBody))
        return nameBody+(nameSuffix.empty() ? "const &" : "const ")+argName+nameSuffix;
    return "const "+nameBody+(nameSuffix.empty() ? " &" : " ")+argName+nameSuffix;
}
