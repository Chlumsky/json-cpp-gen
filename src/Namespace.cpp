
#include "Namespace.h"

Namespace::Namespace(Namespace *parentNamespace) : parent(parentNamespace) { }

void Namespace::inheritFrom(Namespace *baseNamespace) {
    if (baseNamespace && baseNamespace != this && !baseNamespace->inheritsRecursively(this)) {
        for (Namespace *preexistingBase : inheritedNamespaces) {
            if (preexistingBase == baseNamespace)
                return;
        }
        inheritedNamespaces.push_back(baseNamespace);
    }
}

Namespace *Namespace::parentNamespace() {
    return parent;
}

bool Namespace::makeLocalAlias(const UnqualifiedName &name, const SymbolPtr &originalSymbol) {
    if (originalSymbol) {
        SymbolPtr &symbol = symbols[name];
        if (symbol)
            return symbol == originalSymbol;
        symbol = originalSymbol;
    }
    return true;
}

SymbolPtr Namespace::findLocalSymbol(const UnqualifiedName &name) const {
    std::map<std::string, SymbolPtr>::const_iterator it = symbols.find(name);
    if (it != symbols.end())
        return it->second;
    return nullptr;
}

SymbolPtr Namespace::requireLocalSymbol(const UnqualifiedName &name) {
    SymbolPtr &symbol = symbols[name];
    if (!symbol)
        symbol = SymbolPtr(new Symbol);
    return symbol;
}

SymbolPtr Namespace::establishSymbol(QualifiedName::Ref name, bool establishNamespace) {
    if (!name)
        return nullptr;
    if (name.isAbsolute()) {
        if (parent)
            return parent->establishSymbol(name, establishNamespace);
        name = name.exceptAbsolute();
    }
    SymbolPtr &symbol = symbols[name.prefix()];
    name = name.exceptPrefix();
    if (!symbol)
        symbol = SymbolPtr(new Symbol);
    if (!name && !establishNamespace)
        return symbol;
    if (!symbol->ns)
        symbol->ns = std::unique_ptr<Namespace>(new Namespace(this));
    if (!name)
        return symbol;
    return symbol->ns->establishSymbol(name, establishNamespace);
}

SymbolPtr Namespace::establishNamespace(QualifiedName::Ref name) {
    return establishSymbol(name, true);
}

SymbolPtr Namespace::findSymbol(QualifiedName::Ref name, bool parentFallback) const {
    if (!name)
        return nullptr;
    if (name.isAbsolute()) {
        if (parent)
            return parent->findSymbol(name, parentFallback);
        name = name.exceptAbsolute();
        parentFallback = false;
    }
    std::map<std::string, SymbolPtr>::const_iterator it = symbols.find(name.prefix());
    if (it != symbols.end() && it->second) {
        QualifiedName::Ref innerName = name.exceptPrefix();
        if (!innerName)
            return it->second;
        else if (it->second->ns)
            return it->second->ns->findSymbol(innerName, false);
    } else {
        for (Namespace *baseNamespace : inheritedNamespaces) {
            if (SymbolPtr symbol = baseNamespace->findSymbol(name, false))
                return symbol;
        }
        if (parent && parentFallback)
            return parent->findSymbol(name, true);
    }
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

bool Namespace::inheritsRecursively(const Namespace *ns) const {
    for (Namespace *baseNs : inheritedNamespaces) {
        if (baseNs == ns || baseNs->inheritsRecursively(ns))
            return true;
    }
    return false;
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
