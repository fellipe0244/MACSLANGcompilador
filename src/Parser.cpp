
#include "Parser.h"

#include <stdexcept>
#include <utility>

// Cria um no de operacao com dois operandos.
//
// Exemplo: idade + 1
//
// Primeiro filho: operando esquerdo (idade)
// Segundo filho: operando direito (1)
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

// Construtor: recebe a lista de tokens do Lexer.
//
// O ultimo token deve indicar o fim do arquivo.
Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens) {

    if (tokens.empty() ||
        tokens.back().type != TokenType::EndOfFile) {

        throw std::runtime_error(
            "Lista de tokens invalida: falta EndOfFile"
        );
    }
}

// Retorna o token atual sem avancar.
const Token& Parser::atual() const {
    return tokens[posicao];
}

// Verifica se o token possui o tipo informado.
bool Parser::eh(TokenType tipo) const {
    return atual().type == tipo;
}

// Consome o token quando corresponde ao esperado.
bool Parser::aceitar(TokenType tipo) {

    if (!eh(tipo)) {
        return false;
    }

    if (!eh(TokenType::EndOfFile)) {
        ++posicao;
    }

    return true;
}

// Exige um token especifico.
//
// Se o token nao corresponder ao esperado,
// interrompe a analise com erro.
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

// Exibe erros sintaticos com linha e coluna.
[[noreturn]]
void Parser::erro(const std::string& mensagem) const {

    throw std::runtime_error(
        "Linha " + std::to_string(atual().line) +
        ", coluna " + std::to_string(atual().column) +
        ": " + mensagem
    );
}

// Inicia o analisador sintatico.
//
// A raiz Program armazena todos os comandos
// encontrados no codigo-fonte.
std::unique_ptr<ASTNode> Parser::analisar() {

    auto raiz = std::make_unique<ASTNode>(
        NodeType::Program
    );

    while (!eh(TokenType::EndOfFile)) {
        raiz->addChild(comando());
    }

    return raiz;
}

// Identifica o comando que deve ser processado.
//
// Cada tipo de comando possui uma funcao
// especifica para reconhecer sua estrutura.
std::unique_ptr<ASTNode> Parser::comando() {

    if (eh(TokenType::Var)) return declaracao();

    if (eh(TokenType::Print)) return impressao();

    if (eh(TokenType::Input)) return entrada();

    if (eh(TokenType::Identifier)) return atribuicao();

    if (eh(TokenType::LeftBrace)) return bloco();

    if (eh(TokenType::If)) return condicional();

    if (eh(TokenType::While)) return repeticaoWhile();

    erro("comando inesperado ou ainda nao implementado");
}

// Reconhece declaracoes de variaveis.
//
// Exemplo: var idade: int = 20;
//
// O valor inicial e opcional.
std::unique_ptr<ASTNode> Parser::declaracao() {

    Token inicio = consumir(
        TokenType::Var,
        "esperado 'var'"
    );

    Token nome = consumir(
        TokenType::Identifier,
        "esperado nome da variavel"
    );

    consumir(
        TokenType::Colon,
        "esperado ':' depois do nome"
    );

    // Verifica se existe um tipo valido.
    if (!eh(TokenType::Int) &&
        !eh(TokenType::Float) &&
        !eh(TokenType::Char) &&
        !eh(TokenType::Bool) &&
        !eh(TokenType::String)) {

        erro(
            "esperado um tipo: int, float, "
            "char, bool ou string"
        );
    }

    Token tipo = atual();
    aceitar(tipo.type);

    auto no = std::make_unique<ASTNode>(
        NodeType::VarDecl,
        nome.lexeme,
        tipo.lexeme,
        inicio.line,
        inicio.column
    );

    // O valor inicial fica no primeiro filho.
    if (aceitar(TokenType::Equal)) {
        no->addChild(expressao());
    }

    consumir(
        TokenType::Semicolon,
        "esperado ';' depois da declaracao"
    );

    return no;
}

// Reconhece atribuicoes.
//
// Exemplo: idade = idade + 1;
std::unique_ptr<ASTNode> Parser::atribuicao() {

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

    // A expressao atribuida fica no primeiro filho.
    no->addChild(expressao());

    consumir(
        TokenType::Semicolon,
        "esperado ';' depois da atribuicao"
    );

    return no;
}

