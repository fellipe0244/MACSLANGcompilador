
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "Lexer.h"

// Ponto de entrada do compilador MACSLang.
// Recebe o nome do arquivo a ser analisado.
int main(int argc, char* argv[]) {

    // Verifica se o usuario informou o arquivo.
    if (argc != 2) {
        std::cerr
            << "Uso: " << argv[0]
            << " arquivo.macs\n";

        return 1;
    }

    // Abre o arquivo informado no terminal.
    std::ifstream arquivo(argv[1]);

    // Verifica se foi possivel abrir o arquivo.
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

        // ETAPA 1 - Analise lexica.
        // Cria o Lexer utilizando o codigo recebido.
        Lexer lexer(codigo);

        // Executa a analise e recebe os tokens.
        std::vector<Token> tokens = lexer.analisar();

        // Mostra os tokens para verificar o resultado.
        // Futuramente, serao enviados ao Parser.
        for (const Token& token : tokens) {

            // O token EndOfFile indica o fim.
            if (token.type == TokenType::EndOfFile) {
                break;
            }

            // Mostra localizacao, texto e tipo do token.
            std::cout
                << "[" << token.line
                << ":" << token.column << "] "
                << token.lexeme
                << " (tipo "
                << static_cast<int>(token.type)
                << ")\n";
        }

        std::cout
            << "Analise lexica concluida!\n";

    } catch (const std::exception& erro) {

        // Captura erros encontrados pelo analisador.
        std::cerr
            << "Erro lexico: "
            << erro.what() << "\n";

        return 1;
    }

    // Retorno 0 indica que tudo ocorreu corretamente.
    return 0;
}
