#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <variant>
#include <vector>
using namespace std;

// MACROS //

#define IS(variant_value, T) (holds_alternative<T>(variant_value))
#define AS(variant_value, T) (get<T>(variant_value))

#define IS_PTR(variant_value, T) (holds_alternative<ptr<T>>(variant_value))
#define AS_PTR(variant_value, T) (get<ptr<T>>(variant_value))

#define CREATE(T) (ptr<T>(new T))

// POINTERS //

template <class T>
using ptr = shared_ptr<T>;

template <class T>
using wptr = weak_ptr<T>;

// JSON //

// FIXME: Only _after_ implementing this system did I realise it would probably be more efficient
//        (both in memory and time usage) to implement some form of "JsonSeraliser" that has utilty
//        methods for creating objects and arrays that _directly_ output valid JSON strings. This way
//        you could implement "JsonSeraliser::to_json" overloads for each of your structs, and then
//        create JSON seralisations without having to rebuild your whole data structure!
//
//        Never mind, work for another day!

struct Json;
using JsonArray = vector<Json>;
using JsonObject = map<string, Json>;
struct Json
{
    using Value = variant<string, int, double, bool, monostate, JsonArray, JsonObject>;
    Value value;

    Json() : value(monostate()){};
    Json(string value) : value(value){};
    Json(int value) : value(value){};
    Json(double value) : value(value){};
    Json(bool value) : value(value){};
    Json(monostate value) : value(value){};
    Json(JsonArray value) : value(value){};
    Json(JsonObject value) : value(value){};
};

Json to_json(Json value) { return value; }
Json to_json(string value) { return Json(value); }
Json to_json(int value) { return Json(value); }
Json to_json(double value) { return Json(value); }
Json to_json(bool value) { return Json(value); }
Json to_json(monostate value) { return Json(value); }

template <typename T>
Json to_json(optional<T> value)
{
    return value.has_value() ? to_json(value.value()) : Json();
}

template <typename T>
Json to_json(vector<T> vector)
{
    JsonArray array;
    for (auto elem : vector)
        array.emplace_back(to_json(elem));
    return array;
}

template <typename T>
Json to_json(map<string, T> map)
{
    JsonObject object;
    for (auto entry : map)
        object.emplace(entry.first, to_json(entry.second));
    return object;
}

string json_str(Json json, size_t depth = 1)
{
    auto value = json.value;
    if (IS(value, string))
        return '"' + AS(value, string) + '"'; // FIXME: Does not correctly escape the string

    else if (IS(value, int))
        return to_string(AS(value, int));

    else if (IS(value, double))
        return to_string(AS(value, double));

    else if (IS(value, bool))
        return AS(value, bool) ? "true" : "false";

    else if (IS(value, monostate))
        return "null";

    else if (IS(value, JsonArray))
    {
        auto array = AS(value, JsonArray);
        if (array.size() == 0)
            return "[]";

        string str = "[";
        string sep = "\n" + string(depth, '\t');
        bool first_value = true;

        depth++;
        for (auto elem : array)
        {
            if (first_value)
                first_value = false;
            else
                str += ",";
            str += sep + json_str(elem, depth);
        }

        str += "\n" + string(depth - 2, '\t') + "]";
        return str;
    }

    else if (IS(value, JsonObject))
    {
        auto object = AS(value, JsonObject);
        if (object.size() == 0)
            return "{}";

        string str = "{";
        string sep = "\n" + string(depth, '\t');
        bool first_value = true;

        depth++;
        for (auto entry : object)
        {
            if (first_value)
                first_value = false;
            else
                str += ",";
            str += sep + "\"" + entry.first + "\": " + json_str(entry.second, depth);
        }

        str += "\n" + string(depth - 2, '\t') + "}";
        return str;
    }

    throw "Cannot serialise invalid JSON value.";
}

// TOKENS //

struct Token
{
    enum Kind
    {
        Line,

        Equal,
        NotEqual,
        LessThanEqual,
        GreaterThanEqual,

