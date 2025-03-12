
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//
//  JSON-CPP-GEN v0.0.1 by Viktor Chlumsky (c) 2021 - 2022
//  A generator of JSON parser & serializer C++ code from structure header files
//
//  Usage: json-cpp-gen configuration.json
//
//  https://github.com/Chlumsky/json-cpp-gen
//  See GitHub page for configuration format and licensing info
//

#include <cstdio>
#include <string>
#include "AbsPath.h"
#include "TypeSet.h"
#include "types/StringType.h"
#include "types/ConstStringType.h"
#include "types/TypeAlias.h"
#include "container-templates/OptionalContainerTemplate.h"
#include "container-templates/ArrayContainerTemplate.h"
#include "container-templates/FixedArrayContainerTemplate.h"
#include "container-templates/StaticArrayContainerTemplate.h"
#include "container-templates/ObjectContainerTemplate.h"
#include "container-templates/ObjectMapContainerTemplate.h"
#include "HeaderParser.h"
#include "text-file-io.h"
#include "ParserGenerator.h"
#include "SerializerGenerator.h"
#include "../generated/ConfigurationParser.h"

#define PROGRAM_NAME "json-cpp-gen"
#ifdef _WIN32
    #define PROGRAM_FILE_NAME PROGRAM_NAME ".exe"
#else
    #define PROGRAM_FILE_NAME PROGRAM_NAME
#endif

static bool containsString(const std::vector<std::string> &list, const std::string &needle) {
    for (const std::string &str : list) {
        if (str == needle)
            return true;
    }
    return false;
}

static const StringType *findStringType(const TypeSet &typeSet, const std::string &name) {
    if (const Type *type = typeSet.find(name))
        return type->stringType();
    return nullptr;
}

static const ArrayContainerTemplate *findArrayContainerTemplate(const TypeSet &typeSet, const std::string &name) {
    if (const ContainerTemplate<> *containerTemplate = typeSet.findContainerTemplate<>(name))
        return containerTemplate->arrayContainerTemplate();
    return nullptr;
}

static int lineAtPosition(const std::string &str, int position) {
    int line = 1;
    for (char c : str) {
        if (!position--)
            break;
        if (c == '\n')
            ++line;
    }
    return line;
}

static std::string visualizeErrorPosition(const std::string &str, int position, const std::string &indent) {
    int strLen = int(str.size());
    if (position >= strLen)
        return std::string();
    std::string output;
    int caretPos = 0;
    if (position <= 32) {
        if (strLen > 64)
            output = indent+str.substr(0, 64)+"...";
        else
            output = indent+str.substr(0, strLen);
        caretPos = position;
    } else {
        if (strLen-(position-32) > 64)
            output = indent+"..."+str.substr(position-32, 64)+"...";
        else
            output = indent+"..."+str.substr(position-32, strLen-(position-32));
        caretPos = 35;
    }
    for (char &c : output) {
        if (c == '\r' || c == '\n')
            c = ' ';
    }
    output.push_back('\n');
    output += indent;
    for (int i = 0; i < caretPos; ++i)
        output.push_back(' ');
    output.push_back('^');
    output.push_back('\n');
    return output;
}

