
#include "Parser.h"

#include <stdexcept>
#include <utility>

// Cria um no com dois operandos.
// filho[0] = esquerda; filho[1] = direita.
static std::unique_ptr<ASTNode> criarBinario(
    const Token& operador,
    std::unique_ptr<ASTNode> esquerda,
    std::unique_ptr<ASTNode> direita
) {
    auto no = std::make_unique<ASTNode>(
        NodeType::BinaryOp,
        operador.lexeme,
        "",
        operador.line,
        operador.column
    );

    no->addChild(std::move(esquerda));
    no->addChild(std::move(direita));

    return no;
}

// Recebe a lista de tokens do Lexer.
Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens) {

    if (tokens.empty() ||
        tokens.back().type != TokenType::EndOfFile) {
        throw std::runtime_error(
            "Lista de tokens invalida: falta EndOfFile"
        );
    }
}

// Consulta o token atual sem avancar.
const Token& Parser::atual() const {
    return tokens[posicao];
}

// Verifica se o token atual corresponde ao tipo esperado.
bool Parser::eh(TokenType tipo) const {
    return atual().type == tipo;
}

// Consome o token caso seja do tipo esperado.
bool Parser::aceitar(TokenType tipo) {
    if (!eh(tipo)) {
        return false;
    }

    if (!eh(TokenType::EndOfFile)) {
        ++posicao;
    }

    return true;
}

// Exige um token e apresenta erro caso nao exista.
Token Parser::consumir(
    TokenType tipo,
    const std::string& mensagem
) {
    if (!eh(tipo)) {
        erro(mensagem);
    }

    Token token = atual();
    aceitar(tipo);
    return token;
}

// Apresenta o erro com a linha e a coluna.
[[noreturn]]
void Parser::erro(const std::string& mensagem) const {
    throw std::runtime_error(
        "Linha " + std::to_string(atual().line) +
        ", coluna " + std::to_string(atual().column) +
        ": " + mensagem
    );
}

// Cria a raiz da AST e percorre o programa.
//
// Funcoes sao reconhecidas no nivel principal.
// Os demais elementos sao tratados como comandos.
std::unique_ptr<ASTNode> Parser::analisar() {
    auto raiz = std::make_unique<ASTNode>(
        NodeType::Program
    );

    while (!eh(TokenType::EndOfFile)) {
        if (eh(TokenType::Func)) {
            raiz->addChild(funcao());
        } else {
            raiz->addChild(comando());
        }
    }

    return raiz;
}

// Identifica o comando pela categoria do token atual.
std::unique_ptr<ASTNode> Parser::comando() {
    if (eh(TokenType::Var)) return declaracao();
    if (eh(TokenType::Print)) return impressao();
    if (eh(TokenType::Input)) return entrada();
    if (eh(TokenType::LeftBrace)) return bloco();
    if (eh(TokenType::If)) return condicional();
    if (eh(TokenType::While)) return repeticaoWhile();
    if (eh(TokenType::For)) return repeticaoFor();
    if (eh(TokenType::Return)) return retorno();

    if (eh(TokenType::Identifier)) {

        // Identificador seguido de '(' indica
        // chamada de funcao, como soma(2, 3).
        if (posicao + 1 < tokens.size() &&
            tokens[posicao + 1].type ==
                TokenType::LeftParen) {

            return chamadaComoComando();
        }

        // Caso contrario, reconhece uma atribuicao.
        return atribuicao();
    }

    erro("comando inesperado ou ainda nao implementado");
}

// Reconhece:
// var idade: int = 20;
//
// O valor inicial e opcional.
// filho[0] guarda a expressao inicial, se existir.
std::unique_ptr<ASTNode> Parser::declaracao() {
    Token inicio = consumir(
        TokenType::Var, "esperado 'var'"
    );

    Token nome = consumir(
        TokenType::Identifier,
        "esperado nome da variavel"
    );

    consumir(
        TokenType::Colon,
        "esperado ':' depois do nome"
    );

    Token tipo = tipoDeDado();

    auto no = std::make_unique<ASTNode>(
        NodeType::VarDecl,
        nome.lexeme,
        tipo.lexeme,
        inicio.line,
        inicio.column
    );

    if (aceitar(TokenType::Equal)) {
        no->addChild(expressao());
    }

    consumir(
        TokenType::Semicolon,
        "esperado ';' depois da declaracao"
    );

    return no;
}

