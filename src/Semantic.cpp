
#include "Semantic.h"

#include <stdexcept>
#include <utility>

// Abre um novo escopo para armazenar variaveis.
void Semantic::abrirEscopo() {
    escopos.emplace_back();
}

// Fecha o escopo atual.
//
// As variaveis declaradas nele deixam
// de estar acessiveis.
void Semantic::fecharEscopo() {
    escopos.pop_back();
}

// Apresenta uma mensagem de erro semantico
// com a linha e coluna do codigo original.
[[noreturn]]
void Semantic::erro(
    const ASTNode& no,
    const std::string& mensagem
) {
    throw std::runtime_error(
        "Erro semantico na linha " +
        std::to_string(no.line) +
        ", coluna " +
        std::to_string(no.column) +
        ": " + mensagem
    );
}

// Registra uma variavel no escopo atual.
//
// CORRECAO 2:
// Nao permitimos variaveis com nomes de funcoes.
//
// Isso evita conflitos no codigo C++ gerado,
// inclusive quando um parametro ou variavel local
// possui o nome de uma funcao.
void Semantic::declarar(const ASTNode& no) {

    if (funcoes.count(no.value) != 0) {
        erro(
            no,
            "nome '" + no.value +
            "' ja utilizado por uma funcao"
        );
    }

    auto& atual = escopos.back();

    // Tambem impede duplicidade no mesmo escopo.
    if (atual.count(no.value) != 0) {
        erro(
            no,
            "variavel '" + no.value +
            "' ja declarada neste escopo"
        );
    }

    atual[no.value] = no.dataType;
}

// Busca uma variavel nos escopos ativos.
//
// Comeca pelo escopo mais interno,
// permitindo acesso a variaveis externas.
std::string Semantic::buscarVariavel(
    const ASTNode& no
) const {

    for (auto escopo = escopos.rbegin();
         escopo != escopos.rend();
         ++escopo) {

        auto encontrado =
            escopo->find(no.value);

        if (encontrado != escopo->end()) {
            return encontrado->second;
        }
    }

    erro(
        no,
        "variavel '" + no.value +
        "' nao declarada"
    );
}

// Verifica se um tipo representa um numero.
bool Semantic::numerico(
    const std::string& tipo
) {
    return tipo == "int" || tipo == "float";
}

// Verifica se um valor pode ser atribuido
// a uma variavel de determinado tipo.
//
// Permite tipos iguais e int -> float.
bool Semantic::compativel(
    const std::string& destino,
    const std::string& origem
) {
    return destino == origem ||
           (destino == "float" && origem == "int");
}

// Registra as funcoes antes de analisar seus corpos.
//
// Isso permite chamadas recursivas e chamadas
// a funcoes declaradas posteriormente.
void Semantic::registrarFuncoes(
    const ASTNode& programa
) {
    for (const auto& filho : programa.children) {

        const ASTNode& no = *filho;

        if (no.type != NodeType::Function) {
            continue;
        }

        if (funcoes.count(no.value) != 0) {
            erro(
                no,
                "funcao '" + no.value +
                "' declarada mais de uma vez"
            );
        }

        Assinatura assinatura;
        assinatura.retorno = no.dataType;

        // Registra os tipos dos parametros.
        for (const auto& parte : no.children) {

            if (parte->type == NodeType::Parameter) {
                assinatura.parametros.push_back(
                    parte->dataType
                );
            }
        }

        funcoes.emplace(
            no.value,
            std::move(assinatura)
        );
    }
}

// Executa a analise semantica do programa.
//
// Primeiro registra as funcoes.
// Depois verifica os comandos globais.
// Por ultimo, analisa os corpos das funcoes.
void Semantic::analisar(
    const ASTNode& programa
) {
    if (programa.type != NodeType::Program) {
        erro(
            programa,
            "a raiz da AST precisa ser Program"
        );
    }

    // Limpa informacoes de analises anteriores.
    escopos.clear();
    funcoes.clear();
    retornoAtual.clear();

    abrirEscopo();

    registrarFuncoes(programa);

    // Analise dos comandos globais.
    for (const auto& filho : programa.children) {

        if (filho->type != NodeType::Function) {
            analisarComando(*filho);
        }
    }

    // Analise dos corpos das funcoes.
    for (const auto& filho : programa.children) {

        if (filho->type == NodeType::Function) {
            analisarFuncao(*filho);
        }
    }

    fecharEscopo();
}