        Add,
        Sub,
        Mul,
        Div,
        Dot,
        Comma,
        Colon,
        Question,
        Assign,
        Hash,
        ParenL,
        ParenR,
        CurlyL,
        CurlyR,
        SquareL,
        SquareR,
        TrigL,
        TrigR,

        KeyEntity,
        KeyEnum,
        KeyExtend,
        KeyFn,
        KeyStatic,
        KeyState,
        KeyProperty,
        KeyBreak,
        KeyContinue,
        KeyElse,
        KeyFor,
        KeyIf,
        KeyIn,
        KeyLoop,
        KeyMatch,
        KeyReturn,
        KeyUntil,
        KeyChoose,
        KeyFilter,
        KeyTransform,
        KeyAnd,
        KeyOr,
        KeyNot,

        Boolean,
        Number,
        String,
        Identity,
    };

    Kind kind;
    string str;
    size_t line;
    size_t column;

    Token(Kind kind, string str, size_t line, size_t column) : kind(kind),
                                                               str(str),
                                                               line(line),
                                                               column(column) {}
};

const map<Token::Kind, string> token_name = {
    {Token::Line, "Line"},

    {Token::Equal, "Equal"},
    {Token::NotEqual, "NotEqual"},
    {Token::LessThanEqual, "LessThanEqual"},
    {Token::GreaterThanEqual, "GreaterThanEqual"},

    {Token::Add, "Add"},
    {Token::Sub, "Sub"},
    {Token::Mul, "Mul"},
    {Token::Div, "Div"},
    {Token::Dot, "Dot"},
    {Token::Comma, "Comma"},
    {Token::Colon, "Colon"},
    {Token::Question, "Question"},
    {Token::Assign, "Assign"},
    {Token::Hash, "Hash"},
    {Token::ParenL, "ParenL"},
    {Token::ParenR, "ParenR"},
    {Token::CurlyL, "CurlyL"},
    {Token::CurlyR, "CurlyR"},
    {Token::SquareL, "SquareL"},
    {Token::SquareR, "SquareR"},
    {Token::TrigL, "TrigL"},
    {Token::TrigR, "TrigR"},

    {Token::KeyEntity, "KeyEntity"},
    {Token::KeyEnum, "KeyEnum"},
    {Token::KeyExtend, "KeyExtend"},
    {Token::KeyFn, "KeyFn"},
    {Token::KeyStatic, "KeyStatic"},
    {Token::KeyState, "KeyState"},
    {Token::KeyProperty, "KeyProperty"},
    {Token::KeyBreak, "KeyBreak"},
    {Token::KeyContinue, "KeyContinue"},
    {Token::KeyElse, "KeyElse"},
    {Token::KeyFor, "KeyFor"},
    {Token::KeyIf, "KeyIf"},
    {Token::KeyIn, "KeyIn"},
    {Token::KeyLoop, "KeyLoop"},
    {Token::KeyMatch, "KeyMatch"},
    {Token::KeyReturn, "KeyReturn"},
    {Token::KeyUntil, "KeyUntil"},
    {Token::KeyChoose, "KeyChoose"},
    {Token::KeyFilter, "KeyFilter"},
    {Token::KeyTransform, "KeyTransform"},
    {Token::KeyAnd, "KeyAnd"},
    {Token::KeyOr, "KeyOr"},
    {Token::KeyNot, "KeyNot"},

    {Token::Boolean, "Boolean"},
    {Token::Number, "Number"},
    {Token::String, "String"},
    {Token::Identity, "Identity"},
};

