#pragma once
#ifndef APM_H
#define APM_H

#include "token.h"
#include "utilty.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
using namespace std;

// Forward declarations

struct Program;
struct CodeBlock;
struct Scope;

struct UnresolvedIdentity;

struct Variable;

struct OptionalPattern;

struct EnumType;
struct EnumValue;

struct Entity;

struct StateProperty;
struct FunctionProperty;

struct NativeType;
struct InvalidPattern;

using Pattern = variant<
    ptr<UnresolvedIdentity>,
    ptr<OptionalPattern>,
    ptr<InvalidPattern>,
    ptr<EnumType>,
    ptr<Entity>,
    ptr<NativeType>>;

struct Literal;
struct ListValue;
struct InstanceList;
struct Unary;
struct Binary;
struct PropertyIndex;
struct Match;
struct InvalidValue;
using Expression = variant<
    ptr<UnresolvedIdentity>,
    ptr<Variable>,
    ptr<EnumValue>,
    ptr<Literal>,
    ptr<ListValue>,
    ptr<InstanceList>,
    ptr<Unary>,
    ptr<Binary>,
    ptr<PropertyIndex>,
    ptr<Match>,
    ptr<InvalidValue>>;

using Statement = variant<
    Expression,
    ptr<CodeBlock>>;

// Program

struct Program
{
    ptr<Scope> global_scope;
};

struct CodeBlock
{
    bool singleton_block = false;
    ptr<Scope> scope;
    vector<Statement> statements;
};

struct Scope
{
    struct OverloadedIdentity;

    using LookupValue = variant<
        ptr<Variable>,
        ptr<NativeType>,
        ptr<EnumType>,
        ptr<Entity>,
        ptr<StateProperty>,
        ptr<FunctionProperty>,
        ptr<OverloadedIdentity>>;

    struct OverloadedIdentity
    {
        string identity;
        vector<LookupValue> overloads;
    };

    wptr<Scope> parent;
    unordered_map<string, LookupValue> lookup;
};

// Unresolved Identity

struct UnresolvedIdentity
{
    string identity;
    Token token;
};

// Variables

struct Variable
{
    string identity;
    Pattern pattern;
};

// Patterns

struct OptionalPattern
{
    Pattern pattern;
};

struct InvalidPattern
{
    // FIXME: Include the token of node that eventually became this invalid pattern
};

// Enums

struct EnumType
{
    string identity;
    vector<ptr<EnumValue>> values;
};

struct EnumValue
{
    string identity;
};

// Entities

struct Entity
{
    string identity;
};

// Properties

struct StateProperty
{
    string identity;
    Pattern pattern;
    ptr<Scope> scope;
    vector<ptr<Variable>> parameters;
    optional<Expression> initial_value;
};

struct FunctionProperty
{
    string identity;
    Pattern pattern;
    ptr<Scope> scope;
    vector<ptr<Variable>> parameters;
    optional<ptr<CodeBlock>> body;
};

// Types

struct NativeType
{
    string identity;
    string cpp_identity;
};

// Expressions

// FIXME: Literals should contain a reference to their pattern
struct Literal
{
    variant<double, int, bool, string> value;
};

struct ListValue
{
    vector<Expression> values;
};

// FIXME: For now I've named value lists (lists of values that correspond to patterns)
//        `InstanceLists` to avoid confusion with 'list' values. Go in and fix the
//        terminology at some point. (perhaps 'lists' need to be called arrays?)
struct InstanceList
{
    vector<Expression> values;
};

struct Unary
{
    string op;
    Expression value;
};

struct Binary
{
    string op;
    Expression lhs;
    Expression rhs;
};

struct PropertyIndex
{
    Expression expr;
    variant<
        ptr<UnresolvedIdentity>,
        ptr<StateProperty>,
        ptr<FunctionProperty>>
        property;
};

struct Match
{
    struct Rule
    {
        Expression pattern;
        Expression result;
    };
    Expression subject;
    vector<Rule> rules;
};

struct InvalidValue
{
    // FIXME: Include the token of node that eventually became this invalid value
};

// Methods

string identity_of(Scope::LookupValue value);
bool directly_declared_in_scope(ptr<Scope> scope, string identity);
bool declared_in_scope(ptr<Scope> scope, string identity);
bool is_overloadable(Scope::LookupValue value);

void declare(ptr<Scope> scope, Scope::LookupValue value);
Scope::LookupValue fetch(ptr<Scope> scope, string identity);
vector<Scope::LookupValue> fetch_all_overloads(ptr<Scope> scope, string identity);

Pattern determine_expression_pattern(Expression expr);
bool is_pattern_subset_of_superset(Pattern subset, Pattern superset);
bool does_instance_list_match_parameters(ptr<InstanceList> instance_list, vector<ptr<Variable>> parameters);

// JSON Serialisation

string to_json(const ptr<Program> &program, const size_t &depth = 0);
string to_json(const ptr<CodeBlock> &code_block, const size_t &depth = 0);
string to_json(const Scope::LookupValue &lookup_value, const size_t &depth = 0);
string to_json(const ptr<Scope::OverloadedIdentity> &lookup_index, const size_t &depth = 0);
string to_json(const ptr<Scope> &scope, const size_t &depth = 0);
string to_json(const ptr<UnresolvedIdentity> &unresolved_identity, const size_t &depth = 0);
string to_json(const ptr<Variable> &unresolved_identity, const size_t &depth = 0);
string to_json(const ptr<OptionalPattern> &optional_pattern, const size_t &depth = 0);
string to_json(const ptr<InvalidPattern> &invalid_type, const size_t &depth = 0);
string to_json(const ptr<EnumType> &enum_type, const size_t &depth = 0);
string to_json(const ptr<EnumValue> &enum_value, const size_t &depth = 0);
string to_json(const ptr<Entity> &entity, const size_t &depth = 0);
string to_json(const ptr<StateProperty> &state, const size_t &depth = 0);
string to_json(const ptr<FunctionProperty> &state, const size_t &depth = 0);
string to_json(const ptr<NativeType> &native_type, const size_t &depth = 0);
string to_json(const Pattern &pattern, const size_t &depth = 0);
string to_json(const ptr<Unary> &unary, const size_t &depth = 0);
string to_json(const ptr<Binary> &binary, const size_t &depth = 0);
string to_json(const Match::Rule &rule, const size_t &depth = 0);
string to_json(const ptr<PropertyIndex> &property_index, const size_t &depth = 0);
string to_json(const ptr<Match> &match, const size_t &depth = 0);
string to_json(const ptr<InvalidValue> &invalid_value, const size_t &depth = 0);
string to_json(const ptr<Literal> &literal, const size_t &depth = 0);
string to_json(const ptr<InstanceList> &list_value, const size_t &depth = 0);
string to_json(const ptr<ListValue> &list_value, const size_t &depth = 0);
string to_json(const Expression &expression, const size_t &depth = 0);
string to_json(const Statement &statement, const size_t &depth = 0);

#endif