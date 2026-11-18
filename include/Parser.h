
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
// Recebe a lista de tokens produzida pelo Lexer,
// verifica a estrutura do codigo e constroi a AST.
//
// A verificacao de tipos e variaveis declaradas
// sera responsabilidade do analisador semantico.
class Parser {
public:

    // Recebe os tokens, incluindo o EndOfFile.
    explicit Parser(const std::vector<Token>& tokens);

    // Executa a analise e devolve a raiz da AST.
    std::unique_ptr<ASTNode> analisar();

private:

    // Lista original de tokens.
    const std::vector<Token>& tokens;

    // Posicao atual na lista.
    std::size_t posicao = 0;

    // Funcoes auxiliares para ler os tokens.
    const Token& atual() const;
    bool eh(TokenType tipo) const;
    bool aceitar(TokenType tipo);

    Token consumir(
        TokenType tipo,
        const std::string& mensagem
    );

    // Interrompe a analise com erro de sintaxe.
    [[noreturn]]
    void erro(const std::string& mensagem) const;

    // Comandos basicos da MACSLang.
    std::unique_ptr<ASTNode> comando();
    std::unique_ptr<ASTNode> declaracao();

    // O incremento do for nao usa ponto e virgula.
    std::unique_ptr<ASTNode> atribuicao(
        bool comPontoVirgula = true
    );

    std::unique_ptr<ASTNode> impressao();
    std::unique_ptr<ASTNode> entrada();
    std::unique_ptr<ASTNode> bloco();

    // Estruturas condicionais e de repeticao.
    std::unique_ptr<ASTNode> condicional();
    std::unique_ptr<ASTNode> repeticaoWhile();
    std::unique_ptr<ASTNode> repeticaoFor();

    // Funcoes, parametros e retornos.
    std::unique_ptr<ASTNode> funcao();
    std::unique_ptr<ASTNode> retorno();
    std::unique_ptr<ASTNode> chamada();
    std::unique_ptr<ASTNode> chamadaComoComando();

    // Verifica os tipos definidos na linguagem.
    Token tipoDeDado();

    // Expressoes organizadas por prioridade.
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