const map<Token::Kind, regex> token_match_rules = {
    {Token::Line, regex("\n")},

    {Token::Equal, regex("==")},
    {Token::NotEqual, regex("!=")},
    {Token::LessThanEqual, regex("<=")},
    {Token::GreaterThanEqual, regex(">=")},

    {Token::Add, regex("\\+")},
    {Token::Sub, regex("\\-")},
    {Token::Mul, regex("\\*")},
    {Token::Div, regex("\\\\")},
    {Token::Dot, regex("\\.")},
    {Token::Comma, regex("\\,")},
    {Token::Colon, regex("\\:")},
    {Token::Question, regex("\\?")},
    {Token::Assign, regex("\\=")},
    {Token::Hash, regex("\\#")},
    {Token::ParenL, regex("\\(")},
    {Token::ParenR, regex("\\)")},
    {Token::CurlyL, regex("\\{")},
    {Token::CurlyR, regex("\\}")},
    {Token::SquareL, regex("\\[")},
    {Token::SquareR, regex("\\]")},
    {Token::TrigL, regex("\\<")},
    {Token::TrigR, regex("\\>")},

    {Token::Number, regex("[0-9]+(\\.[0-9]+)?")},
    {Token::String, regex("\"(\\.|.)*?\"")}, // FIXME: This regular expression matches "\", which is incorrect
    {Token::Identity, regex("[a-zA-Z][a-zA-Z0-9_]*")},
};

const map<string, Token::Kind> keyword_match_rules = {
    {"entity", Token::KeyEntity},
    {"enum", Token::KeyEnum},
    {"extend", Token::KeyExtend},
    {"fn", Token::KeyFn},

    {"static", Token::KeyStatic},
    {"state", Token::KeyState},
    {"property", Token::KeyProperty},

    {"break", Token::KeyBreak},
    {"continue", Token::KeyContinue},
    {"else", Token::KeyElse},
    {"for", Token::KeyFor},
    {"if", Token::KeyIf},
    {"in", Token::KeyIn},
    {"loop", Token::KeyLoop},
    {"match", Token::KeyMatch},
    {"return", Token::KeyReturn},
    {"until", Token::KeyUntil},

    {"choose", Token::KeyChoose},
    {"filter", Token::KeyFilter},
    {"transform", Token::KeyTransform},

    {"and", Token::Kind ::KeyAnd},
    {"or", Token::KeyOr},
    {"not", Token::KeyNot},

    {"true", Token::Boolean},
    {"false", Token::Boolean},
};

string to_string(Token t)
{
    if (t.kind == Token::Line)
        return "[" + to_string(t.line) + ":" + to_string(t.column) + " /]";
    return "[" + to_string(t.line) + ":" + to_string(t.column) + " " + token_name.at(t.kind) + " " + t.str + "]";
}

// ERRORS //

vector<string> errors;

void emit_error(string msg)
{
    errors.emplace_back("[Error] " + msg);
}

void emit_error(string msg, size_t line, size_t column)
{
    errors.emplace_back("[Error at " + to_string(line) + ":" + to_string(column) + "] " + msg);
}

void emit_error(string msg, Token t)
{
    emit_error(msg, t.line, t.column);
}

// LEXING //

vector<Token> generate_tokens(string src)
{
    vector<Token> tokens;
    size_t line = 1;
    size_t column = 1;
    size_t position = 0;
    string sub = src;

    auto advance = [&](int amt)
    {
        column += amt;
        position += amt;
        sub = src.substr(position);
    };

    auto advance_line = [&]()
    {
        line += 1;
        column = 1;
        position += 1;
        sub = src.substr(position);
    };

    size_t multi_line_comment_nesting = 0;
    bool is_line_comment = false;
    bool panic_mode = false;

    while (position < src.length())
    {
        bool error_occurred = false;
        string next = sub.substr(0, 1);
        string next_two = sub.substr(0, 2);

        if (is_line_comment)
        {
            if (next == "\n")
            {
                tokens.emplace_back(Token::Line, "\n", line, column);
                advance_line();
                is_line_comment = false;
            }
            else
            {
                advance(1);
            }
        }

        else if (next_two == "/*")
        {
            multi_line_comment_nesting++;
            advance(2);
        }

        else if (multi_line_comment_nesting > 0)
        {
            if (next_two == "*/")
            {
                multi_line_comment_nesting--;
                advance(2);
            }
            else if (next == "\n")
            {
                advance_line();
            }
            else
            {
                advance(1);
            }
        }

        else if (next_two == "//")
        {
            is_line_comment = true;
            advance(2);
        }

        else if (next == "\n")
        {
            tokens.emplace_back(Token::Line, "\n", line, column);
            advance_line();
        }

        else if (next == " " || next == "\t")
        {
            advance(1);
        }

        else
        {
            bool character_parsed = false;
            for (auto rule : token_match_rules)
            {
                smatch info;
                if (regex_search(sub, info, rule.second, regex_constants::match_continuous))
                {
                    Token::Kind kind = rule.first;
                    string str = info.str();

                    if (kind == Token::Identity)
                    {
                        for (auto key_rule : keyword_match_rules)
                        {
                            if (str == key_rule.first)
                            {
                                kind = key_rule.second;
                                break;
                            }
                        }
                    }

                    tokens.emplace_back(kind, str, line, column);
                    advance(info.length());
                    character_parsed = true;
                    break;
                }
            }

            if (!character_parsed)
            {
                if (!panic_mode)
                    emit_error("Unrecognised character " + next, line, column); // FIXME: Make this a proper error message.}
                advance(1);
                panic_mode = true;
                error_occurred = true;
            }
        }

        panic_mode = error_occurred;
    }

    return tokens;
}

