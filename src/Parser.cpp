#include "Parser.h"
#include <stdexcept>
#include <utility>

// Recebe os tokens gerados pelo Lexer.
Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens) {

    // A lista deve terminar com EndOfFile.
    if (tokens.empty() ||
        tokens.back().type != TokenType::EndOfFile) {

        throw std::runtime_error(
            "Lista de tokens invalida: falta EndOfFile"
        );
    }
}

// Consulta o token atual sem avancar.
const Token& Parser::atual() const {
    return tokens[posicao];
}

// Verifica se o token possui o tipo informado.
bool Parser::eh(TokenType tipo) const {
    return atual().type == tipo;
}

// Consome um token quando o tipo corresponde.
bool Parser::aceitar(TokenType tipo) {

    if (!eh(tipo)) {
        return false;
    }

    if (!eh(TokenType::EndOfFile)) {
        ++posicao;
    }

    return true;
}

// Exige um token especifico.
// Se nao encontrar, gera um erro de sintaxe.
Token Parser::consumir(
    TokenType tipo,
    const std::string& mensagem
) {
    if (!eh(tipo)) {
        erro(mensagem);
    }

    Token token = atual();
    aceitar(tipo);

    return token;
}

// Informa onde ocorreu um erro sintatico.
[[noreturn]]
void Parser::erro(const std::string& mensagem) const {

    throw std::runtime_error(
        "Linha " + std::to_string(atual().line) +
        ", coluna " + std::to_string(atual().column) +
        ": " + mensagem
    );
}

// Funcao principal do Parser.
//
// Cria a raiz da AST e processa os comandos
// ate encontrar o final do arquivo.
std::unique_ptr<ASTNode> Parser::analisar() {

    auto raiz = std::make_unique<ASTNode>(
        NodeType::Program
    );

    while (!eh(TokenType::EndOfFile)) {
        raiz->addChild(comando());
    }

    return raiz;
}

// Identifica qual comando sera analisado.
//
// Nesta primeira parte, os comandos aceitos
// sao declaracao, atribuicao, print e input.
std::unique_ptr<ASTNode> Parser::comando() {

    if (eh(TokenType::Var)) {
        return declaracao();
    }

    if (eh(TokenType::Print)) {
        return impressao();
    }

    if (eh(TokenType::Input)) {
        return entrada();
    }

    if (eh(TokenType::Identifier)) {
        return atribuicao();
    }

    erro("comando inesperado nesta versao do Parser");
}

// Reconhece declaracoes como:
// var idade: int = 20;
//
// O valor inicial e opcional.
std::unique_ptr<ASTNode> Parser::declaracao() {

    Token inicio = consumir(
        TokenType::Var,
        "esperado 'var'"
    );

    Token nome = consumir(
        TokenType::Identifier,
        "esperado nome da variavel"
    );

    consumir(
        TokenType::Colon,
        "esperado ':' depois do nome"
    );

    // Verifica se foi informado um dos cinco
    // tipos definidos pela linguagem MACSLang.
    if (!eh(TokenType::Int) &&
        !eh(TokenType::Float) &&
        !eh(TokenType::Char) &&
        !eh(TokenType::Bool) &&
        !eh(TokenType::String)) {

        erro(
            "esperado um tipo: int, float, "
            "char, bool ou string"
        );
    }

    Token tipo = atual();
    aceitar(tipo.type);

    // Cria o no da declaracao.
    auto no = std::make_unique<ASTNode>(
        NodeType::VarDecl,
        nome.lexeme,
        tipo.lexeme,
        inicio.line,
        inicio.column
    );

    // Se houver '=' processa o valor inicial.
    // A expressao sera o primeiro filho do no.
    if (aceitar(TokenType::Equal)) {
        no->addChild(expressao());
    }

    // Toda declaracao termina com ';'.
    consumir(
        TokenType::Semicolon,
        "esperado ';' depois da declaracao"
    );

    return no;
}

// Reconhece atribuicoes como:
// idade = idade + 1;
std::unique_ptr<ASTNode> Parser::atribuicao() {

    Token nome = consumir(
        TokenType::Identifier,
        "esperado nome da variavel"
    );

    consumir(
        TokenType::Equal,
        "esperado '=' na atribuicao"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::Assignment,
        nome.lexeme,
        "",
        nome.line,
        nome.column
    );

    // A expressao atribuida sera o primeiro filho.
    no->addChild(expressao());

    consumir(
        TokenType::Semicolon,
        "esperado ';' depois da atribuicao"
    );

    return no;
}