int main(int argc, const char *const *argv) {

    if (argc < 2) {
        fputs(
            "Usage: " PROGRAM_FILE_NAME " configuration.json\n"
            "See the documentation for information about the structure of the configuration file.\n",
            stderr
        );
        return 0;
    }

    std::string configString;
    AbsPath basePath(argv[1]);
    if (!readTextFile(configString, argv[1])) {
        fprintf(stderr, "Error: Could not read configuration file '%s'\n", argv[1]);
        return -1;
    }

    Configuration config;
    if (ConfigurationParser::Error error = ConfigurationParser::parse(config, configString.c_str())) {
        fprintf(stderr,
            "Error: Malformed configuration file '%s'\n"
            "       %s at position %d (line %d)\n"
            "%s",
            argv[1], error.typeString(), error.position, lineAtPosition(configString, error.position),
            visualizeErrorPosition(configString, error.position, "       ").c_str()
        );
        return -1;
    }

    if (config.parsers.empty() && config.serializers.empty()) {
        fputs("No output requested\n", stderr);
        return 0;
    }

    TypeSet typeSet;
    { // CUSTOM TYPE DEFINITIONS
        for (const Configuration::StringDef &stringDef : config.stringTypes) {
            if (QualifiedName::validate(stringDef.name))
                typeSet.root().establishSymbol(QualifiedName(stringDef.name), false)->type = std::unique_ptr<Type>(new StringType(Generator::safeName(stringDef.name), stringDef.api));
            else
                fprintf(stderr, "Error: String type '%s' is not a simple type name\n", stringDef.name.c_str());
        }
        for (const Configuration::ConstStringDef &fixedStringDef : config.constStringTypes) {
            if (const StringType *dynamicStringType = findStringType(typeSet, fixedStringDef.stringType)) {
                if (QualifiedName::validate(fixedStringDef.name))
                    typeSet.root().establishSymbol(QualifiedName(fixedStringDef.name), false)->type = std::unique_ptr<Type>(new ConstStringType(Generator::safeName(fixedStringDef.name), dynamicStringType, fixedStringDef.api));
                else
                    fprintf(stderr, "Error: Fixed string type '%s' is not a simple type name\n", fixedStringDef.name.c_str());
            }
            else {
                fprintf(stderr, "Error: String type '%s' not found, skipping fixed string type '%s'\n", fixedStringDef.stringType.c_str(), fixedStringDef.name.c_str());
                continue;
            }
        }
        for (const Configuration::ArrayContainerDef &arrayContainerDef : config.arrayContainerTypes)
            typeSet.addContainerTemplate(std::unique_ptr<ContainerTemplate<> >(new ArrayContainerTemplate(Generator::safeName(arrayContainerDef.name), arrayContainerDef.api)));
        for (const Configuration::FixedArrayContainerDef &fixedArrayContainerDef : config.fixedArrayContainerTypes) {
            if (const ArrayContainerTemplate *arrayContainerTemplate = findArrayContainerTemplate(typeSet, fixedArrayContainerDef.arrayContainerType))
                typeSet.addContainerTemplate(std::unique_ptr<ContainerTemplate<> >(new FixedArrayContainerTemplate(Generator::safeName(fixedArrayContainerDef.name), arrayContainerTemplate, fixedArrayContainerDef.api)));
            else {
                fprintf(stderr, "Error: Array container type '%s' not found, skipping fixed array container type '%s'\n", fixedArrayContainerDef.arrayContainerType.c_str(), fixedArrayContainerDef.name.c_str());
                continue;
            }
        }
        for (const Configuration::StaticArrayContainerDef &staticArrayContainerDef : config.staticArrayContainerTypes)
            typeSet.addContainerTemplate(std::unique_ptr<ContainerTemplate<int> >(new StaticArrayContainerTemplate(Generator::safeName(staticArrayContainerDef.name), staticArrayContainerDef.api)));
        for (const Configuration::ObjectContainerDef &objectContainerDef : config.objectContainerTypes) {
            if (const Type *keyType = typeSet.find(objectContainerDef.keyType))
                typeSet.addContainerTemplate(std::unique_ptr<ContainerTemplate<> >(new ObjectContainerTemplate(Generator::safeName(objectContainerDef.name), keyType, objectContainerDef.api)));
            else {
                fprintf(stderr, "Error: Key type '%s' not found, skipping object container type '%s'\n", objectContainerDef.keyType.c_str(), objectContainerDef.name.c_str());
                continue;
            }
        }
        for (const Configuration::ObjectMapContainerDef &objectMapContainerDef : config.objectMapContainerTypes)
            typeSet.addContainerTemplate(std::unique_ptr<ContainerTemplate<const Type *> >(new ObjectMapContainerTemplate(Generator::safeName(objectMapContainerDef.name), objectMapContainerDef.api)));
        for (const Configuration::OptionalContainerDef &optionalContainerDef : config.optionalContainerTypes)
            typeSet.addContainerTemplate(std::unique_ptr<ContainerTemplate<> >(new OptionalContainerTemplate(Generator::safeName(optionalContainerDef.name), optionalContainerDef.api)));
    }

    // TYPE ALIASES
    for (const std::map<std::string, std::string>::value_type &typeAlias : config.typeAliases) {
        if (!typeSet.addAlias(typeAlias.first, typeAlias.second)) {
            fprintf(stderr, "Error: Cyclic alias '%s', aborting\n", typeAlias.first.c_str());
            return -1;
        }
    }

    // INPUT SOURCE FILES
    int totalParserPasses = 0;
    for (HeaderParser::Pass parserPass; parserPass; ++parserPass, ++totalParserPasses) {
        for (const std::string &input : config.inputs) {
            std::string inputString;
            if (!readTextFile(inputString, (basePath+input).cStr())) {
                fprintf(stderr, "Error: Failed to open input file '%s', aborting\n", input.c_str());
                return -1;
            }
            if (HeaderParser::Error parseError = parseHeader(parserPass, typeSet, inputString)) {
                fprintf(stderr,
                    "Error: Failed to parse input file '%s', aborting\n"
                    "       %s at position %d (line %d)\n"
                    "%s",
                    input.c_str(), parseError.typeString(), parseError.position, lineAtPosition(inputString, parseError.position),
                    visualizeErrorPosition(inputString, parseError.position, "       ").c_str()
                );
            #if !defined(NDEBUG) && defined(DUMP_TYPESET)
                fprintf(stderr, "\nROOT NAMESPACE DUMP:\n%s\n", typeSet.root().dump().c_str());
            #endif
                return -1;
            }
        }
    }
#ifndef NDEBUG
    fprintf(stderr, "Header parser passes: %d\n", totalParserPasses);
#endif

    // TYPE FINALIZATION
    if (const Type *badType = typeSet.compile()) {
        fprintf(stderr, "Error: Cyclic definition of type '%s', aborting\n", badType->name().body().c_str());
        return -1;
    }

    // STRING TYPE DEFINITION
    const StringType *stringType = findStringType(typeSet, config.stringType);
    if (!stringType) {
        fprintf(stderr, "Error: String type '%s' not found, aborting", config.stringType.c_str());
        return -1;
    }
    if (stringType->name().body() == "std::string" && stringType->name().suffix().empty() && !containsString(config.includes, "<string>")) {
        fputs("Note: Adding <string> to include files\n", stderr);
        config.includes.push_back("<string>");
    }

    // OUTPUT PARSERS
    for (const Configuration::GeneratorDef &parserDef : config.parsers) {
        ParserGenerator parserGen(parserDef.name, stringType, config.settings);
        AbsPath headerPath = basePath+parserDef.headerOutput;
        AbsPath sourcePath = basePath+parserDef.sourceOutput;
        if (parserDef.replacementIncludes.empty()) {
            for (const std::string &include : config.includes)
                parserGen.addTypeInclude(*include.c_str() == '<' || *include.c_str() == '"' ? include : "\""+(basePath+include-headerPath)+"\"");
            for (const std::string &input : config.inputs)
                parserGen.addTypeInclude("\""+(basePath+input-headerPath)+"\"");
        } else {
            for (const std::string &include : parserDef.replacementIncludes)
                parserGen.addTypeInclude(*include.c_str() == '<' || *include.c_str() == '"' ? include : "\""+(basePath+include-headerPath)+"\"");
        }
        // TODO baseClass
        for (const std::string &typeName : parserDef.types) {
            if (const Type *type = parseType(typeSet, typeName))
                parserGen.generateParserFunction(type);
            else
                fprintf(stderr, "Error: Type '%s' not found, will not be parsed by '%s'\n", typeName.c_str(), parserDef.name.c_str());
        }
        std::string header = parserGen.generateHeader();
        std::string source = parserGen.generateSource(headerPath-sourcePath);
        if (!parserDef.headerOutput.empty() && !writeTextFile(header, headerPath.cStr())) {
            fprintf(stderr, "Error: Failed to write output header file '%s'\n", parserDef.headerOutput.c_str());
            return -1;
        }
        if (!parserDef.sourceOutput.empty() && !writeTextFile(source, sourcePath.cStr())) {
            fprintf(stderr, "Error: Failed to write output source file '%s'\n", parserDef.sourceOutput.c_str());
            return -1;
        }
    }

    // OUTPUT SERIALIZERS
    for (const Configuration::GeneratorDef &serializerDef : config.serializers) {
        SerializerGenerator serializerGen(serializerDef.name, stringType, config.settings);
        AbsPath headerPath = basePath+serializerDef.headerOutput;
        AbsPath sourcePath = basePath+serializerDef.sourceOutput;
        if (serializerDef.replacementIncludes.empty()) {
            for (const std::string &include : config.includes)
                serializerGen.addTypeInclude(*include.c_str() == '<' || *include.c_str() == '"' ? include : "\""+(basePath+include-headerPath)+"\"");
            for (const std::string &input : config.inputs)
                serializerGen.addTypeInclude("\""+(basePath+input-headerPath)+"\"");
        } else {
            for (const std::string &include : serializerDef.replacementIncludes)
                serializerGen.addTypeInclude(*include.c_str() == '<' || *include.c_str() == '"' ? include : "\""+(basePath+include-headerPath)+"\"");
        }
        // TODO baseClass
        for (const std::string &typeName : serializerDef.types) {
            if (const Type *type = parseType(typeSet, typeName))
                serializerGen.generateSerializerFunction(type);
            else
                fprintf(stderr, "Error: Type '%s' not found, will not be serialized by '%s'\n", typeName.c_str(), serializerDef.name.c_str());
        }
        std::string header = serializerGen.generateHeader();
        std::string source = serializerGen.generateSource(headerPath-sourcePath);
        if (!serializerDef.headerOutput.empty() && !writeTextFile(header, headerPath.cStr())) {
            fprintf(stderr, "Error: Failed to write output header file '%s'\n", serializerDef.headerOutput.c_str());
            return -1;
        }
        if (!serializerDef.sourceOutput.empty() && !writeTextFile(source, sourcePath.cStr())) {
            fprintf(stderr, "Error: Failed to write output source file '%s'\n", serializerDef.sourceOutput.c_str());
            return -1;
        }
    }

    return 0;
}
