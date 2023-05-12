#ifndef RESOLVER_H
#define RESOLVER_H

#include "apm.h"
#include "utilty.h"
#include <optional>
using namespace std;

class Resolver
{
public:
    void resolve(ptr<Program> program);

private:
    ptr<Program> program = nullptr;

    void resolve_program(ptr<Program> program);
    void resolve_code_block(ptr<CodeBlock> code_block, optional<Pattern> pattern_hint = {});
    void resolve_scope(ptr<Scope> scope);
    void resolve_scope_lookup_value_property_signatures_pass(Scope::LookupValue value, ptr<Scope> scope);
    void resolve_scope_lookup_value_final_pass(Scope::LookupValue value, ptr<Scope> scope);

    void resolve_optional_pattern(ptr<OptionalPattern> optional_pattern, ptr<Scope> scope);
    [[nodiscard]] Pattern resolve_pattern(Pattern pattern, ptr<Scope> scope);

    [[nodiscard]] Expression resolve_expression(Expression expression, ptr<Scope> scope, optional<Pattern> pattern_hint = {});
    void resolve_list_value(ptr<ListValue> list, ptr<Scope> scope, optional<Pattern> pattern_hint = {});
    void resolve_instance_list(ptr<InstanceList> list, ptr<Scope> scope, optional<Pattern> pattern_hint = {});
    void resolve_match(ptr<Match> match, ptr<Scope> scope, optional<Pattern> pattern_hint = {});
    void resolve_property_index(ptr<PropertyIndex> property_index, ptr<Scope> scope, optional<Pattern> pattern_hint = {});
    void resolve_unary(ptr<Unary> unary, ptr<Scope> scope, optional<Pattern> pattern_hint = {});
    void resolve_binary(ptr<Binary> binary, ptr<Scope> scope, optional<Pattern> pattern_hint = {});

    [[nodiscard]] Statement resolve_statement(Statement statement, ptr<Scope> scope, optional<Pattern> pattern_hint = {});
};

#endif