// Reconhece atribuicoes:
// idade = idade + 1;
//
// O parametro indica se deve consumir ';'.
// No incremento de um for, nao existe esse ';'.
std::unique_ptr<ASTNode> Parser::atribuicao(
    bool comPontoVirgula
) {
    Token nome = consumir(
        TokenType::Identifier,
        "esperado nome da variavel"
    );

    consumir(
        TokenType::Equal,
        "esperado '=' na atribuicao"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::Assignment,
        nome.lexeme,
        "",
        nome.line,
        nome.column
    );

    no->addChild(expressao());

    if (comPontoVirgula) {
        consumir(
            TokenType::Semicolon,
            "esperado ';' depois da atribuicao"
        );
    }

    return no;
}

// Reconhece o comando:
// print(expressao);
std::unique_ptr<ASTNode> Parser::impressao() {
    Token inicio = consumir(
        TokenType::Print, "esperado 'print'"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' depois de print"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::Print, "", "",
        inicio.line, inicio.column
    );

    no->addChild(expressao());

    consumir(
        TokenType::RightParen,
        "esperado ')' depois da expressao"
    );

    consumir(
        TokenType::Semicolon,
        "esperado ';' depois de print"
    );

    return no;
}

// Reconhece entrada de dados:
// input(idade);
//
// O nome da variavel fica no campo value.
std::unique_ptr<ASTNode> Parser::entrada() {
    Token inicio = consumir(
        TokenType::Input, "esperado 'input'"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' depois de input"
    );

    Token nome = consumir(
        TokenType::Identifier,
        "esperado nome em input"
    );

    consumir(
        TokenType::RightParen,
        "esperado ')' depois do nome"
    );

    consumir(
        TokenType::Semicolon,
        "esperado ';' depois de input"
    );

    return std::make_unique<ASTNode>(
        NodeType::Input,
        nome.lexeme,
        "",
        inicio.line,
        inicio.column
    );
}

// Reconhece blocos delimitados por { }.
//
// Todos os comandos do bloco sao adicionados
// como filhos do no Block.
std::unique_ptr<ASTNode> Parser::bloco() {
    Token inicio = consumir(
        TokenType::LeftBrace,
        "esperado '{' para abrir bloco"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::Block, "", "",
        inicio.line, inicio.column
    );

    while (!eh(TokenType::RightBrace) &&
           !eh(TokenType::EndOfFile)) {
        no->addChild(comando());
    }

    consumir(
        TokenType::RightBrace,
        "esperado '}' para fechar bloco"
    );

    return no;
}

// Reconhece:
// if (condicao) { ... } else { ... }
//
// filho[0] = condicao
// filho[1] = bloco do if
// filho[2] = bloco else, quando existir
std::unique_ptr<ASTNode> Parser::condicional() {
    Token inicio = consumir(
        TokenType::If, "esperado 'if'"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' depois de if"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::If, "", "",
        inicio.line, inicio.column
    );

    no->addChild(expressao());

    consumir(
        TokenType::RightParen,
        "esperado ')' depois da condicao"
    );

    no->addChild(bloco());

    if (aceitar(TokenType::Else)) {
        no->addChild(bloco());
    }

    return no;
}

// Reconhece:
// while (condicao) { ... }
//
// filho[0] = condicao
// filho[1] = bloco de repeticao
std::unique_ptr<ASTNode> Parser::repeticaoWhile() {
    Token inicio = consumir(
        TokenType::While, "esperado 'while'"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' depois de while"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::While, "", "",
        inicio.line, inicio.column
    );

    no->addChild(expressao());

    consumir(
        TokenType::RightParen,
        "esperado ')' depois da condicao"
    );

    no->addChild(bloco());

    return no;
}

// Reconhece os cinco tipos definidos na MACSLang.
// Usado em variaveis, parametros e funcoes.
Token Parser::tipoDeDado() {
    if (!eh(TokenType::Int) &&
        !eh(TokenType::Float) &&
        !eh(TokenType::Char) &&
        !eh(TokenType::Bool) &&
        !eh(TokenType::String)) {

        erro(
            "esperado tipo: int, float, "
            "char, bool ou string"
        );
    }

    Token tipo = atual();
    aceitar(tipo.type);

    return tipo;
}

