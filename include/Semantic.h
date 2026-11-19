
#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <string>
#include <unordered_map>
#include <vector>
#include "AST.h"

// FASE 3 - ANALISE SEMANTICA
//
// Recebe a AST produzida pelo Parser e verifica:
// - Declaracao de variaveis
// - Compatibilidade de tipos
// - Escopos de variaveis
// - Assinaturas e chamadas de funcoes
// - Retornos obrigatorios das funcoes
//
// O analisador nao modifica a AST.
class Semantic {
public:

    // Inicia a analise semantica.
    // Lanca uma excecao se encontrar um erro.
    void analisar(const ASTNode& programa);

private:

    // Armazena nome e tipo das variaveis
    // que pertencem a cada escopo.
    using Escopo =
        std::unordered_map<std::string, std::string>;

    // Pilha de escopos ativos.
    std::vector<Escopo> escopos;

    // Guarda o tipo de retorno e os tipos
    // dos parametros de cada funcao.
    struct Assinatura {
        std::string retorno;
        std::vector<std::string> parametros;
    };

    // Tabela de funcoes declaradas.
    std::unordered_map<std::string, Assinatura> funcoes;

    // Tipo de retorno esperado na funcao atual.
    // Fica vazio fora de uma funcao.
    std::string retornoAtual;

    // Gerenciamento de escopos.
    void abrirEscopo();
    void fecharEscopo();

    // Registra e consulta variaveis.
    void declarar(const ASTNode& no);

    std::string buscarVariavel(
        const ASTNode& no
    ) const;

    // Registra e analisa funcoes.
    void registrarFuncoes(
        const ASTNode& programa
    );

    void analisarFuncao(
        const ASTNode& funcao
    );

    // Analisa os comandos de um bloco.
    void analisarBloco(
        const ASTNode& bloco,
        bool criarEscopo = true
    );

    // Verifica se todos os caminhos possiveis
    // de um bloco terminam em return.
    //
    // Essa funcao corrige a primeira falha
    // encontrada na auditoria.
    static bool garanteRetorno(
        const ASTNode& no
    );

    // Analise dos comandos e expressoes.
    void analisarComando(const ASTNode& no);

    std::string tipoExpressao(
        const ASTNode& no
    );

    // Funcoes auxiliares para tipos.
    static bool numerico(
        const std::string& tipo
    );

    static bool compativel(
        const std::string& destino,
        const std::string& origem
    );

    // Apresenta mensagens de erro.
    [[noreturn]]
    static void erro(
        const ASTNode& no,
        const std::string& mensagem
    );
};

#endif
