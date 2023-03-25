
#include "StructureType.h"

#include "OptionalContainerType.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

class StructureType::ParserSwitchTreeCaseGenerator : public ParserGenerator::SwitchTreeCaseGenerator {

public:
    inline ParserSwitchTreeCaseGenerator(const StructureType *parent, std::vector<bool> &optionalMembers) : parent(parent), optionalMembers(optionalMembers), i(0) { }
    virtual std::string operator()(ParserGenerator *parserGenerator, const std::string &caseLabel, const StringType *valueType, const char *value, int knownMinLength, const std::string &indent) override;

private:
    const StructureType *parent;
    std::vector<bool> &optionalMembers;
    int i;

};

static std::string hexUint32(unsigned long x) {
    char buffer[] = "0x00000000";
    char *c = buffer+sizeof(buffer)-1;
    for (int i = 0; i < 8; ++i, x >>= 4)
        *--c = "0123456789abcdef"[x&0x0f];
    return std::string(buffer);
}

std::string StructureType::ParserSwitchTreeCaseGenerator::operator()(ParserGenerator *parserGenerator, const std::string &caseLabel, const StringType *valueType, const char *value, int, const std::string &indent) {
    const Type *memberType = parent->findMember(caseLabel);
    if (memberType->name().substance() == TypeName::VIRTUAL)
        parserGenerator->resolveVirtualTypename(memberType, parent, caseLabel);
    std::string body;
    body += indent+"if ("+valueType->generateEqualsStringLiteral(value, parserGenerator->getJsonMemberNameLiteral(caseLabel).c_str())+") {\n";
    if (parserGenerator->settings().checkMissingKeys || parserGenerator->settings().checkRepeatingKeys) {
        std::string checkBits = "doneKeys["+std::to_string(i/32)+"]";
        std::string checkMask = hexUint32(1ul<<(i%32));
        if (parserGenerator->settings().checkRepeatingKeys) {
            body += indent+INDENT "if ("+checkBits+"&"+checkMask+")\n";
            body += indent+INDENT INDENT+parserGenerator->generateErrorStatement(ParserGenerator::Error::REPEATED_KEY)+";\n";
        }
        body += indent+INDENT+checkBits+" |= "+checkMask+";\n";
        if (parserGenerator->settings().checkMissingKeys)
            optionalMembers.push_back(!!dynamic_cast<const OptionalContainerType *>(memberType));
        ++i;
    }
    if (parserGenerator->settings().noThrow) {
        body += indent+INDENT "if (Error error = "+parserGenerator->generateParserFunctionCall(memberType, "value."+caseLabel)+")\n";
        body += indent+INDENT INDENT "return error;\n";
    } else {
        body += indent+INDENT+parserGenerator->generateParserFunctionCall(memberType, "value."+caseLabel)+";\n";
    }
    body += indent+INDENT "continue;\n";
    body += indent+"}\n";
    return body;
}

StructureType::StructureType(const std::string &name, TypeName::Substance nameSubstance) : DirectType(TypeName(name, nameSubstance)) { }

StructureType::StructureType(std::string &&name, TypeName::Substance nameSubstance) : DirectType(TypeName((std::string &&) name, nameSubstance)) { }

std::string StructureType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    std::vector<bool> optionalMembers;
    // TODO make key a class member to reduce the number of allocations
    body += indent+generator->stringType()->name().variableDeclaration("key")+";\n";
    body += indent+"if (!matchSymbol('{'))\n";
    body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    if ((generator->settings().checkMissingKeys || generator->settings().checkRepeatingKeys) && !orderedMembers.empty())
        body += indent+"unsigned long doneKeys["+std::to_string((orderedMembers.size()+31)/32)+"] = { };\n";
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
        ParserSwitchTreeCaseGenerator switchTreeCaseGenerator(this, optionalMembers);
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
    if (generator->settings().checkMissingKeys && !orderedMembers.empty()) {
        { // Consider optional members done
            size_t i = 0, group = 0;
            while (i < optionalMembers.size()) {
                unsigned long optionalMask = 0ul;
                for (int j = 0; j < 32 && i < optionalMembers.size(); ++j, ++i) {
                    if (optionalMembers[i])
                        optionalMask |= 1<<j;
                }
                if (optionalMask)
                    body += indent+"doneKeys["+std::to_string(group)+"] |= "+hexUint32(optionalMask)+";\n";
                ++group;
            }
        }
        size_t groups = (orderedMembers.size()+31)/32;
        body += indent+"if (!(";
        for (size_t i = 0; i < groups-1; ++i)
            body += "doneKeys["+std::to_string(i)+"] == 0xffffffff && ";
        body += "doneKeys["+std::to_string(groups-1)+"] == "+hexUint32(~(orderedMembers.size()%32 ? ~0ul<<(orderedMembers.size()%32) : 0ul))+"))\n";
        body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::MISSING_KEY)+";\n";
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
        if (member.second->name().substance() == TypeName::VIRTUAL)
            generator->resolveVirtualTypename(member.second, this, member.first);
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

void StructureType::inheritFrom(StructureType *baseType) {
    baseTypes.push_back(baseType);
}

bool StructureType::absorb(const StructureType *other) {
    if (other) {
        for (const std::pair<std::string, const Type *> &member : other->orderedMembers) {
            if (!addMember(member.first, member.second))
                return false;
        }
    }
    return true;
}

bool StructureType::addMember(const std::string &name, const Type *type) {
    std::map<std::string, const Type *>::iterator it = members.find(name);
    if (it != members.end())
        return false;
    members.insert(it, std::make_pair(name, type));
    orderedMembers.emplace_back(name, type);
    return true;
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
            for (const std::pair<std::string, const Type *> &baseMember : baseType->orderedMembers) {
                std::map<std::string, const Type *>::iterator it = members.find(baseMember.first);
                if (it == members.end()) {
                    members.insert(it, baseMember);
                    fullOrderedMembers.push_back(baseMember);
                } else { // member shadowing
                    std::string qualifiedName = baseType->name().body()+"::"+baseMember.first;
                    members.insert(std::make_pair(qualifiedName, baseMember.second));
                    fullOrderedMembers.emplace_back(std::move(qualifiedName), baseMember.second);
                }
            }
        }
    }
    fullOrderedMembers.insert(fullOrderedMembers.end(), orderedMembers.begin(), orderedMembers.end());
    orderedMembers = std::move(fullOrderedMembers);
    baseTypes.clear();
    inheritanceBeingFinalized = false;
    return true;
}
