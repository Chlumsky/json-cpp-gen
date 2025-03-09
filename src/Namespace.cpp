
#include "Namespace.h"

// input in a
static void splitQualifiedName(std::string &a, std::string &b) {
    size_t sepPos = a.find(':');
    if (sepPos != std::string::npos) {
        size_t sepEnd = sepPos;
        while (sepEnd < a.size() && (a[sepEnd] == ':' || a[sepEnd] == ' ' || a[sepEnd] == '\t' || a[sepEnd] == '\n' || a[sepEnd] == '\r'))
            ++sepEnd;
        b = a.substr(sepEnd);
        a.resize(sepPos);
    }
    while (!a.empty() && (a.back() == ' ' || a.back() == '\t' || a.back() == '\n' || a.back() == '\r'))
        a.pop_back();
}

Namespace::Namespace(Namespace *parentNamespace) : parent(parentNamespace) { }

Namespace *Namespace::parentNamespace() {
    return parent;
}

bool Namespace::makeSymbolAlias(const std::string &unqualifiedName, const SymbolPtr &originalSymbol) {
    SymbolPtr &symbol = symbols[unqualifiedName];
    if (symbol)
        return symbol == originalSymbol;
    symbol = originalSymbol;
    return true;
}

SymbolPtr Namespace::establishSymbol(const std::string &qualifiedName, bool establishNamespace) {
    if (qualifiedName.empty())
        return nullptr;
    if (qualifiedName[0] == ':') {
        if (parent)
            return parent->establishSymbol(qualifiedName, establishNamespace);
        std::string nameCopy(qualifiedName);
        std::string nonRootName;
        splitQualifiedName(nameCopy, nonRootName);
        return establishSymbol(nonRootName, establishNamespace);
    }
    std::string outerName(qualifiedName);
    std::string innerName;
    splitQualifiedName(outerName, innerName);
    SymbolPtr &symbol = symbols[outerName];
    if (!symbol)
        symbol = SymbolPtr(new Symbol);
    if (innerName.empty() && !establishNamespace)
        return symbol;
    if (!symbol->ns)
        symbol->ns = std::unique_ptr<Namespace>(new Namespace(this));
    if (innerName.empty())
        return symbol;
    return symbol->ns->establishSymbol(innerName, establishNamespace);
}

SymbolPtr Namespace::findSymbol(const std::string &qualifiedName, bool parentFallback) const {
    if (qualifiedName.empty())
        return nullptr;
    if (qualifiedName[0] == ':') {
        if (parent)
            return parent->findSymbol(qualifiedName, parentFallback);
        std::string nameCopy(qualifiedName);
        std::string nonRootName;
        splitQualifiedName(nameCopy, nonRootName);
        return findSymbol(nonRootName, false);
    }
    std::string outerName(qualifiedName);
    std::string innerName;
    splitQualifiedName(outerName, innerName);
    std::map<std::string, SymbolPtr>::const_iterator it = symbols.find(outerName);
    if (it != symbols.end() && it->second) {
        if (innerName.empty())
            return it->second;
        else if (it->second->ns)
            return it->second->ns->findSymbol(innerName, false);
    } else if (parent && parentFallback)
        return parent->findSymbol(qualifiedName, true);
    return nullptr;
}

int Namespace::compileTypes(TemplateInstanceCache *templateInstanceCache, int stopMask, const Type **stoppingType) {
    int result = 0;
    for (const std::map<std::string, SymbolPtr>::value_type &symbol : symbols) {
        if (symbol.second) {
            if (symbol.second->type && ((result |= symbol.second->type->compile(templateInstanceCache))&stopMask)) {
                if (stoppingType)
                    *stoppingType = symbol.second->type.get();
                break;
            }
            if (symbol.second->ns && ((result |= symbol.second->ns->compileTypes(templateInstanceCache, stopMask, stoppingType))&stopMask))
                break;
        }
    }
    return result;
}
