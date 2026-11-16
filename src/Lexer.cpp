
#include "Lexer.h"

#include <cctype>
#include <stdexcept>
#include <unordered_map>
#include <utility>

// Construtor: armazena o codigo recebido.
Lexer::Lexer(std::string codigo)
    : codigo(std::move(codigo)) {}

// Retorna o caractere atual.
// Se a leitura terminou, retorna '\0'.
char Lexer::atual() const {
    return posicao < codigo.size()
        ? codigo[posicao]
        : '\0';
}

// Avanca um caractere e atualiza linha e coluna.
char Lexer::avancar() {
    char c = codigo[posicao++];

    if (c == '\n') {
        ++linha;
        coluna = 1;
    } else {
        ++coluna;
    }

    return c;
}

// Reconhece a segunda parte de operadores compostos,
// como ==, <=, >=, !=, && e ||.
bool Lexer::aceitar(char esperado) {
    if (atual() != esperado) {
        return false;
    }

    avancar();
    return true;
}

// Funcao principal da analise lexica.
// Retorna uma lista de tokens reconhecidos.
std::vector<Token> Lexer::analisar() {

    std::vector<Token> tokens;

    // Relaciona palavras reservadas aos respectivos tipos.
    const std::unordered_map<std::string, TokenType> palavras = {
        {"var", TokenType::Var},
        {"int", TokenType::Int},
        {"float", TokenType::Float},
        {"char", TokenType::Char},
        {"bool", TokenType::Bool},
        {"string", TokenType::String},
        {"true", TokenType::True},
        {"false", TokenType::False},
        {"if", TokenType::If},
        {"else", TokenType::Else},
        {"while", TokenType::While},
        {"for", TokenType::For},
        {"func", TokenType::Func},
        {"return", TokenType::Return},
        {"print", TokenType::Print},
        {"input", TokenType::Input}
    };

    // Verifica se um caractere pode fazer parte
    // de um identificador.
    auto ehLetra = [](char c) {
        return std::isalpha(
            static_cast<unsigned char>(c)
        ) || c == '_';
    };

    // Verifica se o caractere representa um digito.
    auto ehDigito = [](char c) {
        return std::isdigit(
            static_cast<unsigned char>(c)
        );
    };

    // Continua lendo ate chegar ao final do codigo.
    while (posicao < codigo.size()) {

        char c = atual();

        // Ignora espacos, tabulacoes e quebras de linha.
        if (std::isspace(static_cast<unsigned char>(c))) {
            avancar();
            continue;
        }

        // Ignora comentarios iniciados com //.
        if (c == '/' &&
            posicao + 1 < codigo.size() &&
            codigo[posicao + 1] == '/') {

            while (posicao < codigo.size() &&
                   atual() != '\n') {
                avancar();
            }

            continue;
        }

        // Ignora comentarios de bloco: /* ... */
        if (c == '/' &&
            posicao + 1 < codigo.size() &&
            codigo[posicao + 1] == '*') {

            int linhaInicial = linha;

            avancar(); // Consome /
            avancar(); // Consome *

            bool fechado = false;

            while (posicao < codigo.size()) {

                if (atual() == '*' &&
                    posicao + 1 < codigo.size() &&
                    codigo[posicao + 1] == '/') {

                    avancar();
                    avancar();
                    fechado = true;
                    break;
                }

                avancar();
            }

            // Um comentario iniciado precisa terminar.
            if (!fechado) {
                throw std::runtime_error(
                    "Comentario nao fechado na linha " +
                    std::to_string(linhaInicial)
                );
            }

            continue;
        }

        // Guarda a posicao inicial do proximo token.
        std::size_t inicio = posicao;
        int linhaToken = linha;
        int colunaToken = coluna;

        // Adiciona o token encontrado na lista.
        auto adicionar = [&](TokenType tipo) {
            tokens.push_back({
                tipo,
                codigo.substr(inicio, posicao - inicio),
                linhaToken,
                colunaToken
            });
        };

        // Consome o primeiro caractere.
        c = avancar();

        // Identifica palavras reservadas e identificadores.
        if (ehLetra(c)) {

            while (ehLetra(atual()) || ehDigito(atual())) {
                avancar();
            }

            std::string palavra =
                codigo.substr(inicio, posicao - inicio);

            auto encontrado = palavras.find(palavra);

            // Se nao for reservada, sera um identificador.
            adicionar(
                encontrado == palavras.end()
                    ? TokenType::Identifier
                    : encontrado->second
            );

            continue;
        }

        // Identifica numeros inteiros e decimais.
        if (ehDigito(c)) {

            while (ehDigito(atual())) {
                avancar();
            }

            bool decimal = false;

            // Um numero decimal utiliza ponto.
            if (atual() == '.' &&
                posicao + 1 < codigo.size() &&
                ehDigito(codigo[posicao + 1])) {

                decimal = true;
                avancar();

                while (ehDigito(atual())) {
                    avancar();
                }
            }

            adicionar(
                decimal
                    ? TokenType::FloatLiteral
                    : TokenType::IntegerLiteral
            );

            continue;
        }

        // Identifica strings e caracteres.
        if (c == '"' || c == '\'') {

            bool escapado = false;
            bool fechado = false;

            while (posicao < codigo.size()) {

                char proximo = avancar();

                // Literais nao podem ocupar varias linhas.
                if (proximo == '\n') {
                    break;
                }

                // Verifica a aspa de fechamento.
                if (proximo == c && !escapado) {
                    fechado = true;
                    break;
                }

                // Controla caracteres de escape.
                if (proximo == '\\' && !escapado) {
                    escapado = true;
                } else {
                    escapado = false;
                }
            }

            if (!fechado) {
                throw std::runtime_error(
                    "Literal nao fechado na linha " +
                    std::to_string(linhaToken)
                );
            }

            // Um char aceita um caractere ou
            // uma sequencia de escape simples.
            if (c == '\'') {

                std::string valor = codigo.substr(
                    inicio + 1,
                    posicao - inicio - 2
                );

                if (!(valor.size() == 1 ||
                    (valor.size() == 2 &&
                     valor[0] == '\\'))) {

                    throw std::runtime_error(
                        "Caractere invalido na linha " +
                        std::to_string(linhaToken)
                    );
                }
            }

            adicionar(
                c == '"'
                    ? TokenType::StringLiteral
                    : TokenType::CharLiteral
            );

            continue;
        }

        // Identifica operadores e simbolos.
        // Operadores compostos utilizam aceitar().
        switch (c) {

            case '+':
                adicionar(TokenType::Plus);
                break;

            case '-':
                adicionar(TokenType::Minus);
                break;

            case '*':
                adicionar(TokenType::Star);
                break;

            case '/':
                adicionar(TokenType::Slash);
                break;

            case '=':
                adicionar(aceitar('=')
                    ? TokenType::EqualEqual
                    : TokenType::Equal);
                break;

            case '!':
                adicionar(aceitar('=')
                    ? TokenType::BangEqual
                    : TokenType::Bang);
                break;

            case '<':
                adicionar(aceitar('=')
                    ? TokenType::LessEqual
                    : TokenType::Less);
                break;

            case '>':
                adicionar(aceitar('=')
                    ? TokenType::GreaterEqual
                    : TokenType::Greater);
                break;

            case '&':
                if (!aceitar('&')) {
                    throw std::runtime_error(
                        "Esperado '&&' na linha " +
                        std::to_string(linhaToken)
                    );
                }
                adicionar(TokenType::AndAnd);
                break;

            case '|':
                if (!aceitar('|')) {
                    throw std::runtime_error(
                        "Esperado '||' na linha " +
                        std::to_string(linhaToken)
                    );
                }
                adicionar(TokenType::OrOr);
                break;

            case '(':
                adicionar(TokenType::LeftParen);
                break;

            case ')':
                adicionar(TokenType::RightParen);
                break;

            case '{':
                adicionar(TokenType::LeftBrace);
                break;

            case '}':
                adicionar(TokenType::RightBrace);
                break;

            case ':':
                adicionar(TokenType::Colon);
                break;

            case ';':
                adicionar(TokenType::Semicolon);
                break;

            case ',':
                adicionar(TokenType::Comma);
                break;

            // Qualquer simbolo desconhecido gera erro.
            default:
                throw std::runtime_error(
                    "Simbolo invalido na linha " +
                    std::to_string(linhaToken) +
                    ", coluna " +
                    std::to_string(colunaToken)
                );
        }
    }

    // Informa ao Parser que a leitura terminou.
    tokens.push_back({
        TokenType::EndOfFile,
        "",
        linha,
        coluna
    });

    return tokens;
}