// Reconhece o comando de saida.
//
// Exemplo: print(idade);
std::unique_ptr<ASTNode> Parser::impressao() {

    Token inicio = consumir(
        TokenType::Print,
        "esperado 'print'"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' depois de print"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::Print,
        "",
        "",
        inicio.line,
        inicio.column
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

// Reconhece o comando de entrada.
//
// Exemplo: input(idade);
std::unique_ptr<ASTNode> Parser::entrada() {

    Token inicio = consumir(
        TokenType::Input,
        "esperado 'input'"
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

// Reconhece blocos delimitados por chaves.
//
// Exemplo:
// {
//     var x: int = 10;
//     print(x);
// }
//
// Cada comando vira um filho do no Block.
std::unique_ptr<ASTNode> Parser::bloco() {

    Token inicio = consumir(
        TokenType::LeftBrace,
        "esperado '{' para abrir bloco"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::Block,
        "",
        "",
        inicio.line,
        inicio.column
    );

    // Processa comandos ate encontrar a chave final.
    while (!eh(TokenType::RightBrace) &&
           !eh(TokenType::EndOfFile)) {

        no->addChild(comando());
    }

    // Um bloco aberto precisa ser fechado.
    consumir(
        TokenType::RightBrace,
        "esperado '}' para fechar bloco"
    );

    return no;
}

// Reconhece condicionais if/else.
//
// Exemplo:
// if (idade >= 18) {
//     print("Maior");
// } else {
//     print("Menor");
// }
//
// AST:
// filho[0] = condicao
// filho[1] = bloco do if
// filho[2] = bloco do else (opcional)
std::unique_ptr<ASTNode> Parser::condicional() {

    Token inicio = consumir(
        TokenType::If,
        "esperado 'if'"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' depois de if"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::If,
        "",
        "",
        inicio.line,
        inicio.column
    );

    // Analisa a expressao da condicao.
    no->addChild(expressao());

    consumir(
        TokenType::RightParen,
        "esperado ')' depois da condicao"
    );

    // O corpo do if precisa ter chaves.
    no->addChild(bloco());

    // O else e opcional.
    if (aceitar(TokenType::Else)) {
        no->addChild(bloco());
    }

    return no;
}

// Reconhece repeticoes while.
//
// Exemplo:
// while (contador < 5) {
//     contador = contador + 1;
// }
//
// AST:
// filho[0] = condicao
// filho[1] = bloco de repeticao
std::unique_ptr<ASTNode> Parser::repeticaoWhile() {

    Token inicio = consumir(
        TokenType::While,
        "esperado 'while'"
    );

    consumir(
        TokenType::LeftParen,
        "esperado '(' depois de while"
    );

    auto no = std::make_unique<ASTNode>(
        NodeType::While,
        "",
        "",
        inicio.line,
        inicio.column
    );

    // Condicao que controla a repeticao.
    no->addChild(expressao());

    consumir(
        TokenType::RightParen,
        "esperado ')' depois da condicao"
    );

    // Comandos executados durante a repeticao.
    no->addChild(bloco());

    return no;
}

// Inicia a analise de expressoes.
//
// Comecamos pelo operador de menor prioridade.
std::unique_ptr<ASTNode> Parser::expressao() {
    return logicoOu();
}

// Reconhece o operador logico OU.
//
// Exemplo: ativo || autorizado
std::unique_ptr<ASTNode> Parser::logicoOu() {

    auto esquerda = logicoE();

    while (eh(TokenType::OrOr)) {

        Token operador = atual();
        aceitar(operador.type);

        esquerda = criarBinario(
            operador,
            std::move(esquerda),
            logicoE()
        );
    }

    return esquerda;
}

// Reconhece o operador logico E.
//
// Exemplo: idade >= 18 && ativo
std::unique_ptr<ASTNode> Parser::logicoE() {

    auto esquerda = igualdade();

    while (eh(TokenType::AndAnd)) {

        Token operador = atual();
        aceitar(operador.type);

        esquerda = criarBinario(
            operador,
            std::move(esquerda),
            igualdade()
        );
    }

    return esquerda;
}

// Reconhece igualdade e diferenca.
//
// Operadores: == e !=
std::unique_ptr<ASTNode> Parser::igualdade() {

    auto esquerda = comparacao();

    while (eh(TokenType::EqualEqual) ||
           eh(TokenType::BangEqual)) {

        Token operador = atual();
        aceitar(operador.type);

        esquerda = criarBinario(
            operador,
            std::move(esquerda),
            comparacao()
        );
    }

    return esquerda;
}

// Reconhece comparacoes.
//
// Operadores: <, <=, > e >=
std::unique_ptr<ASTNode> Parser::comparacao() {

    auto esquerda = soma();

    while (eh(TokenType::Less) ||
           eh(TokenType::LessEqual) ||
           eh(TokenType::Greater) ||
           eh(TokenType::GreaterEqual)) {

        Token operador = atual();
        aceitar(operador.type);

        esquerda = criarBinario(
            operador,
            std::move(esquerda),
            soma()
        );
    }

    return esquerda;
}

// Reconhece soma e subtracao.
//
// Possuem prioridade menor que * e /.
std::unique_ptr<ASTNode> Parser::soma() {

    auto esquerda = multiplicacao();

    while (eh(TokenType::Plus) ||
           eh(TokenType::Minus)) {

        Token operador = atual();
        aceitar(operador.type);

        esquerda = criarBinario(
            operador,
            std::move(esquerda),
            multiplicacao()
        );
    }

    return esquerda;
}

// Reconhece multiplicacao e divisao.
//
// Exemplo: 2 + 3 * 4
//
// A multiplicacao sera analisada antes da soma.
std::unique_ptr<ASTNode> Parser::multiplicacao() {

    auto esquerda = unario();

    while (eh(TokenType::Star) ||
           eh(TokenType::Slash)) {

        Token operador = atual();
        aceitar(operador.type);

        esquerda = criarBinario(
            operador,
            std::move(esquerda),
            unario()
        );
    }

    return esquerda;
}

// Reconhece operadores unarios.
//
// Exemplos:
// -5
// +10
// !ativo
std::unique_ptr<ASTNode> Parser::unario() {

    if (eh(TokenType::Plus) ||
        eh(TokenType::Minus) ||
        eh(TokenType::Bang)) {

        Token operador = atual();
        aceitar(operador.type);

        auto no = std::make_unique<ASTNode>(
            NodeType::UnaryOp,
            operador.lexeme,
            "",
            operador.line,
            operador.column
        );

        no->addChild(unario());

        return no;
    }

    return primario();
}

// Reconhece elementos basicos.
//
// Exemplos:
// 20
// 1.75
// "Ola"
// true
// idade
// (2 + 3)
std::unique_ptr<ASTNode> Parser::primario() {

    Token token = atual();

    NodeType tipo;
    std::string tipoDado;

    switch (token.type) {

        case TokenType::Identifier:
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

        // Expressao entre parenteses.
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