// Analisa uma funcao individualmente.
//
// Cada funcao recebe seu proprio escopo.
// Seus parametros sao registrados como variaveis.
void Semantic::analisarFuncao(
    const ASTNode& funcao
) {
    abrirEscopo();

    retornoAtual = funcao.dataType;

    // Registra os parametros recebidos.
    for (const auto& filho : funcao.children) {

        if (filho->type == NodeType::Parameter) {
            declarar(*filho);
        }
    }

    // O ultimo filho deve conter o corpo.
    if (funcao.children.empty() ||
        funcao.children.back()->type != NodeType::Block) {

        erro(
            funcao,
            "corpo da funcao ausente"
        );
    }

    // Analisa as instrucoes da funcao.
    analisarBloco(
        *funcao.children.back(),
        false
    );

    // CORRECAO 1:
    // Exige retorno em todos os caminhos possiveis.
    //
    // Nao basta encontrar um return em qualquer
    // ponto do programa.
    if (!garanteRetorno(*funcao.children.back())) {
        erro(
            funcao,
            "funcao '" + funcao.value +
            "' pode terminar sem retornar " +
            funcao.dataType
        );
    }

    retornoAtual.clear();

    fecharEscopo();
}

// Analisa todos os comandos de um bloco.
//
// Quando necessario, cria um escopo especifico
// e o fecha depois de analisar os comandos.
void Semantic::analisarBloco(
    const ASTNode& bloco,
    bool criarEscopo
) {
    if (criarEscopo) {
        abrirEscopo();
    }

    for (const auto& comando : bloco.children) {
        analisarComando(*comando);
    }

    if (criarEscopo) {
        fecharEscopo();
    }
}

// CORRECAO 1 - VERIFICACAO DE RETURN
//
// Regras:
// - return garante retorno.
// - Um bloco garante retorno se algum comando
//   necessariamente encerrar sua execucao.
// - if/else garante retorno somente quando
//   ambos os caminhos retornam.
// - while e for nao garantem retorno, pois
//   podem executar zero vezes.
//
// Esta verificacao e intencionalmente simples.
bool Semantic::garanteRetorno(
    const ASTNode& no
) {
    if (no.type == NodeType::Return) {
        return true;
    }

    if (no.type == NodeType::Block) {

        for (const auto& comando : no.children) {

            if (garanteRetorno(*comando)) {
                return true;
            }
        }

        return false;
    }

    if (no.type == NodeType::If) {

        // Deve existir um bloco if e um else.
        return no.children.size() == 3 &&
            garanteRetorno(*no.children.at(1)) &&
            garanteRetorno(*no.children.at(2));
    }

    // Os demais comandos nao garantem retorno.
    return false;
}

// Descobre o tipo de uma expressao.
//
// Exemplo:
// 20      -> int
// 1.5     -> float
// true    -> bool
// 2 + 3   -> int
// 5 > 2   -> bool
std::string Semantic::tipoExpressao(
    const ASTNode& no
) {
    switch (no.type) {

        case NodeType::IntegerLiteral:
            return "int";

        case NodeType::FloatLiteral:
            return "float";

        case NodeType::StringLiteral:
            return "string";

        case NodeType::CharLiteral:
            return "char";

        case NodeType::BoolLiteral:
            return "bool";

        // Busca o tipo na tabela de variaveis.
        case NodeType::Identifier:
            return buscarVariavel(no);

        // Verifica chamadas de funcoes.
        case NodeType::Call: {

            auto f = funcoes.find(no.value);

            if (f == funcoes.end()) {
                erro(
                    no,
                    "funcao '" + no.value +
                    "' nao declarada"
                );
            }

            const Assinatura& a = f->second;

            // Verifica quantidade de argumentos.
            if (no.children.size() !=
                a.parametros.size()) {

                erro(
                    no,
                    "quantidade incorreta de argumentos em '" +
                    no.value + "'"
                );
            }

            // Verifica os tipos dos argumentos.
            for (std::size_t i = 0;
                 i < no.children.size();
                 ++i) {

                std::string recebido =
                    tipoExpressao(*no.children[i]);

                if (!compativel(
                    a.parametros[i],
                    recebido
                )) {
                    erro(
                        *no.children[i],
                        "tipo incorreto no argumento de '" +
                        no.value + "'"
                    );
                }
            }

            return a.retorno;
        }

        // Operadores de um operando.
        case NodeType::UnaryOp: {

            std::string tipo =
                tipoExpressao(*no.children.at(0));

            if (no.value == "!") {

                if (tipo != "bool") {
                    erro(
                        no,
                        "operador ! exige bool"
                    );
                }

                return "bool";
            }

            if (no.value == "+" ||
                no.value == "-") {

                if (!numerico(tipo)) {
                    erro(
                        no,
                        "operador unario exige numero"
                    );
                }

                return tipo;
            }

            erro(no, "operador unario desconhecido");
        }

        // Operadores de dois operandos.
        case NodeType::BinaryOp: {

            std::string e =
                tipoExpressao(*no.children.at(0));

            std::string d =
                tipoExpressao(*no.children.at(1));

            const std::string& op = no.value;

            // Permite concatenar texto com numeros.
            // Necessario para o fatorial do professor.
            if (op == "+" &&
                (e == "string" || d == "string")) {

                return "string";
            }

            // Operacoes aritmeticas.
            if (op == "+" || op == "-" ||
                op == "*" || op == "/") {

                if (!numerico(e) || !numerico(d)) {
                    erro(
                        no,
                        "operacao aritmetica exige numeros"
                    );
                }

                return (e == "float" || d == "float")
                    ? "float"
                    : "int";
            }

            // Comparacoes de ordem.
            if (op == "<" || op == "<=" ||
                op == ">" || op == ">=") {

                if (!numerico(e) || !numerico(d)) {
                    erro(
                        no,
                        "comparacao de ordem exige numeros"
                    );
                }

                return "bool";
            }

            // Comparacoes de igualdade.
            if (op == "==" || op == "!=") {

                if (e != d &&
                    !(numerico(e) && numerico(d))) {

                    erro(
                        no,
                        "comparacao entre tipos incompativeis"
                    );
                }

                return "bool";
            }

            // Operadores logicos.
            if (op == "&&" || op == "||") {

                if (e != "bool" || d != "bool") {
                    erro(
                        no,
                        "operador logico exige bool"
                    );
                }

                return "bool";
            }

            erro(no, "operador binario desconhecido");
        }

        default:
            erro(
                no,
                "estrutura invalida onde era "
                "esperada uma expressao"
            );
    }
}