// Reconhece declaracoes de funcoes.
//
// Exemplo:
// func soma(a: int, b: int): int {
//     return a + b;
// }
//
// Os parametros sao filhos da Function.
// O ultimo filho e sempre o bloco da funcao.
std::unique_ptr<ASTNode> Parser::funcao() {
    Token inicio = consumir(
        TokenType::Func, "esperado 'func'"
    );

    Token nome = consumir(
        TokenType::Identifier,
        "esperado nome da funcao"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' apos nome da funcao"
    );

    // Armazena os parametros ate conhecer o retorno.
    std::vector<std::unique_ptr<ASTNode>> parametros;

    if (!eh(TokenType::RightParen)) {
        do {
            Token parametro = consumir(
                TokenType::Identifier,
                "esperado nome do parametro"
            );

            consumir(
                TokenType::Colon,
                "esperado ':' depois do parametro"
            );

            Token tipo = tipoDeDado();

            parametros.push_back(
                std::make_unique<ASTNode>(
                    NodeType::Parameter,
                    parametro.lexeme,
                    tipo.lexeme,
                    parametro.line,
                    parametro.column
                )
            );

        } while (aceitar(TokenType::Comma));
    }

    consumir(
        TokenType::RightParen,
        "esperado ')' nos parametros"
    );

    consumir(
        TokenType::Colon,
        "esperado ':' antes do tipo de retorno"
    );

    Token retornoTipo = tipoDeDado();

    auto no = std::make_unique<ASTNode>(
        NodeType::Function,
        nome.lexeme,
        retornoTipo.lexeme,
        inicio.line,
        inicio.column
    );

    for (auto& parametro : parametros) {
        no->addChild(std::move(parametro));
    }

    // Ultimo filho = corpo da funcao.
    no->addChild(bloco());

    return no;
}

// Reconhece:
// return expressao;
//
// A expressao retornada fica no primeiro filho.
std::unique_ptr<ASTNode> Parser::retorno() {
    Token inicio = consumir(
        TokenType::Return, "esperado 'return'"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::Return, "", "",
        inicio.line, inicio.column
    );

    no->addChild(expressao());

    consumir(
        TokenType::Semicolon,
        "esperado ';' depois de return"
    );

    return no;
}

// Reconhece o for utilizado no exemplo do professor.
//
// for (var i: int = 1; i <= n; i = i + 1) {
//     ...
// }
//
// filho[0] = inicializacao
// filho[1] = condicao
// filho[2] = incremento
// filho[3] = bloco
std::unique_ptr<ASTNode> Parser::repeticaoFor() {
    Token inicio = consumir(
        TokenType::For, "esperado 'for'"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' depois de for"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::For, "", "",
        inicio.line, inicio.column
    );

    // A inicializacao pode declarar ou atribuir.
    // Ambas consomem o primeiro ';'.
    if (eh(TokenType::Var)) {
        no->addChild(declaracao());
    } else {
        no->addChild(atribuicao());
    }

    // Condicao do laco.
    no->addChild(expressao());

    consumir(
        TokenType::Semicolon,
        "esperado ';' na condicao do for"
    );

    // Incremento nao possui ';' no final.
    no->addChild(atribuicao(false));

    consumir(
        TokenType::RightParen,
        "esperado ')' no for"
    );

    // Corpo do laco.
    no->addChild(bloco());

    return no;
}

// Reconhece chamadas como:
// soma(2, 3)
// fatorial(numero)
//
// Cada argumento se torna um filho do no Call.
std::unique_ptr<ASTNode> Parser::chamada() {
    Token nome = consumir(
        TokenType::Identifier,
        "esperado nome da funcao"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' na chamada da funcao"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::Call,
        nome.lexeme,
        "",
        nome.line,
        nome.column
    );

    // Permite zero ou mais argumentos.
    if (!eh(TokenType::RightParen)) {
        do {
            no->addChild(expressao());
        } while (aceitar(TokenType::Comma));
    }

    consumir(
        TokenType::RightParen,
        "esperado ')' na chamada da funcao"
    );

    return no;
}

// Reconhece uma chamada isolada como comando:
// soma(2, 3);
//
// A chamada fica dentro de ExpressionStatement.
std::unique_ptr<ASTNode> Parser::chamadaComoComando() {
    auto no = std::make_unique<ASTNode>(
        NodeType::ExpressionStatement
    );

    no->addChild(chamada());

    consumir(
        TokenType::Semicolon,
        "esperado ';' apos chamada de funcao"
    );

    return no;
}

// Ponto de entrada da analise de expressoes.
// Inicia pelo operador de menor prioridade.
std::unique_ptr<ASTNode> Parser::expressao() {
    return logicoOu();
}

// Operador logico OU: ||
std::unique_ptr<ASTNode> Parser::logicoOu() {
    auto esquerda = logicoE();

    while (eh(TokenType::OrOr)) {
        Token op = atual();
        aceitar(op.type);

        esquerda = criarBinario(
            op, std::move(esquerda), logicoE()
        );
    }

    return esquerda;
}

