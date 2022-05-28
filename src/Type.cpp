
#include "Type.h"

const TypeName & Type::name() const {
    return typeName;
}

std::string Type::parserOutputArgDeclaration() const {
    return typeName.refArgDeclaration("value");
}

std::string Type::serializerInputArgDeclaration() const {
    return typeName.constRefArgDeclaration("value");
}
