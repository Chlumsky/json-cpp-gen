
#include "StructureType.h"

#include "OptionalContainerType.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

StructureType::StructureType(const std::string &name) : DirectType(TypeName(name)) { }

StructureType::StructureType(std::string &&name) : DirectType(TypeName((std::string &&) name)) { }

void StructureType::inheritFrom(const StructureType *baseType) {
    members.insert(baseType->members.begin(), baseType->members.end());
    orderedMembers.insert(orderedMembers.end(), baseType->orderedMembers.begin(), baseType->orderedMembers.end());
}

std::string StructureType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    // TODO make key a class member to reduce the number of allocations
    body += indent+generator->stringType()->name().variableDeclaration("key")+";\n";
    if (generator->settings().noThrow) {
        body += indent+"if (!matchSymbol('{'))\n";
        body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    } else
        body += indent+"requireSymbol('{');\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+"int separatorCheck = -1;\n";
    body += indent+"while (!matchSymbol('}')) {\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+INDENT "if (!separatorCheck)\n";
        body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    body += generator->generateValueParse(generator->stringType(), "key", indent+INDENT);
    if (generator->settings().noThrow) {
        body += indent+INDENT "if (!matchSymbol(':'))\n";
        body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    } else
        body += indent+INDENT "requireSymbol(':');\n";
    bool first = true;
    if (generator->settings().noThrow) {
        for (const std::pair<std::string, const Type *> &member : orderedMembers) {
            body += indent+INDENT+(first ? "if" : "} else if")+" (key == "+generator->getJsonMemberNameLiteral(member.first)+") {\n";
            body += indent+INDENT INDENT+"if (Error error = "+generator->generateParserFunctionCall(member.second, "value."+member.first, false)+")\n";
            body += indent+INDENT INDENT INDENT "return error;\n";
            first = false;
        }
        body += indent+INDENT "} else {\n";
    } else {
        for (const std::pair<std::string, const Type *> &member : orderedMembers) {
            body += indent+INDENT+(first ? "if" : "else if")+" (key == "+generator->getJsonMemberNameLiteral(member.first)+")\n";
            body += indent+INDENT INDENT+generator->generateParserFunctionCall(member.second, "value."+member.first, false)+";\n";
            first = false;
        }
        body += indent+INDENT "else\n";
    }
    if (generator->settings().ignoreExtraKeys) {
        if (generator->settings().noThrow) {
            body += indent+INDENT INDENT "if (Error error = skipValue())\n";
            body += indent+INDENT INDENT INDENT "return error;\n";
        } else
            body += indent+INDENT INDENT "skipValue();\n";
    } else
        body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::UNKNOWN_KEY)+";\n";
    if (generator->settings().noThrow)
        body += indent+INDENT "}\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+INDENT "separatorCheck = matchSymbol(',');\n";
    else
        body += indent+INDENT "matchSymbol(',');\n";
    body += indent+"}\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"if (separatorCheck == 1)\n";
        body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    return body;
}

std::string StructureType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    bool first = true;
    bool maybeFirst = false;
    for (const std::pair<std::string, const Type *> &member : orderedMembers) {
        const OptionalContainerType *optionalMemberType = nullptr;
        if (generator->settings().skipEmptyFields)
            optionalMemberType = dynamic_cast<const OptionalContainerType *>(member.second);
        std::string memberSerialization;
        std::string subIndent = indent;
        if (optionalMemberType) {
            if (first) {
                body += indent+"bool prev = false;\n";
                body += indent+"write('{');\n";
                maybeFirst = true;
            }
            body += indent+"if ("+optionalMemberType->generateHasValue(("value."+member.first).c_str())+") {\n";
            subIndent += INDENT;
            memberSerialization = generator->generateValueSerialization(optionalMemberType->elementType(), optionalMemberType->generateGetValue(("value."+member.first).c_str()), subIndent);
        } else
            memberSerialization = generator->generateValueSerialization(member.second, "value."+member.first, subIndent);
        if (maybeFirst && !first) {
            body += subIndent+"if (prev)\n";
            body += subIndent+INDENT "write(',');\n";
        }
        body += subIndent+"write(\""+(maybeFirst ? "" : first ? "{" : ",")+"\\\"\" "+generator->getJsonMemberNameLiteral(member.first)+" \"\\\":\");\n"; // TODO optimize, add strlen
        body += memberSerialization;
        if (optionalMemberType) {
            if (maybeFirst)
                body += subIndent+"prev = true;\n";
            body += indent+"}\n";
        } else
            maybeFirst = false;
        first = false;
    }
    body += indent+"write('}');\n";
    return body;
}

void StructureType::addMember(const std::string &name, const Type *type) {
    members.insert(std::make_pair(name, type));
    orderedMembers.emplace_back(name, type);
}

const std::vector<std::pair<std::string, const Type *> > & StructureType::getMembers() const {
    return orderedMembers;
}

const Type * StructureType::findMember(const std::string &name) const {
    std::map<std::string, const Type *>::const_iterator it = members.find(name);
    if (it != members.end())
        return it->second;
    return nullptr;
}

void StructureType::finalize() {
    finalized = true;
}

bool StructureType::isFinalized() const {
    return finalized;
}
