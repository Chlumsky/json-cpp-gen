
#pragma once

#include <memory>
#include "Type.h"

class Namespace;

struct Symbol {

    std::unique_ptr<Type> type;
    std::unique_ptr<Namespace> ns;

};

typedef std::shared_ptr<Symbol> SymbolPtr;

inline const Type *symbolType(const SymbolPtr &symbol) {
    return symbol ? symbol->type.get() : nullptr;
}