// PROGRAM MODEL //

struct Program;
struct Scope;

struct NativeType;

struct EnumType;
struct EnumValue;

struct Entity;
struct EntityField;

struct Literal;
using Expression = std::variant<ptr<Literal>, ptr<EnumValue>>;

// Program

struct Program
{
    ptr<Scope> global_scope;
};

struct Scope
{
    using LookupValue = variant<ptr<NativeType>, ptr<EnumType>, ptr<Entity>>;
    wptr<Scope> parent;
    map<string, LookupValue> lookup;
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

struct NativeType
{
    string identity;
    string cpp_identity;
};

struct Entity
{
    string identity;
    map<string, ptr<EntityField>> fields;
};

struct EntityField
{
    string identity;
    string type;
    bool is_static = false;
    bool is_property = false;
    optional<Expression> default_value;
};

// Expressions

struct Literal
{
    variant<double, int, bool, string> value;
};

// SERIALISE PROGRAM MODEL //

// FIXME: Implement versions of these to_json functions that instead of outputing `string`,
//        output a `json` data structure, and then implement `to_string(json)`. This way
//        `to_string(json)` can handle pretty printing a JSON output, and the `to_json`
//        functions can just crudely convert to the correct data structure.

Json to_json(ptr<Program> program);
Json to_json(ptr<Scope> scope);
Json to_json(ptr<NativeType> native_type);
Json to_json(ptr<EnumType> enum_type);
Json to_json(ptr<EnumValue> enum_value);
Json to_json(ptr<Entity> entity);
Json to_json(ptr<EntityField> field);
Json to_json(Scope::LookupValue entity);
Json to_json(ptr<Literal> literal);
Json to_json(Expression expression);

Json to_json(ptr<Program> program)
{
    return JsonObject({
        {"node", string("Program")},
        {"global_scope", to_json(program->global_scope)},
    });
}

Json to_json(ptr<Scope> scope)
{
    return JsonObject({
        {"node", string("Scope")},
        {"lookup", to_json<Scope::LookupValue>(scope->lookup)},
    });
}

Json to_json(ptr<NativeType> native_type)
{
    return JsonObject({
        {"node", string("NativeType")},
        {"identity", native_type->identity},
        {"cpp_identity", native_type->cpp_identity},
    });
}

Json to_json(ptr<EnumType> enum_type)
{
    return JsonObject({
        {"node", string("EnumType")},
        {"identity", enum_type->identity},
        {"values", to_json<ptr<EnumValue>>(enum_type->values)},
    });
}

Json to_json(ptr<EnumValue> enum_value)
{
    return JsonObject({
        {"node", string("EnumValue")},
        {"identity", enum_value->identity},
    });
}

Json to_json(ptr<Entity> entity)
{
    return JsonObject({
        {"node", string("Entity")},
        {"identity", entity->identity},
        {"fields", to_json<ptr<EntityField>>(entity->fields)},
    });
}

Json to_json(ptr<EntityField> field)
{
    return JsonObject({
        {"node", string("EntityField")},
        {"identity", field->identity},
        {"type", field->type},
        {"is_static", field->is_static},
        {"is_property", field->is_property},
        {"default_value", to_json<Expression>(field->default_value)},
    });
}

Json to_json(Scope::LookupValue value)
{
    if (IS_PTR(value, NativeType))
        return to_json(AS_PTR(value, NativeType));
    else if (IS_PTR(value, EnumType))
        return to_json(AS_PTR(value, EnumType));
    else if (IS_PTR(value, Entity))
        return to_json(AS_PTR(value, Entity));

    throw "Unable to serialise Scope::LookupValue";
}

Json to_json(ptr<Literal> literal)
{
    auto value = literal->value;

    if (IS(value, double))
        return JsonObject({
            {"node", string("Literal")},
            {"value", AS(value, double)},
        });

    if (IS(value, int))
        return JsonObject({
            {"node", string("Literal")},
            {"value", AS(value, int)},
        });

    if (IS(value, bool))
        return JsonObject({
            {"node", string("Literal")},
            {"value", AS(value, bool)},
        });

    if (IS(value, string))
        return JsonObject({
            {"node", string("Literal")},
            {"value", AS(value, string)},
        });

    throw "Unable to seralise Literal node";
}

Json to_json(Expression expression)
{
    if (IS_PTR(expression, Literal))
        return to_json(AS_PTR(expression, Literal));

    throw "Unable to serialise Expression node";
}

// PARSER //

class Parser
{
public:
    ptr<Program> parse(vector<Token> new_tokens)
    {
        tokens = new_tokens;
        current_token = 0;
        panic_mode = false;

        parse_program();
        return program;
    };

private:
    vector<Token> tokens;
    ptr<Program> program = nullptr;
    size_t current_token;
    bool panic_mode;

