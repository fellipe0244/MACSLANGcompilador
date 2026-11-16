
#ifndef LEXER_H
#define LEXER_H

#include <cstddef>
#include <string>
#include <vector>
#include "Token.h"

// Analisador lexico da MACSLang.
// Percorre o codigo-fonte e identifica os tokens.
class Lexer {
public:

    // Recebe o codigo completo que sera analisado.
    explicit Lexer(std::string codigo);

    // Executa a analise lexica.
    // Retorna os tokens na ordem encontrada.
    // Lanca uma excecao quando identifica um erro.
    std::vector<Token> analisar();

private:

    // Conteudo original do programa.
    std::string codigo;

    // Posicao atual da leitura.
    std::size_t posicao = 0;

    // Controle da linha e coluna atual.
    int linha = 1;
    int coluna = 1;

    // Consulta o caractere atual sem consumi-lo.
    char atual() const;

    // Consome o caractere e avanca a posicao.
    char avancar();

    // Verifica se o proximo caractere corresponde
    // ao esperado, consumindo-o quando corresponde.
    bool aceitar(char esperado);
};

#endif
