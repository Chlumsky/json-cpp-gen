
#include "Namespace.h"

Namespace::Namespace(Root) : parent(nullptr) { }

Namespace::Namespace(const UnqualifiedName &name, Namespace *parentNamespace) : name(name), parent(parentNamespace) { }

bool Namespace::inheritsRecursively(const Namespace *ns) const {
    for (Namespace *baseNs : inheritedNamespaces) {
        if (baseNs == ns || baseNs->inheritsRecursively(ns))
            return true;
    }
    return false;
}

void Namespace::inheritFrom(Namespace *baseNamespace) {
    if (baseNamespace && baseNamespace != this && !baseNamespace->inheritsRecursively(this)) {
        for (Namespace *preexistingBase : inheritedNamespaces) {
            if (preexistingBase == baseNamespace)
                return;
        }
        inheritedNamespaces.push_back(baseNamespace);
    }
}

void Namespace::usingNamespace(QualifiedName &&usingNamespaceName) {
    if (usingNamespaceName) {
        for (const QualifiedName &preexistingUsingNamespace : usingNamespaceNames) {
            if (preexistingUsingNamespace == usingNamespaceName)
                return;
        }
        usingNamespaceNames.push_back((QualifiedName &&) usingNamespaceName);
    }
}

const std::vector<QualifiedName> &Namespace::usingNamespaces() const {
    return usingNamespaceNames;
}

QualifiedName Namespace::fullName() const {
    QualifiedName qualifiedName;
    if (parent) {
        qualifiedName = parent->fullName();
        qualifiedName.append(name);
    }
    return qualifiedName;
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
    const UnqualifiedName &subname = name.prefix();
    name = name.exceptPrefix();
    SymbolPtr &symbol = symbols[subname];
    if (!symbol)
        symbol = SymbolPtr(new Symbol);
    if (!name && !establishNamespace)
        return symbol;
    if (!symbol->ns)
        symbol->ns = std::unique_ptr<Namespace>(new Namespace(subname, this));
    if (!name)
        return symbol;
    return symbol->ns->establishSymbol(name, establishNamespace);
}

SymbolPtr Namespace::establishNamespace(QualifiedName::Ref name) {
    return establishSymbol(name, true);
}

#define FALLBACK_FLAG_INHERITED 0x01
#define FALLBACK_FLAG_PARENT 0x02
#define FALLBACK_FLAG_USING 0x04

SymbolPtr Namespace::findSymbol(QualifiedName::Ref name, bool localOnly) const {
    return findSymbol(name, localOnly ? FALLBACK_FLAG_INHERITED : FALLBACK_FLAG_INHERITED|FALLBACK_FLAG_PARENT|FALLBACK_FLAG_USING);
}

SymbolPtr Namespace::findSymbol(QualifiedName::Ref name, int fallbackFlags) const {
    if (!name)
        return nullptr;
    if (name.isAbsolute()) {
        if (parent)
            return parent->findSymbol(name, fallbackFlags);
        name = name.exceptAbsolute();
        fallbackFlags &= FALLBACK_FLAG_INHERITED;
    }
    std::map<std::string, SymbolPtr>::const_iterator it = symbols.find(name.prefix());
    if (it != symbols.end() && it->second) {
        QualifiedName::Ref innerName = name.exceptPrefix();
        if (!innerName)
            return it->second;
        else if (it->second->ns)
            return it->second->ns->findSymbol(innerName, true);
    } else {
        if (fallbackFlags&FALLBACK_FLAG_INHERITED) {
            for (Namespace *baseNamespace : inheritedNamespaces) {
                if (SymbolPtr symbol = baseNamespace->findSymbol(name, FALLBACK_FLAG_INHERITED))
                    return symbol;
            }
        }
        if ((fallbackFlags&FALLBACK_FLAG_PARENT) && parent) {
            if (SymbolPtr symbol = parent->findSymbol(name, fallbackFlags|FALLBACK_FLAG_USING))
                return symbol;
        }
        if (fallbackFlags&FALLBACK_FLAG_USING) {
            for (const QualifiedName &usingNamespace : usingNamespaceNames) {
                if (SymbolPtr symbol = findSymbol(usingNamespace+name, fallbackFlags&FALLBACK_FLAG_PARENT))
                    return symbol;
            }
        }
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
