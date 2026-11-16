
#ifndef PARSER_H
#define PARSER_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "AST.h"
#include "Token.h"

// Analisa a ordem dos tokens e cria a arvore sintatica.
//
// Nesta primeira parte reconhecemos:
// - Declaracao de variaveis
// - Atribuicao de valores
// - Comandos print e input
// - Expressoes aritmeticas
class Parser {
public:

    // Recebe a lista de tokens produzida pelo Lexer.
    explicit Parser(const std::vector<Token>& tokens);

    // Inicia a analise sintatica.
    // Retorna a raiz da AST ou gera um erro.
    std::unique_ptr<ASTNode> analisar();

private:

    // Referencia para os tokens recebidos.
    const std::vector<Token>& tokens;

    // Posicao atual na lista de tokens.
    std::size_t posicao = 0;

    // Funcoes auxiliares de leitura.
    const Token& atual() const;
    bool eh(TokenType tipo) const;
    bool aceitar(TokenType tipo);

    // Exige um token especifico.
    Token consumir(
        TokenType tipo,
        const std::string& mensagem
    );

    // Mostra erros de sintaxe com linha e coluna.
    [[noreturn]]
    void erro(const std::string& mensagem) const;

    // Reconhecimento dos comandos.
    std::unique_ptr<ASTNode> comando();
    std::unique_ptr<ASTNode> declaracao();
    std::unique_ptr<ASTNode> atribuicao();
    std::unique_ptr<ASTNode> impressao();
    std::unique_ptr<ASTNode> entrada();

    // Reconhecimento de expressoes matematicas.
    std::unique_ptr<ASTNode> expressao();
    std::unique_ptr<ASTNode> multiplicacao();
    std::unique_ptr<ASTNode> unario();
    std::unique_ptr<ASTNode> primario();
};

#endif
