
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
    static const unsigned FEATURE_CSTDLIB;

    Generator(const std::string &className, const StringType *stringType, const Settings &settings);
    void addTypeInclude(const std::string &includeAddress);
    void addFeature(unsigned featureBit);
    std::string getJsonMemberNameLiteral(const std::string &memberName) const;
    std::string getJsonEnumValueLiteral(const std::string &enumValue) const;
    void resolveVirtualTypename(const Type *type, const Type *parentType, const std::string &memberName);
    inline const StringType *stringType() const { return mStringType; }
    inline const Settings &settings() const { return mSettings; }

    static std::string safeName(const std::string &name);
    static std::string charLiteral(char c);

protected:
    struct Function {
        const Type *type;
        std::string name;
        std::string body;
    };

    static const char *const signature;

    std::vector<std::string> classNamespaces;
    std::string className;

    std::vector<std::string> typeIncludes;

    std::vector<Function> functions;
    std::map<std::string, std::string> functionNames;
    std::set<std::string> usedFunctionNames;
    std::vector<std::pair<std::string, std::string> > virtualTypedefs;
    std::set<std::string> resolvedVirtualTypenames;
    std::vector<const Type *> entryTypes;
    unsigned featureBits = 0;

    std::string generateVirtualTypedefs(const std::string &indent);
    std::string generateFunctionName(const char *prefix, const Type *type);
    std::string beginNamespace() const;
    std::string endNamespace() const;

private:
    const StringType *mStringType;
    Settings mSettings;

};
