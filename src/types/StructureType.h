
#pragma once

#include <string>
#include <vector>
#include <map>
#include <utility>
#include "DirectType.h"

class StructureType : public DirectType {

public:
    explicit StructureType(const std::string &name);
    explicit StructureType(std::string &&name);
    void inheritFrom(StructureType *baseType);
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    void addMember(const std::string &name, const Type *type);
    const std::vector<std::pair<std::string, const Type *> > & getMembers() const;
    const Type * findMember(const std::string &name) const;
    bool membersFinalized() const;
    void finalizeMembers();
    bool finalizeInheritance();

private:
    class ParserSwitchTreeCaseGenerator;

    std::vector<StructureType *> baseTypes;
    std::map<std::string, const Type *> members;
    std::vector<std::pair<std::string, const Type *> > orderedMembers;
    bool finalizedMembers = false;
    bool inheritanceBeingFinalized = false;

};
