
#ifndef PARSER_H
#define PARSER_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "AST.h"
#include "Token.h"

// FASE 2 - ANALISE SINTATICA
//
// Recebe os tokens gerados pelo Lexer,
// verifica a estrutura dos comandos e cria a AST.
//
// Nesta etapa reconhecemos:
// - Declaracoes de variaveis e atribuicoes
// - Comandos print e input
// - Blocos de codigo
// - Condicionais if/else
// - Repeticao while
// - Expressoes aritmeticas, logicas e comparacoes
class Parser {
public:

    // Recebe os tokens, incluindo EndOfFile.
    explicit Parser(const std::vector<Token>& tokens);

    // Executa a analise sintatica.
    // Retorna a AST ou gera um erro.
    std::unique_ptr<ASTNode> analisar();

private:

    // Lista de tokens recebida do Lexer.
    const std::vector<Token>& tokens;

    // Posicao atual durante a leitura.
    std::size_t posicao = 0;

    // Funcoes auxiliares de leitura.
    const Token& atual() const;
    bool eh(TokenType tipo) const;
    bool aceitar(TokenType tipo);

    // Exige determinado token.
    Token consumir(
        TokenType tipo,
        const std::string& mensagem
    );

    // Apresenta erros com linha e coluna.
    [[noreturn]]
    void erro(const std::string& mensagem) const;

    // Reconhecimento dos comandos.
    std::unique_ptr<ASTNode> comando();
    std::unique_ptr<ASTNode> declaracao();
    std::unique_ptr<ASTNode> atribuicao();
    std::unique_ptr<ASTNode> impressao();
    std::unique_ptr<ASTNode> entrada();

    // Estruturas de controle.
    std::unique_ptr<ASTNode> bloco();
    std::unique_ptr<ASTNode> condicional();
    std::unique_ptr<ASTNode> repeticaoWhile();

    // Analise de expressoes em ordem
    // crescente de prioridade.
    std::unique_ptr<ASTNode> expressao();
    std::unique_ptr<ASTNode> logicoOu();
    std::unique_ptr<ASTNode> logicoE();
    std::unique_ptr<ASTNode> igualdade();
    std::unique_ptr<ASTNode> comparacao();
    std::unique_ptr<ASTNode> soma();
    std::unique_ptr<ASTNode> multiplicacao();
    std::unique_ptr<ASTNode> unario();
    std::unique_ptr<ASTNode> primario();
};

#endif
