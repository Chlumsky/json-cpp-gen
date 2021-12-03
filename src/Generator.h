
#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include "Type.h"
#include "types/StringType.h"
#include "TypeSet.h"
#include "Configuration.h"

#define INDENT "    "

// A code generator - prototype of ParserGenerator & SerializerGenerator.
class Generator {

public:
    Generator(const std::string &className, const StringType *stringType, const Settings &settings);
    void addTypeInclude(const std::string &includeAddress);
    std::string getJsonMemberNameLiteral(const std::string &memberName) const;
    std::string getJsonEnumValueLiteral(const std::string &enumValue) const;
    inline const StringType * stringType() const { return mStringType; }
    inline const Settings & settings() const { return mSettings; }

    std::string generateErrorStatement(const char *errorName) const; // throw / return depending on config

protected:
    struct Function {
        const Type *type;
        std::string name;
        std::string body;
        bool rootStructure;
    };

    static const char * const signature;

    std::vector<std::string> classNamespaces;
    std::string className;

    std::vector<std::string> typeIncludes;

    std::vector<Function> functions;
    std::map<std::string, std::string> functionNames;
    std::set<std::string> usedFunctionNames;
    std::string generateFunctionName(const char *prefix, const Type *type);
    std::string beginNamespace() const;
    std::string endNamespace() const;

private:
    const StringType *mStringType;
    Settings mSettings;

};
