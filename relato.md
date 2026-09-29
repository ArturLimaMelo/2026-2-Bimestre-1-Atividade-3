# Relatório sobre implementação de comunicação entre tarefas em C++

## Introdução

Este relato faz parte do processo avaliativo da disciplina de sistemas operacionas no curso superior em análise e desenvolvimento de sistemas, ofertado na Diretoria acadêmica de gestão e tecnologia da informação no campus natal-central do instituto federal de educação, ciência e tecnologia do rio grande do norte.

Tem como objetivo principal relatar as implementações de comunicação entre tarefas na linguagem C++.

O grupo de trabalho foi formado por Artur Lima Melo, Caio Lucas Alves de Oliveira e Arthur Vinicius Barreto Demetrio.

## Comunicação entre tarefas em C++

### Informações gerais

- Permite que diferentes tarefas **troquem informações**.
- Foram estudados três cenários:
  - Threads no mesmo processo;
  - Processos diferentes no mesmo computador;
  - Processos em computadores diferentes.

### Docker

- Padronização do ambiente de execução.
- Facilita a compilação e execução.
- Evita diferenças de configuração entre computadores.

### Comunicação entre tarefas com linhas de execução no mesmo processo

### Funcionamento

- Uma thread **produz** 100 números aleatórios.
- Os dados são armazenados em um vetor compartilhado.
- Outra thread **consome** os dados e calcula a soma.
- `join()` garante que o produtor termine antes do consumidor.

```cpp
#include <iostream>
#include <vector>
#include <random>
#include <thread>

using namespace std;

vector<int> dados;

void produzir_dados() {
    cout << "# produzir - iniciado" << endl;

    random_device rd; # Gera uma semente para criar números aleatórios
    mt19937 gerador(rd()); # Inicializa o gerador usando a semente
    uniform_int_distribution<int> distribuicao(0, 110);

    dados.clear();

    for (int i = 0; i < 100; i++) {
        dados.push_back(distribuicao(gerador));
    }

    cout << "# produzir - terminado" << endl;
}

void consumir_dados() {
    cout << "### consumir - iniciado" << endl;

    cout << "### dados -> ";
    for (int valor : dados) {
        cout << valor << " ";
    }
    cout << endl;

    int resultado = 0;

    for (int valor : dados) {
        resultado += valor;
    }

    cout << "### resultado -> " << resultado << endl;
    cout << "### consumir - terminado" << endl;
}

void principal() {
    cout << "iniciou" << endl;

    thread thread_produtor(produzir_dados);

    thread_produtor.join();

    thread thread_consumidor(consumir_dados);

    thread_consumidor.join();

    cout << "finalizou" << endl;
}

int main() {
    principal();

    return 0;
}
```

### Execução
```
iniciou
# produzir - iniciado
# produzir - terminado
### consumir - iniciado
### resultado -> 5423
### consumir - terminado
finalizou
```

### Comunicação entre tarefas em processos diferentes no mesmo computador

FIXME
> texto explicando o código
> mostrar o código completo

FIXME
> explicar como foi executado
> mostrar as saídas do terminal
> mostrar as saídas do terminal

FIXME
> se houve problema na execução, enumerar os problemas e suas respectivas soluções

### Comunicação entre tarefas em processos diferentes em computadores diferentes

FIXME
> texto explicando o código
> mostrar o código completo

FIXME
> explicar como foi executado
> mostrar as saídas do terminal
> mostrar as saídas do terminal

FIXME
> se houve problema na execução, enumerar os problemas e suas respectivas soluções

## Considerações finais

FIXME
> conseguiu implementar tudo e executar?
> qual foi o aprendizado nesse trabalho?
> alguma recomendação para próximos alunos?
