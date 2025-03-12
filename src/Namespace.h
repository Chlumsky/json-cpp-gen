
#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Type.h"
#include "QualifiedName.h"
#include "Symbol.h"

class Namespace {

public:
    explicit Namespace(Namespace *parentNamespace);
    void inheritFrom(Namespace *baseNamespace);
    Namespace *parentNamespace();
    bool makeLocalAlias(const UnqualifiedName &name, const SymbolPtr &originalSymbol);
    SymbolPtr findLocalSymbol(const UnqualifiedName &name) const;
    SymbolPtr requireLocalSymbol(const UnqualifiedName &name);
    SymbolPtr establishSymbol(QualifiedName::Ref name, bool establishNamespace = false);
    SymbolPtr establishNamespace(QualifiedName::Ref name);
    SymbolPtr findSymbol(QualifiedName::Ref name, bool parentFallback) const;
    int compileTypes(TemplateInstanceCache *templateInstanceCache, int stopMask = 0, const Type **stoppingType = nullptr);

    std::string dump(int indent = 0) const;

private:
    Namespace *parent;
    std::vector<Namespace *> inheritedNamespaces;
    std::map<std::string, SymbolPtr> symbols;

    bool inheritsRecursively(const Namespace *ns) const;

};