// Reconhece o comando de saida:
// print(idade);
// print(10 + 5);
std::unique_ptr<ASTNode> Parser::impressao() {

    Token inicio = consumir(
        TokenType::Print,
        "esperado 'print'"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' depois de print"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::Print,
        "",
        "",
        inicio.line,
        inicio.column
    );

    // A expressao impressa fica no primeiro filho.
    no->addChild(expressao());

    consumir(
        TokenType::RightParen,
        "esperado ')' depois da expressao"
    );

    consumir(
        TokenType::Semicolon,
        "esperado ';' depois de print"
    );

    return no;
}

// Reconhece o comando de entrada:
// input(idade);
std::unique_ptr<ASTNode> Parser::entrada() {

    Token inicio = consumir(
        TokenType::Input,
        "esperado 'input'"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' depois de input"
    );

    Token nome = consumir(
        TokenType::Identifier,
        "esperado nome da variavel em input"
    );

    consumir(
        TokenType::RightParen,
        "esperado ')' depois do nome"
    );

    consumir(
        TokenType::Semicolon,
        "esperado ';' depois de input"
    );

    // O nome da variavel fica no campo value.
    return std::make_unique<ASTNode>(
        NodeType::Input,
        nome.lexeme,
        "",
        inicio.line,
        inicio.column
    );
}

// Analisa soma e subtracao.
//
// Primeiro processamos multiplicacoes para
// respeitar a prioridade das operacoes.
std::unique_ptr<ASTNode> Parser::expressao() {

    auto esquerda = multiplicacao();

    while (eh(TokenType::Plus) ||
           eh(TokenType::Minus)) {

        Token operador = atual();
        aceitar(operador.type);

        auto direita = multiplicacao();

        auto no = std::make_unique<ASTNode>(
            NodeType::BinaryOp,
            operador.lexeme,
            "",
            operador.line,
            operador.column
        );

        // Primeiro filho: operando esquerdo.
        no->addChild(std::move(esquerda));

        // Segundo filho: operando direito.
        no->addChild(std::move(direita));

        // O resultado se torna a nova expressao.
        esquerda = std::move(no);
    }

    return esquerda;
}

// Processa multiplicacao e divisao.
//
// Essas operacoes possuem prioridade
// maior que soma e subtracao.
std::unique_ptr<ASTNode> Parser::multiplicacao() {

    auto esquerda = unario();

    while (eh(TokenType::Star) ||
           eh(TokenType::Slash)) {

        Token operador = atual();
        aceitar(operador.type);

        auto direita = unario();

        auto no = std::make_unique<ASTNode>(
            NodeType::BinaryOp,
            operador.lexeme,
            "",
            operador.line,
            operador.column
        );

        no->addChild(std::move(esquerda));
        no->addChild(std::move(direita));

        esquerda = std::move(no);
    }

    return esquerda;
}

// Reconhece operadores unarios.
//
// Exemplos:
// -5
// +10
std::unique_ptr<ASTNode> Parser::unario() {

    if (eh(TokenType::Minus) ||
        eh(TokenType::Plus)) {

        Token operador = atual();
        aceitar(operador.type);

        auto no = std::make_unique<ASTNode>(
            NodeType::UnaryOp,
            operador.lexeme,
            "",
            operador.line,
            operador.column
        );

        no->addChild(unario());

        return no;
    }

    return primario();
}

// Identifica os valores mais simples.
//
// Exemplos:
// 20
// 1.75
// "Ola"
// idade
// (2 + 3)
std::unique_ptr<ASTNode> Parser::primario() {

    Token token = atual();

    NodeType noTipo;
    std::string dado;

    switch (token.type) {

        case TokenType::Identifier:
            noTipo = NodeType::Identifier;
            break;

        case TokenType::IntegerLiteral:
            noTipo = NodeType::IntegerLiteral;
            dado = "int";
            break;

        case TokenType::FloatLiteral:
            noTipo = NodeType::FloatLiteral;
            dado = "float";
            break;

        case TokenType::StringLiteral:
            noTipo = NodeType::StringLiteral;
            dado = "string";
            break;

        case TokenType::CharLiteral:
            noTipo = NodeType::CharLiteral;
            dado = "char";
            break;

        case TokenType::True:
        case TokenType::False:
            noTipo = NodeType::BoolLiteral;
            dado = "bool";
            break;

        // Expressoes entre parenteses.
        case TokenType::LeftParen: {

            aceitar(TokenType::LeftParen);

            auto dentro = expressao();

            consumir(
                TokenType::RightParen,
                "esperado ')' na expressao"
            );

            return dentro;
        }

        default:
            erro(
                "esperado um valor, variavel "
                "ou '(' na expressao"
            );
    }

    aceitar(token.type);

    return std::make_unique<ASTNode>(
        noTipo,
        token.lexeme,
        dado,
        token.line,
        token.column
    );
}