    class Error : public std::exception
    {
    public:
        string msg;
        Token token;

        Error(string msg, Token token) : msg(msg), token(token){};

        string what()
        {
            return msg;
        }
    };

    bool end_of_file()
    {
        return current_token >= tokens.size();
    }

    bool peek(Token::Kind kind)
    {
        if (end_of_file())
            return false;

        if (kind == Token::Line)
            return tokens.at(current_token).kind == kind;

        size_t i = current_token;
        while (tokens.at(i).kind == Token::Line)
            i++;
        return tokens.at(i).kind == kind;
    };

    Token eat(Token::Kind kind)
    {
        if (end_of_file())
        {
            Token token = tokens.at(tokens.size() - 1);
            throw Error("Expected " + token_name.at(kind) + ", got end of file", token);
        }

        Token token = tokens.at(current_token);

        if (!peek(kind))
        {
            Token::Kind other = token.kind;
            throw Error("Expected " + token_name.at(kind) + ", got " + token_name.at(other), token);
        }

        if (kind != Token::Line)
        {
            while (token.kind == Token::Line)
            {
                current_token++;
                token = tokens.at(current_token);
            }
        }

        current_token++;
        return token;
    };

    bool match(Token::Kind kind)
    {
        if (peek(kind))
        {
            eat(kind);
            return true;
        }
        return false;
    };

    void parse_program()
    {
        program = CREATE(Program);
        program->global_scope = CREATE(Scope);

        while (current_token < tokens.size())
        {
            try
            {
                while (match(Token::Line))
                    if (end_of_file())
                        break;

                if (peek_entity_definition())
                {
                    panic_mode = false;
                    parse_entity_definition(program->global_scope);
                }
                else if (peek_enum_definition())
                {
                    panic_mode = false;
                    parse_enum_definition(program->global_scope);
                }
                else
                {
                    throw Error("Expected Entity definition", tokens.at(current_token));
                }
            }
            catch (Error err)
            {
                if (!panic_mode)
                    emit_error(err.msg, err.token);
                panic_mode = true;

                while (!match(Token::Line))
                    eat(tokens.at(current_token).kind);
            }
        }
    };

    bool peek_entity_definition()
    {
        return peek(Token::KeyExtend);
    }

