
#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

// Define os tipos de nos que podem existir
// na Arvore Sintatica Abstrata (AST).
//
// Cada tipo representa uma estrutura
// da linguagem MACSLang.
enum class NodeType {

    // Estrutura principal do programa
    Program,
    Block,

    // Comandos da linguagem
    VarDecl,
    Assignment,
    Print,
    Input,
    If,
    While,
    For,
    Function,
    Parameter,
    Return,
    ExpressionStatement,

    // Expressoes e valores
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,
    BoolLiteral,
    BinaryOp,
    UnaryOp,
    Call
};

// Representa um no da arvore sintatica.
//
// O Parser cria esses nos durante a analise.
// Depois, a analise semantica e o gerador
// de codigo utilizarao a mesma estrutura.
struct ASTNode {

    // Identifica a categoria do no.
    NodeType type;

    // Armazena nome, valor ou operador.
    std::string value;

    // Armazena o tipo de dado, quando necessario.
    std::string dataType;

    // Localizacao do elemento no codigo-fonte.
    int line;
    int column;

    // Nos filhos que pertencem a este no.
    // unique_ptr gerencia a memoria automaticamente.
    std::vector<std::unique_ptr<ASTNode>> children;

    // Construtor utilizado para criar os nos.
    ASTNode(
        NodeType type,
        std::string value = "",
        std::string dataType = "",
        int line = 0,
        int column = 0
    )
        : type(type),
          value(std::move(value)),
          dataType(std::move(dataType)),
          line(line),
          column(column) {}

    // Adiciona um novo filho a arvore.
    //
    // std::move transfere a posse do no filho
    // para o no atual.
    void addChild(std::unique_ptr<ASTNode> child) {
        children.push_back(std::move(child));
    }
};

#endif
