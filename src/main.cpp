
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "Lexer.h"
#include "Parser.h"

// Traduz os tipos da AST para nomes legiveis.
// Utilizado apenas para visualizar os testes.
const char* nomeNo(NodeType tipo) {

    switch (tipo) {

        case NodeType::Program: return "Programa";
        case NodeType::Block: return "Bloco";
        case NodeType::If: return "If";
        case NodeType::While: return "While";

        case NodeType::VarDecl: return "Declaracao";
        case NodeType::Assignment: return "Atribuicao";
        case NodeType::Print: return "Print";
        case NodeType::Input: return "Input";

        case NodeType::Identifier: return "Identificador";
        case NodeType::IntegerLiteral: return "Inteiro";
        case NodeType::FloatLiteral: return "Decimal";
        case NodeType::StringLiteral: return "Texto";
        case NodeType::CharLiteral: return "Caractere";
        case NodeType::BoolLiteral: return "Booleano";

        case NodeType::BinaryOp: return "Operacao binaria";
        case NodeType::UnaryOp: return "Operacao unaria";

        default: return "Outro";
    }
}

// Mostra a arvore sintatica no terminal.
//
// A funcao percorre todos os filhos de cada no.
// O nivel determina a indentacao apresentada.
void mostrarAST(const ASTNode& no, int nivel = 0) {

    std::cout
        << std::string(nivel * 2, ' ')
        << nomeNo(no.type);

    if (!no.value.empty()) {
        std::cout << ": " << no.value;
    }

    if (!no.dataType.empty()) {
        std::cout << " [" << no.dataType << "]";
    }

    std::cout << '\n';

    // Visita os filhos do no atual.
    for (const auto& filho : no.children) {
        mostrarAST(*filho, nivel + 1);
    }
}

// Ponto de entrada do compilador MACSLang.
int main(int argc, char* argv[]) {

    // Exige um arquivo informado pelo terminal.
    if (argc != 2) {
        std::cerr
            << "Uso: " << argv[0]
            << " arquivo.macs\n";

        return 1;
    }

    // Abre o arquivo MACSLang.
    std::ifstream arquivo(argv[1]);

    if (!arquivo) {
        std::cerr
            << "Nao foi possivel abrir o arquivo.\n";

        return 1;
    }

    // Carrega todo o codigo-fonte em uma string.
    std::string codigo(
        (std::istreambuf_iterator<char>(arquivo)),
        std::istreambuf_iterator<char>()
    );

    try {

        // FASE 1 - ANALISE LEXICA
        // Transforma o codigo em tokens.
        Lexer lexer(codigo);

        auto tokens = lexer.analisar();

        std::cout
            << "Analise lexica concluida!\n";

        // FASE 2 - ANALISE SINTATICA
        // Verifica a estrutura e cria a AST.
        Parser parser(tokens);

        auto arvore = parser.analisar();

        std::cout
            << "Analise sintatica (parte 2) concluida!\n";

        // Exibe a AST para verificar a estrutura.
        mostrarAST(*arvore);

    } catch (const std::exception& erro) {

        // Interrompe o processo se houver erro.
        std::cerr
            << "Erro de compilacao: "
            << erro.what() << '\n';

        return 1;
    }

    return 0;
}
