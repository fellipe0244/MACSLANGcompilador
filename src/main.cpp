
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "Lexer.h"
#include "Parser.h"

// Converte o tipo de um no em um nome legivel.
// Isso facilita visualizar a AST no terminal.
const char* nomeNo(NodeType tipo) {

    switch (tipo) {

        case NodeType::Program:
            return "Programa";

        case NodeType::VarDecl:
            return "Declaracao";

        case NodeType::Assignment:
            return "Atribuicao";

        case NodeType::Print:
            return "Print";

        case NodeType::Input:
            return "Input";

        case NodeType::Identifier:
            return "Identificador";

        case NodeType::IntegerLiteral:
            return "Inteiro";

        case NodeType::FloatLiteral:
            return "Decimal";

        case NodeType::StringLiteral:
            return "Texto";

        case NodeType::CharLiteral:
            return "Caractere";

        case NodeType::BoolLiteral:
            return "Booleano";

        case NodeType::BinaryOp:
            return "Operacao binaria";

        case NodeType::UnaryOp:
            return "Operacao unaria";

        default:
            return "Outro";
    }
}

// Percorre a arvore para mostrar seus nos.
//
// O nivel controla a indentacao.
// Cada filho aparece abaixo do no que o possui.
void mostrarAST(
    const ASTNode& no,
    int nivel = 0
) {
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

    // Mostra os filhos recursivamente.
    for (const auto& filho : no.children) {
        mostrarAST(*filho, nivel + 1);
    }
}

// Ponto de entrada do compilador.
int main(int argc, char* argv[]) {

    // Exige o caminho de um arquivo MACSLang.
    if (argc != 2) {
        std::cerr
            << "Uso: " << argv[0]
            << " arquivo.macs\n";

        return 1;
    }

    // Abre o arquivo informado.
    std::ifstream arquivo(argv[1]);

    if (!arquivo) {
        std::cerr
            << "Nao foi possivel abrir o arquivo.\n";

        return 1;
    }

    // Carrega o conteudo completo do arquivo.
    std::string codigo(
        (std::istreambuf_iterator<char>(arquivo)),
        std::istreambuf_iterator<char>()
    );

    try {

        // FASE 1 - ANALISE LEXICA
        // Transforma o codigo-fonte em tokens.
        Lexer lexer(codigo);

        auto tokens = lexer.analisar();

        std::cout
            << "Analise lexica concluida!\n";

        // FASE 2 - ANALISE SINTATICA
        // Recebe os tokens e constroi a AST.
        Parser parser(tokens);

        auto arvore = parser.analisar();

        std::cout
            << "Analise sintatica (parte 1) concluida!\n";

        // Exibe a arvore para testar o Parser.
        mostrarAST(*arvore);

    } catch (const std::exception& erro) {

        // Interrompe a compilacao quando ha erro.
        std::cerr
            << "Erro de compilacao: "
            << erro.what() << '\n';

        return 1;
    }

    return 0;
}
