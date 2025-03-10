
#pragma once

#include <string>
#include <map>
#include <memory>
#include "Type.h"
#include "Symbol.h"

class Namespace {

public:
    static void splitPrefix(std::string &inWholeOutPrefix, std::string &outSuffix);

    explicit Namespace(Namespace *parentNamespace);
    Namespace *parentNamespace();
    bool makeLocalAlias(const std::string &unqualifiedName, const SymbolPtr &originalSymbol);
    SymbolPtr findLocalSymbol(const std::string &unqualifiedName) const;
    SymbolPtr requireLocalSymbol(const std::string &unqualifiedName);
    SymbolPtr establishSymbol(const std::string &qualifiedName, bool establishNamespace = false);
    SymbolPtr establishNamespace(const std::string &qualifiedName);
    SymbolPtr findSymbol(const std::string &qualifiedName, bool parentFallback) const;
    int compileTypes(TemplateInstanceCache *templateInstanceCache, int stopMask = 0, const Type **stoppingType = nullptr);

    std::string dump(int indent = 0) const;

private:
    Namespace *parent;
    std::map<std::string, SymbolPtr> symbols;

};