// Verifica as regras dos comandos da MACSLang.
void Semantic::analisarComando(
    const ASTNode& no
) {
    switch (no.type) {

        // Declaracao de variavel.
        case NodeType::VarDecl: {

            declarar(no);

            // Verifica o valor inicial, se existir.
            if (!no.children.empty()) {

                std::string valor =
                    tipoExpressao(*no.children[0]);

                if (!compativel(
                    no.dataType,
                    valor
                )) {
                    erro(
                        no,
                        "nao e possivel atribuir " +
                        valor +
                        " a variavel '" +
                        no.value +
                        "' do tipo " +
                        no.dataType
                    );
                }
            }

            break;
        }

        // Atribuicao a variavel existente.
        case NodeType::Assignment: {

            std::string destino =
                buscarVariavel(no);

            std::string origem =
                tipoExpressao(*no.children.at(0));

            if (!compativel(destino, origem)) {
                erro(
                    no,
                    "atribuicao de " +
                    origem + " a '" +
                    no.value +
                    "' do tipo " + destino
                );
            }

            break;
        }

        // Entrada exige variavel declarada.
        case NodeType::Input:
            (void)buscarVariavel(no);
            break;

        // Verifica a expressao impressa ou chamada.
        case NodeType::Print:
        case NodeType::ExpressionStatement:
            (void)tipoExpressao(
                *no.children.at(0)
            );
            break;

        // Blocos com escopos proprios.
        case NodeType::Block:
            analisarBloco(no);
            break;

        // Condicao do if precisa ser bool.
        case NodeType::If: {

            if (tipoExpressao(*no.children.at(0))
                != "bool") {

                erro(
                    no,
                    "condicao do if precisa ser bool"
                );
            }

            analisarBloco(*no.children.at(1));

            if (no.children.size() == 3) {
                analisarBloco(*no.children.at(2));
            }

            break;
        }

        // Condicao do while precisa ser bool.
        case NodeType::While: {

            if (tipoExpressao(*no.children.at(0))
                != "bool") {

                erro(
                    no,
                    "condicao do while precisa ser bool"
                );
            }

            analisarBloco(*no.children.at(1));
            break;
        }

        // Analise do laco for.
        case NodeType::For: {

            // Variavel declarada no for existe
            // somente durante a repeticao.
            abrirEscopo();

            analisarComando(*no.children.at(0));

            if (tipoExpressao(*no.children.at(1))
                != "bool") {

                erro(
                    no,
                    "condicao do for precisa ser bool"
                );
            }

            analisarComando(*no.children.at(2));
            analisarBloco(*no.children.at(3));

            fecharEscopo();
            break;
        }

        // Verifica o tipo do return.
        //
        // A garantia de retorno em todos os caminhos
        // e verificada separadamente na funcao
        // garanteRetorno().
        case NodeType::Return: {

            if (retornoAtual.empty()) {
                erro(
                    no,
                    "return fora de uma funcao"
                );
            }

            std::string tipo =
                tipoExpressao(*no.children.at(0));

            if (!compativel(
                retornoAtual,
                tipo
            )) {
                erro(
                    no,
                    "return do tipo " +
                    tipo +
                    " em funcao que retorna " +
                    retornoAtual
                );
            }

            break;
        }

        default:
            erro(
                no,
                "comando desconhecido na "
                "analise semantica"
            );
    }
}
