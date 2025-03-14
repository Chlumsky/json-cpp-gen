
#include "StructureType.h"

#include <cassert>
#include <set>
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
            body += indent+"\tif ("+checkBits+"&"+checkMask+")\n";
            body += indent+"\t\t"+parserGenerator->generateErrorStatement(ParserGenerator::Error::REPEATED_KEY)+";\n";
        }
        body += indent+"\t"+checkBits+" |= "+checkMask+";\n";
        if (parserGenerator->settings().checkMissingKeys)
            optionalMembers.push_back(!!memberType->optionalContainerType());
        ++i;
    }
    if (parserGenerator->settings().noThrow) {
        body += indent+"\tif (Error error = "+parserGenerator->generateParserFunctionCall(memberType, "value."+caseLabel)+")\n";
        body += indent+"\t\treturn error;\n";
    } else {
        body += indent+"\t"+parserGenerator->generateParserFunctionCall(memberType, "value."+caseLabel)+";\n";
    }
    body += indent+"\tcontinue;\n";
    body += indent+"}\n";
    return body;
}

StructureType::StructureType(const std::string &name, TypeName::Substance nameSubstance) : DirectType(TypeName(name, nameSubstance)) { }

StructureType::StructureType(std::string &&name, TypeName::Substance nameSubstance) : DirectType(TypeName((std::string &&) name, nameSubstance)) { }

std::string StructureType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    std::vector<bool> optionalMembers;
    body += indent+"if (!matchSymbol('{'))\n";
    body += indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    if ((generator->settings().checkMissingKeys || generator->settings().checkRepeatingKeys) && !orderedMembers.empty())
        body += indent+"unsigned long doneKeys["+std::to_string((orderedMembers.size()+31)/32)+"] = { };\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+"int separatorCheck = -1;\n";
    body += indent+"for (; !matchSymbol('}'); "+(generator->settings().strictSyntaxCheck ? "separatorCheck = " : "")+"matchSymbol(',')) {\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"\tif (!separatorCheck)\n";
        body += indent+"\t\t"+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    body += generator->generateValueParse(generator->stringType(), ParserGenerator::COMMON_STRING_BUFFER, indent+"\t");
    body += indent+"\tif (!matchSymbol(':'))\n";
    body += indent+"\t\t"+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    if (!orderedMembers.empty()) {
        std::vector<std::string> labels;
        labels.reserve(orderedMembers.size());
        for (const Member &member : orderedMembers)
            labels.push_back(member.name);
        ParserSwitchTreeCaseGenerator switchTreeCaseGenerator(this, optionalMembers);
        body += generator->generateSwitchTree(&switchTreeCaseGenerator, StringSwitchTree::build(labels.data(), labels.size()).get(), generator->stringType(), ParserGenerator::COMMON_STRING_BUFFER, indent+"\t");
    }
    if (generator->settings().ignoreExtraKeys) {
        if (generator->settings().noThrow) {
            body += indent+"\tif (Error error = skipValue())\n";
            body += indent+"\t\treturn error;\n";
        } else
            body += indent+"\tskipValue();\n";
    } else
        body += indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::UNKNOWN_KEY)+";\n";
    body += indent+"}\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"if (separatorCheck == 1)\n";
        body += indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
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
        body += indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::MISSING_KEY)+";\n";
    }
    return body;
}

std::string StructureType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    if (orderedMembers.empty())
        return indent+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"{}\"")+";\n";
    std::string body;
    bool first = true;
    bool maybeFirst = false;
    for (const Member &member : orderedMembers) {
        if (member.type->name().substance() == TypeName::VIRTUAL)
            generator->resolveVirtualTypename(member.type, this, member.name);
        const OptionalContainerType *optionalMemberType = nullptr;
        if (generator->settings().skipEmptyFields)
            optionalMemberType = member.type->optionalContainerType();
        std::string memberSerialization;
        std::string subIndent = indent;
        if (optionalMemberType) {
            if (first) {
                body += indent+"bool prev = false;\n";
                body += indent+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "'{'")+";\n";
                maybeFirst = true;
            }
            body += indent+"if ("+optionalMemberType->generateHasValue(("value."+member.name).c_str())+") {\n";
            subIndent += "\t";
            memberSerialization = generator->generateValueSerialization(optionalMemberType->elementType(), optionalMemberType->generateGetValue(("value."+member.name).c_str()), subIndent);
        } else
            memberSerialization = generator->generateValueSerialization(member.type, "value."+member.name, subIndent);
        if (maybeFirst && !first)
            body += subIndent+"if (prev) { "+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "','")+"; }\n";
        body += subIndent+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, (std::string("\"")+(maybeFirst ? "" : first ? "{" : ",")+"\\\"\" "+generator->getJsonMemberNameLiteral(member.name)+" \"\\\":\"").c_str())+";\n"; // TODO merge to one literal
        body += memberSerialization;
        if (optionalMemberType) {
            if (maybeFirst)
                body += subIndent+"prev = true;\n";
            body += indent+"}\n";
        } else
            maybeFirst = false;
        first = false;
    }
    body += indent+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "'}'")+";\n";
    return body;
}

