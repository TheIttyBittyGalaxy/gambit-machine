#ifndef TOKEN_H
#define TOKEN_H

#include <map>
#include <regex>
#include <string>
using namespace std;

struct Token
{
    enum Kind
    {
        InvalidToken,

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
        KeyFn,
        KeyStatic,
        KeyState,
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
        KeyMap,
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

    Token() : kind(Token::InvalidToken), str(""), line(0), column(0){};

    Token(Kind kind, string str, size_t line, size_t column) : kind(kind),
                                                               str(str),
                                                               line(line),
                                                               column(column) {}
};

string to_string(Token t);

extern const map<Token::Kind, string> token_name;
extern const map<Token::Kind, regex> token_match_rules;
extern const map<string, Token::Kind> keyword_match_rules;

#endif