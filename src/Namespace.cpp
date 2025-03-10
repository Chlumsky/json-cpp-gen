
#include "Namespace.h"

static bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

void Namespace::splitPrefix(std::string &inWholeOutPrefix, std::string &outSuffix) {
    size_t sepStart = inWholeOutPrefix.find("::");
    if (sepStart != std::string::npos) {
        size_t sepEnd = sepStart+2;
        while (sepEnd < inWholeOutPrefix.size() && isWhitespace(inWholeOutPrefix[sepEnd]))
            ++sepEnd;
        while (sepStart > 0 && isWhitespace(inWholeOutPrefix[sepStart-1]))
            --sepStart;
        outSuffix = inWholeOutPrefix.substr(sepEnd);
        inWholeOutPrefix.resize(sepStart);
    } else
        outSuffix = std::string();
}

Namespace::Namespace(Namespace *parentNamespace) : parent(parentNamespace) { }

Namespace *Namespace::parentNamespace() {
    return parent;
}

bool Namespace::makeLocalAlias(const std::string &unqualifiedName, const SymbolPtr &originalSymbol) {
    if (originalSymbol) {
        SymbolPtr &symbol = symbols[unqualifiedName];
        if (symbol)
            return symbol == originalSymbol;
        symbol = originalSymbol;
    }
    return true;
}

SymbolPtr Namespace::findLocalSymbol(const std::string &unqualifiedName) const {
    std::map<std::string, SymbolPtr>::const_iterator it = symbols.find(unqualifiedName);
    if (it != symbols.end())
        return it->second;
    return nullptr;
}

SymbolPtr Namespace::requireLocalSymbol(const std::string &unqualifiedName) {
    SymbolPtr &symbol = symbols[unqualifiedName];
    if (!symbol)
        symbol = SymbolPtr(new Symbol);
    return symbol;
}

SymbolPtr Namespace::establishSymbol(const std::string &qualifiedName, bool establishNamespace) {
    if (qualifiedName.empty())
        return nullptr;
    if (qualifiedName[0] == ':') {
        if (parent)
            return parent->establishSymbol(qualifiedName, establishNamespace);
        std::string nameCopy(qualifiedName);
        std::string nonRootName;
        splitPrefix(nameCopy, nonRootName);
        return establishSymbol(nonRootName, establishNamespace);
    }
    std::string outerName(qualifiedName);
    std::string innerName;
    splitPrefix(outerName, innerName);
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

SymbolPtr Namespace::establishNamespace(const std::string &qualifiedName) {
    return establishSymbol(qualifiedName, true);
}

SymbolPtr Namespace::findSymbol(const std::string &qualifiedName, bool parentFallback) const {
    if (qualifiedName.empty())
        return nullptr;
    if (qualifiedName[0] == ':') {
        if (parent)
            return parent->findSymbol(qualifiedName, parentFallback);
        std::string nameCopy(qualifiedName);
        std::string nonRootName;
        splitPrefix(nameCopy, nonRootName);
        return findSymbol(nonRootName, false);
    }
    std::string outerName(qualifiedName);
    std::string innerName;
    splitPrefix(outerName, innerName);
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

std::string Namespace::dump(int indent) const {
    std::string indentation;
    for (int i = 0; i < indent; ++i)
        indentation += "    ";
    std::string result;
    for (const std::map<std::string, SymbolPtr>::value_type &symbol : symbols) {
        if (symbol.second) {
            result += "\n"+indentation+symbol.first;
            if (symbol.second->type)
                result += ": "+symbol.second->type->name().fullName();
            if (symbol.second->ns) {
                std::string subDump = symbol.second->ns->dump(indent+1);
                if (subDump.empty())
                    result += " { }";
                else
                    result += " {"+subDump+"\n"+indentation+"}";
            }
        }
    }
    return result;
}