bool StructureType::isIncomplete() const {
    return !complete;
}

StructureType *StructureType::incompleteStructureType() {
    return complete ? nullptr : this;
}

void StructureType::inheritFrom(const Type *baseType) {
    baseTypes.push_back(baseType);
}

bool StructureType::absorb(const StructureType *other) {
    if (other) {
        for (const Member &member : other->orderedMembers) {
            if (!addMember(member.type, member.name))
                return false;
        }
    }
    return true;
}

bool StructureType::addMember(const Type *type, const std::string &name) {
    assert(!complete);
    std::map<std::string, const Type *>::iterator it = members.find(name);
    if (it != members.end())
        return false;
    members.insert(it, std::make_pair(name, type));
    orderedMembers.emplace_back(type, name);
    return true;
}

void StructureType::completeMembers() {
    complete = true;
}

const Type *StructureType::findMember(const std::string &name) const {
    std::map<std::string, const Type *>::const_iterator it = members.find(name);
    if (it != members.end())
        return it->second;
    return nullptr;
}

int StructureType::compile(TemplateInstanceCache *instanceCache) {
    bool membersFinal = baseTypes.empty();
    for (Member &member : orderedMembers) {
        if (member.type->isIncomplete())
            membersFinal = false;
        else {
            const Type *actualMemberType = member.type->actualType(instanceCache);
            if (actualMemberType != member.type) {
                membersFinal = false;
                member.type = actualMemberType;
            }
        }
    }
    if (membersFinal)
        return 0;
    for (const Type *baseType : baseTypes) {
        if (const StructureType *baseStructType = baseType->structureType()) {
            if (!baseStructType->baseTypes.empty())
                return BASE_DEPENDENCY_FLAG;
        }
    }
    // Find members inherited from more than one base
    if (baseTypes.size() > 1) {
        std::set<std::string> baseMembers;
        for (const Type *baseType : baseTypes) {
            if (const StructureType *baseStructType = baseType->structureType()) {
                for (const Member &baseMember : baseStructType->orderedMembers) {
                    size_t namespaceEnd = baseMember.name.find_last_of(':');
                    std::string unqualifiedName = namespaceEnd == std::string::npos ? baseMember.name : baseMember.name.substr(namespaceEnd+1);
                    if (!baseMembers.insert(unqualifiedName).second)
                        members.insert(std::make_pair(unqualifiedName, baseMember.type));
                }
            }
        }
    }
    // Add members of base types
    std::vector<Member> fullOrderedMembers;
    for (const Type *baseType : baseTypes) {
        if (const StructureType *baseStructType = baseType->structureType()) {
            for (const Member &baseMember : baseStructType->orderedMembers) {
                if (!baseMember.type->isIncomplete()) {
                    const Type *actualBaseMemberType = baseMember.type->actualType(instanceCache);
                    if (findMember(baseMember.name)) {  // member shadowing
                        if (baseMember.name.find_last_of(':') == std::string::npos) // skip if already inherited from common base
                            fullOrderedMembers.emplace_back(actualBaseMemberType, baseStructType->name().body()+"::"+baseMember.name);
                    } else
                        fullOrderedMembers.push_back(baseMember);
                }
            }
        }
    }
    // Remove incomplete members
    for (Member &member : orderedMembers) {
        if (!member.type->isIncomplete())
            fullOrderedMembers.push_back(std::move(member));
    }
    members.clear();
    for (const Member &member : fullOrderedMembers)
        members.insert(std::make_pair(member.name, member.type));
    orderedMembers = std::move(fullOrderedMembers);
    baseTypes.clear();
    return CHANGE_FLAG;
}
