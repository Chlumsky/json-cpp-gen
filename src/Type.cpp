
#include "Type.h"

std::string Type::parserOutputArgDeclaration() const {
    return typeName.refArgDeclaration("value");
}

std::string Type::serializerInputArgDeclaration() const {
    return typeName.constRefArgDeclaration("value");
}
