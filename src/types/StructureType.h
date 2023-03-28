
#pragma once

#include <string>
#include <vector>
#include <map>
#include <utility>
#include "DirectType.h"

class StructureType : public DirectType {

public:
    explicit StructureType(const std::string &name, TypeName::Substance nameSubstance = TypeName::ACTUAL);
    explicit StructureType(std::string &&name, TypeName::Substance nameSubstance = TypeName::ACTUAL);
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    inline virtual const StructureType *structureType() const override { return this; }
    virtual StructureType *structurePrototype() override;
    void inheritFrom(const Type *baseType);
    bool absorb(const StructureType *other);
    bool addMember(const std::string &name, const Type *type);
    const std::vector<std::pair<std::string, const Type *> > &getMembers() const;
    const Type *findMember(const std::string &name) const;
    bool membersFinalized() const;
    void finalizeMembers();
    virtual int compile() override;

private:
    class ParserSwitchTreeCaseGenerator;

    std::vector<const Type *> baseTypes;
    std::map<std::string, const Type *> members;
    std::vector<std::pair<std::string, const Type *> > orderedMembers;
    bool finalizedMembers = false;

};
