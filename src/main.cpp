
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "Lexer.h"
#include "Parser.h"

// Converte os tipos de nos da AST em nomes legiveis.
// Isso facilita visualizar o resultado no terminal.
const char* nomeNo(NodeType tipo) {
    switch (tipo) {

        // Estrutura do programa
        case NodeType::Program: return "Programa";
        case NodeType::Block: return "Bloco";

        // Estruturas de controle
        case NodeType::If: return "If";
        case NodeType::While: return "While";
        case NodeType::For: return "For";

        // Funcoes
        case NodeType::Function: return "Funcao";
        case NodeType::Parameter: return "Parametro";
        case NodeType::Return: return "Return";
        case NodeType::Call: return "Chamada";
        case NodeType::ExpressionStatement:
            return "Comando de chamada";

        // Comandos comuns
        case NodeType::VarDecl: return "Declaracao";
        case NodeType::Assignment: return "Atribuicao";
        case NodeType::Print: return "Print";
        case NodeType::Input: return "Input";

        // Valores e identificadores
        case NodeType::Identifier: return "Identificador";
        case NodeType::IntegerLiteral: return "Inteiro";
        case NodeType::FloatLiteral: return "Decimal";
        case NodeType::StringLiteral: return "Texto";
        case NodeType::CharLiteral: return "Caractere";
        case NodeType::BoolLiteral: return "Booleano";

        // Operacoes
        case NodeType::BinaryOp: return "Operacao binaria";
        case NodeType::UnaryOp: return "Operacao unaria";

        default: return "Outro";
    }
}

// Percorre a AST e mostra sua estrutura.
//
// Quanto maior o nivel, maior sera a indentacao.
// Isso permite visualizar a relacao entre os nos.
void mostrarAST(const ASTNode& no, int nivel = 0) {

    std::cout
        << std::string(nivel * 2, ' ')
        << nomeNo(no.type);

    // Mostra o nome, valor ou operador.
    if (!no.value.empty()) {
        std::cout << ": " << no.value;
    }

    // Mostra o tipo de dado, quando existir.
    if (!no.dataType.empty()) {
        std::cout << " [" << no.dataType << "]";
    }

    std::cout << '\n';

    // Percorre recursivamente os filhos do no.
    for (const auto& filho : no.children) {
        mostrarAST(*filho, nivel + 1);
    }
}

// Funcao principal do compilador MACSLang.
//
// Recebe o caminho do arquivo .macs informado
// pelo usuario no terminal.
int main(int argc, char* argv[]) {

    // Verifica se foi informado um arquivo.
    if (argc != 2) {
        std::cerr
            << "Uso: " << argv[0]
            << " arquivo.macs\n";

        return 1;
    }

    // Abre o arquivo de codigo-fonte.
    std::ifstream arquivo(argv[1]);

    if (!arquivo) {
        std::cerr
            << "Nao foi possivel abrir o arquivo.\n";

        return 1;
    }

    // Le todo o conteudo do arquivo.
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
        // Verifica a estrutura dos tokens e monta a AST.
        Parser parser(tokens);
        auto arvore = parser.analisar();

        std::cout
            << "Analise sintatica (parte 3) concluida!\n";

        // Apresenta a estrutura da AST no terminal.
        mostrarAST(*arvore);

    } catch (const std::exception& erro) {

        // Interrompe a compilacao se encontrar erro.
        std::cerr
            << "Erro de compilacao: "
            << erro.what() << '\n';

        return 1;
    }

    // Codigo 0 significa encerramento sem erros.
    return 0;
}
