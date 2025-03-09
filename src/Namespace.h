
#pragma once

#include <string>
#include <map>
#include <memory>
#include "Type.h"
#include "Symbol.h"

class Namespace {

public:
    explicit Namespace(Namespace *parentNamespace);
    Namespace *parentNamespace();
    bool makeSymbolAlias(const std::string &unqualifiedName, const SymbolPtr &originalSymbol);
    SymbolPtr establishSymbol(const std::string &qualifiedName, bool establishNamespace);
    SymbolPtr findSymbol(const std::string &qualifiedName, bool parentFallback) const;
    int compileTypes(TemplateInstanceCache *templateInstanceCache, int stopMask = 0, const Type **stoppingType = nullptr);

private:
    Namespace *parent;
    std::map<std::string, SymbolPtr> symbols;

};