// Operador logico E: &&
std::unique_ptr<ASTNode> Parser::logicoE() {
    auto esquerda = igualdade();

    while (eh(TokenType::AndAnd)) {
        Token op = atual();
        aceitar(op.type);

        esquerda = criarBinario(
            op, std::move(esquerda), igualdade()
        );
    }

    return esquerda;
}

// Comparacoes de igualdade: == e !=
std::unique_ptr<ASTNode> Parser::igualdade() {
    auto esquerda = comparacao();

    while (eh(TokenType::EqualEqual) ||
           eh(TokenType::BangEqual)) {

        Token op = atual();
        aceitar(op.type);

        esquerda = criarBinario(
            op, std::move(esquerda), comparacao()
        );
    }

    return esquerda;
}

// Comparacoes: <, <=, > e >=
std::unique_ptr<ASTNode> Parser::comparacao() {
    auto esquerda = soma();

    while (eh(TokenType::Less) ||
           eh(TokenType::LessEqual) ||
           eh(TokenType::Greater) ||
           eh(TokenType::GreaterEqual)) {

        Token op = atual();
        aceitar(op.type);

        esquerda = criarBinario(
            op, std::move(esquerda), soma()
        );
    }

    return esquerda;
}

// Soma e subtracao.
// Prioridade menor que multiplicacao e divisao.
std::unique_ptr<ASTNode> Parser::soma() {
    auto esquerda = multiplicacao();

    while (eh(TokenType::Plus) ||
           eh(TokenType::Minus)) {

        Token op = atual();
        aceitar(op.type);

        esquerda = criarBinario(
            op, std::move(esquerda), multiplicacao()
        );
    }

    return esquerda;
}

// Multiplicacao e divisao.
// Sao processadas antes de + e -.
std::unique_ptr<ASTNode> Parser::multiplicacao() {
    auto esquerda = unario();

    while (eh(TokenType::Star) ||
           eh(TokenType::Slash)) {

        Token op = atual();
        aceitar(op.type);

        esquerda = criarBinario(
            op, std::move(esquerda), unario()
        );
    }

    return esquerda;
}

// Operadores unarios:
// -5
// +10
// !ativo
std::unique_ptr<ASTNode> Parser::unario() {
    if (eh(TokenType::Plus) ||
        eh(TokenType::Minus) ||
        eh(TokenType::Bang)) {

        Token op = atual();
        aceitar(op.type);

        auto no = std::make_unique<ASTNode>(
            NodeType::UnaryOp,
            op.lexeme,
            "",
            op.line,
            op.column
        );

        no->addChild(unario());

        return no;
    }

    return primario();
}

// Reconhece os elementos mais simples de uma expressao.
//
// Exemplos:
// 20
// 1.75
// "Ola"
// true
// idade
// soma(2, 3)
// (2 + 3)
std::unique_ptr<ASTNode> Parser::primario() {
    Token token = atual();

    NodeType tipo;
    std::string tipoDado;

    switch (token.type) {

        case TokenType::Identifier:

            // Identificador seguido de '(' indica chamada.
            if (posicao + 1 < tokens.size() &&
                tokens[posicao + 1].type ==
                    TokenType::LeftParen) {

                return chamada();
            }

            tipo = NodeType::Identifier;
            break;

        case TokenType::IntegerLiteral:
            tipo = NodeType::IntegerLiteral;
            tipoDado = "int";
            break;

        case TokenType::FloatLiteral:
            tipo = NodeType::FloatLiteral;
            tipoDado = "float";
            break;

        case TokenType::StringLiteral:
            tipo = NodeType::StringLiteral;
            tipoDado = "string";
            break;

        case TokenType::CharLiteral:
            tipo = NodeType::CharLiteral;
            tipoDado = "char";
            break;

        case TokenType::True:
        case TokenType::False:
            tipo = NodeType::BoolLiteral;
            tipoDado = "bool";
            break;

        // Expressoes protegidas por parenteses.
        case TokenType::LeftParen: {
            aceitar(TokenType::LeftParen);

            auto dentro = expressao();

            consumir(
                TokenType::RightParen,
                "esperado ')' na expressao"
            );

            return dentro;
        }

        default:
            erro(
                "esperado valor, variavel ou "
                "'(' na expressao"
            );
    }

    aceitar(token.type);

    return std::make_unique<ASTNode>(
        tipo,
        token.lexeme,
        tipoDado,
        token.line,
        token.column
    );
}
