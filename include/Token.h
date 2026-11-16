
#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// Categorias de elementos que o analisador lexico reconhece.
// O Lexer e o Parser devem utilizar os mesmos tipos.
enum class TokenType {

    // Identificadores e valores literais
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,

    // Palavras reservadas e tipos de dados
    Var, Int, Float, Char, Bool, String,
    True, False, If, Else, While, For,
    Func, Return, Print, Input,

    // Operadores aritmeticos e logicos
    Plus, Minus, Star, Slash,
    Equal, EqualEqual, BangEqual,
    Less, LessEqual, Greater, GreaterEqual,
    AndAnd, OrOr, Bang,

    // Simbolos utilizados na linguagem
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    Colon, Semicolon, Comma,

    // Indica o fim do arquivo
    EndOfFile
};

// Representa um elemento encontrado no codigo-fonte.
struct Token {
    TokenType type;     // Categoria do token
    std::string lexeme; // Texto original encontrado
    int line;           // Linha em que aparece
    int column;         // Coluna em que aparece
};

#endif
