
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
    virtual bool isIncomplete() const override;
    inline virtual const StructureType *structureType() const override { return this; }
    virtual StructureType *incompleteStructureType() override;
    void inheritFrom(const Type *baseType);
    bool absorb(const StructureType *other);
    bool addMember(const Type *type, const std::string &name);
    void completeMembers();
    const Type *findMember(const std::string &name) const;
    virtual int compile(TemplateInstanceCache *instanceCache) override;

private:
    class ParserSwitchTreeCaseGenerator;

    struct Member {
        const Type *type;
        std::string name;

        inline Member() : type(nullptr) { }
        inline Member(const Type *type, const std::string &name) : type(type), name(name) { }
        inline Member(const Type *type, std::string &&name) : type(type), name((std::string &&) name) { }
    };

    std::vector<const Type *> baseTypes;
    std::map<std::string, const Type *> members;
    std::vector<Member> orderedMembers;
    bool complete = false;

};
