
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "Lexer.h"
#include "Parser.h"
#include "Semantic.h"

// Traduz os tipos da AST para nomes legiveis.
const char* nomeNo(NodeType tipo) {

    switch (tipo) {

        case NodeType::Program: return "Programa";
        case NodeType::Block: return "Bloco";
        case NodeType::If: return "If";
        case NodeType::While: return "While";
        case NodeType::For: return "For";

        case NodeType::Function: return "Funcao";
        case NodeType::Parameter: return "Parametro";
        case NodeType::Return: return "Return";
        case NodeType::Call: return "Chamada";

        case NodeType::ExpressionStatement:
            return "Comando de chamada";

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

// Exibe os nos da AST com indentacao.
// Cada nivel representa um nivel da arvore.
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

    // Percorre recursivamente os filhos.
    for (const auto& filho : no.children) {
        mostrarAST(*filho, nivel + 1);
    }
}

// Ponto de entrada do compilador MACSLang.
int main(int argc, char* argv[]) {

    // Exige um arquivo .macs como argumento.
    if (argc != 2) {
        std::cerr
            << "Uso: " << argv[0]
            << " arquivo.macs\n";

        return 1;
    }

    // Abre o arquivo de entrada.
    std::ifstream arquivo(argv[1]);

    if (!arquivo) {
        std::cerr
            << "Nao foi possivel abrir o arquivo.\n";

        return 1;
    }

    // Carrega o codigo-fonte completo.
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
        // Verifica a estrutura e cria a AST.
        Parser parser(tokens);

        auto arvore = parser.analisar();

        std::cout
            << "Analise sintatica (parte 3) concluida!\n";

        // Mostra a arvore para facilitar os testes.
        mostrarAST(*arvore);

        // FASE 3 - ANALISE SEMANTICA
        // Verifica tipos, variaveis, escopos e funcoes.
        Semantic semantico;

        semantico.analisar(*arvore);

        std::cout
            << "Analise semantica concluida!\n";

    } catch (const std::exception& erro) {

        // Interrompe o compilador caso encontre erro.
        std::cerr
            << "Erro de compilacao: "
            << erro.what() << '\n';

        return 1;
    }

    return 0;
}
