
#include "StructureType.h"

#include "OptionalContainerType.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

class StructureType::ParserSwitchTreeCaseGenerator : public ParserGenerator::SwitchTreeCaseGenerator {

public:
    inline explicit ParserSwitchTreeCaseGenerator(const StructureType *parent) : parent(parent) { }
    virtual std::string operator()(ParserGenerator *parserGenerator, const std::string &caseLabel, const StringType *valueType, const char *value, int knownMinLength, const std::string &indent) override;

private:
    const StructureType *parent;

};

std::string StructureType::ParserSwitchTreeCaseGenerator::operator()(ParserGenerator *parserGenerator, const std::string &caseLabel, const StringType *valueType, const char *value, int, const std::string &indent) {
    std::string body;
    body += indent+"if ("+valueType->generateEqualsStringLiteral(value, parserGenerator->getJsonMemberNameLiteral(caseLabel).c_str())+") {\n";
    if (parserGenerator->settings().noThrow) {
        body += indent+INDENT "if (Error error = "+parserGenerator->generateParserFunctionCall(parent->findMember(caseLabel), "value."+caseLabel)+")\n";
        body += indent+INDENT INDENT "return error;\n";
    } else {
        body += indent+INDENT+parserGenerator->generateParserFunctionCall(parent->findMember(caseLabel), "value."+caseLabel)+";\n";
    }
    body += indent+INDENT "continue;\n";
    body += indent+"}\n";
    return body;
}

StructureType::StructureType(const std::string &name) : DirectType(TypeName(name)) { }

StructureType::StructureType(std::string &&name) : DirectType(TypeName((std::string &&) name)) { }

void StructureType::inheritFrom(StructureType *baseType) {
    baseTypes.push_back(baseType);
}

std::string StructureType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    // TODO make key a class member to reduce the number of allocations
    body += indent+generator->stringType()->name().variableDeclaration("key")+";\n";
    body += indent+"if (!matchSymbol('{'))\n";
    body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+"int separatorCheck = -1;\n";
    body += indent+"for (; !matchSymbol('}'); "+(generator->settings().strictSyntaxCheck ? "separatorCheck = " : "")+"matchSymbol(',')) {\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+INDENT "if (!separatorCheck)\n";
        body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    body += generator->generateValueParse(generator->stringType(), "key", indent+INDENT);
    body += indent+INDENT "if (!matchSymbol(':'))\n";
    body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    if (!orderedMembers.empty()) {
        std::vector<std::string> labels;
        labels.reserve(orderedMembers.size());
        for (const std::pair<std::string, const Type *> &member : orderedMembers)
            labels.push_back(member.first);
        ParserSwitchTreeCaseGenerator switchTreeCaseGenerator(this);
        body += generator->generateSwitchTree(&switchTreeCaseGenerator, StringSwitchTree::build(labels.data(), labels.size()).get(), generator->stringType(), "key", indent+INDENT);
    }
    if (generator->settings().ignoreExtraKeys) {
        if (generator->settings().noThrow) {
            body += indent+INDENT "if (Error error = skipValue())\n";
            body += indent+INDENT INDENT "return error;\n";
        } else
            body += indent+INDENT "skipValue();\n";
    } else
        body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::UNKNOWN_KEY)+";\n";
    body += indent+"}\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"if (separatorCheck == 1)\n";
        body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    return body;
}

std::string StructureType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    if (orderedMembers.empty())
        return indent+"write(\"{}\");\n";
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

const std::vector<std::pair<std::string, const Type *> > &StructureType::getMembers() const {
    return orderedMembers;
}

const Type *StructureType::findMember(const std::string &name) const {
    std::map<std::string, const Type *>::const_iterator it = members.find(name);
    if (it != members.end())
        return it->second;
    return nullptr;
}

bool StructureType::membersFinalized() const {
    return finalizedMembers;
}

void StructureType::finalizeMembers() {
    finalizedMembers = true;
}

bool StructureType::finalizeInheritance() {
    if (inheritanceBeingFinalized) // cyclic inheritance
        return false;
    inheritanceBeingFinalized = true;
    std::vector<std::pair<std::string, const Type *> > fullOrderedMembers;
    for (StructureType *baseType : baseTypes) {
        if (baseType) {
            if (!baseType->finalizeInheritance())
                return false;
            members.insert(baseType->members.begin(), baseType->members.end());
            fullOrderedMembers.insert(fullOrderedMembers.end(), baseType->orderedMembers.begin(), baseType->orderedMembers.end());
        }
    }
    fullOrderedMembers.insert(fullOrderedMembers.end(), orderedMembers.begin(), orderedMembers.end());
    orderedMembers = std::move(fullOrderedMembers);
    baseTypes.clear();
    inheritanceBeingFinalized = false;
    return true;
}