    void parse_entity_definition(ptr<Scope> scope)
    {
        auto entity = CREATE(Entity);

        eat(Token::KeyExtend);
        entity->identity = eat(Token::Identity).str;

        // FIXME: Use some form of "declare" function, rather than adding to the map directly
        scope->lookup.insert({entity->identity, entity});

        eat(Token::CurlyL);
        while (peek_entity_field())
        {
            parse_entity_field(scope, entity);
        }
        eat(Token::CurlyR);
    };

    bool peek_entity_field()
    {
        return peek(Token::KeyStatic) || peek(Token::KeyState) || peek(Token::KeyProperty);
    }

    void parse_entity_field(ptr<Scope> scope, ptr<Entity> entity)
    {
        auto field = CREATE(EntityField);
        if (match(Token::KeyStatic))
            field->is_static = true;
        else if (match(Token::KeyProperty))
            field->is_property = true;
        else
            eat(Token::KeyState);

        field->type = eat(Token::Identity).str;
        field->identity = eat(Token::Identity).str;

        if (match(Token::Assign))
        {
            field->default_value = parse_expression();
        }

        entity->fields.insert({field->identity, field});
    }

    bool peek_enum_definition()
    {
        return peek(Token::KeyEnum);
    }

    void parse_enum_definition(ptr<Scope> scope)
    {
        auto enum_type = CREATE(EnumType);

        eat(Token::KeyEnum);
        enum_type->identity = eat(Token::Identity).str;

        // FIXME: Use some form of "declare" function, rather than adding to the map directly
        scope->lookup.insert({enum_type->identity, enum_type});

        eat(Token::CurlyL);
        do
        {
            auto enum_value = CREATE(EnumValue);
            enum_value->identity = eat(Token::Identity).str;
            enum_type->values.emplace_back(enum_value);
        } while (match(Token::Comma));

        eat(Token::CurlyR);
    }

    bool peek_expression()
    {
        return peek_literal();
    }

    Expression parse_expression()
    {
        if (peek_literal())
        {
            return parse_literal();
        }

        throw Error("Expected expression", tokens.at(current_token));
    }

    bool peek_literal()
    {
        return peek(Token::Number) || peek(Token::String) || peek(Token::Boolean);
    }

    ptr<Literal> parse_literal()
    {
        auto literal = CREATE(Literal);
        if (peek(Token::Number))
        {
            // FIXME: Treat num and int literals differently
            Token t = eat(Token::Number);
            literal->value = stod(t.str);
        }
        else if (peek(Token::String))
        {
            Token t = eat(Token::String);
            literal->value = t.str;
        }
        else if (peek(Token::Boolean))
        {
            Token t = eat(Token::Boolean);
            literal->value = t.str == "true";
        }
        else
        {
            throw Error("Expected literal", tokens.at(current_token));
        }

        return literal;
    }
};

// MAIN //

int main(int argc, char *argv[])
{
    // FIXME: Allow the path to the source project to be passed directly, rather than infered.
    //        I've just made it easier this way for now to make testing easier for myself.

    // FIXME: Allow for generation of multiple source files.

    string game = (argc == 2) ? (string)argv[1] : "tic-tac-toe";
    string src_path = "game/" + game + "/main.gambit";

    // File loading

    ifstream src_file;
    src_file.open(src_path, ios::in);
    if (!src_file)
    {
        cout << "Source file " << src_path << " could not be loaded" << endl;
        return 0;
    }

    string src((istreambuf_iterator<char>(src_file)), istreambuf_iterator<char>());
    src_file.close();

    // Compile

    cout << "\nLEXING" << endl;
    auto tokens = generate_tokens(src);

    // for (auto t : tokens)
    //     cout << to_string(t) << endl;
    // cout << endl;

    // for (auto t : tokens)
    //     cout << t.str << " ";
    // cout << endl;

    cout << "\nPARSING" << endl;
    Parser parser;
    auto program = parser.parse(tokens);

    std::ofstream parser_output;
    parser_output.open("local/parser_output.json");
    parser_output << json_str(to_json(program));
    cout << "Saved parser output to local/parser_output.json" << endl;

    cout << "\nERRORS" << endl;
    for (auto err : errors)
        cout << err << endl;
    cout << endl;

    return 0;
}