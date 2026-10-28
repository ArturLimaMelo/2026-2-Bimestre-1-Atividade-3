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
- No projeto, o ambiente foi preparado com o Ubuntu 22.04

```dockerfile
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y build-essential cmake && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY src/cpp ./src
RUN g++ -pthread src/exemplo_main.cpp -o threads_demo \
 && g++ -pthread src/tcp_server.cpp -o tcp_server \
 && g++ -pthread src/tcp_client.cpp -o tcp_client 

CMD ["/app/threads_demo"]
```

E a configuração dos serviços no Docker Compose foi:

```yaml
services:
  tcp-server:
    build:
      context: ..
      dockerfile: docker/Dockerfile.cpp
    image: exemplo_cpp
    command: ["/app/tcp_server"]
    ports:
      - "9090:9090"

  tcp-client:
    build:
      context: ..
      dockerfile: docker/Dockerfile.cpp
    image: exemplo_cpp
    extra_hosts:
      - "host.docker.internal:host-gateway"
    command:
      - bash
      - -lc
      - |
        until bash -lc 'echo > /dev/tcp/host.docker.internal/9090' >/dev/null 2>&1; do
          sleep 0.2
        done
        exec /app/tcp_client
    depends_on:
      - tcp-server
```

Essas definições foram importantes porque o IPC usa socket Unix em `/tmp/demo.sock` e o TCP usa a porta `9090`, sendo necessário compartilhar o diretório `/tmp` entre os containers e aguardar a disponibilidade do serviço antes da conexão.

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

Esse exemplo mostra como threads compartilham memória no mesmo processo. Como os dois trechos operam sobre o mesmo vetor `dados`, a comunicação acontece pela área de memória compartilhada do processo, sem depender de sockets ou pipes.

### Comunicação entre tarefas em processos diferentes no mesmo computador

Neste caso, foi implementado um modelo de comunicação por Pipe nomeado (FIFO), usando um caminho como canal de comunicação. O consumidor aguarda dados que vêm deste caminho enquanto o produtor envia dados por esse caminho.

```cpp
// produtor.cpp
#include <iostream>
#include <random>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

int main() {
    const char* caminho = "/tmp/dados_fifo";

    mkfifo(caminho, 0666);

    int fd = open(caminho, O_WRONLY);

    random_device rd;
    mt19937 gerador(rd());
    uniform_int_distribution<int> distribuicao(0, 110);

    for (int i = 0; i < 100; i++) {
        int valor = distribuicao(gerador);

        if (write(fd, &valor, sizeof(valor)) != sizeof(valor)) {
            perror("write");
            close(fd);
            return 1;
        }
    }

    close(fd);

    cout << "Produtor: 100 numeros enviados." << endl;

    return 0;
}
```

```cpp
// consumidor.cpp
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

int main() {
    const char* caminho = "/tmp/dados_fifo";

    mkfifo(caminho, 0666);

    int fd = open(caminho, O_RDONLY);

    int valor;
    int resultado = 0;
    int quantidade = 0;

    while (quantidade < 100) {
        ssize_t n = read(fd, &valor, sizeof(valor));

        if (n == 0) {
            break;
        }

        if (n != sizeof(valor)) {
            cerr << "Erro ao receber um numero." << endl;
            close(fd);
            return 1;
        }

        resultado += valor;
        quantidade++;
    }

    close(fd);

    cout << "Consumidor: " << quantidade
         << " numeros recebidos." << endl;

    cout << "Soma: " << resultado << endl;

    return 0;
}
```

### Como foi executado

Foi criado um Pipe nomeado (FIFO), criando um caminho que age como um canal de rede, lendo todos os dados que forem mandados pelo pipe.

A execução foi simplismente executar os arquivos produtor e consumidor em terminais diferentes
```
./produtor
./consumidor
```
Saída obtida:
```
Consumidor: 100 numeros recebidos.
Soma: 6126
```
```
Produtor: 100 numeros enviados.
```

### Problemas e soluções

- Problema: ao tentar conectar antes que o servidor criasse o socket, o cliente retornava `connect: no such file or directory`.
- Causa: o cliente tentava acessar o arquivo do socket em `/tmp` antes do servidor estar disponível e sem o compartilhamento do diretório entre os containers.
- Solução: foi adicionado o `volumes: - /tmp:/tmp` no cliente e uma espera do tipo `until [ -S /tmp/demo.sock ]` antes da execução do cliente.

### Comunicação entre tarefas em processos diferentes em computadores diferentes

Neste cenário, a comunicação foi realizada com sockets TCP/IP. O servidor foi configurado para escutar na porta `9090`, e o cliente conectando-se pelo endereço host.docker.internal.

```cpp
// tcp_server.cpp
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int main() {
    int port = 9090;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(server_fd, 5) < 0) { perror("listen"); return 1; }

    std::cout << "TCP server listening on port " << port << std::endl;
    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) { perror("accept"); break; }
        const char *msg = "Hello from tcp_server\n";
        write(client, msg, strlen(msg));
        close(client);
    }
    close(server_fd);
    return 0;
}
```

```cpp
// tcp_client.cpp
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <netdb.h>

int main() {
    const char *server_ip = "host.docker.internal";
    int port = 9090;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }


    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *result = nullptr;
    int status = getaddrinfo(server_ip, "9090", &hints, &result);
    if (status != 0) {
        std::cerr << "Erro ao resolver servidor: "
                  << gai_strerror(status) << '\n';
        close(fd);
        return 1;
    }
    if (connect(fd, result->ai_addr, result->ai_addrlen) < 0) {
        perror("connect");
        freeaddrinfo(result);
        close(fd);
        return 1;
    }
    freeaddrinfo(result);
    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf)-1);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << "TCP client received: " << buf;
    }
    close(fd);
    return 0;
}
```

### Como foi executado

A configuração do servidor foi exposta na porta `9090` do Docker com:

```
docker compose -f docker/docker-compose.yml up --build -d

docker compose -f docker/docker-compose.yml up
```
Caso necessário, o acesso à máquina hospedeira pode ser configurado no serviço do cliente, caso o nome do docker não for disponibilizado:

```yaml
extra_hosts:
  - "host.docker.internal:host-gateway"
```

### Problemas e soluções

- Problema: o cliente não conseguia conectar-se utilizando 127.0.0.1.
- Causa: esse endereço representa o próprio container, e não a máquina hospedeira.
- Solução: foi utilizado host.docker.internal para localizar a máquina hospedeira, junto com getaddrinfo() para resolver o endereço do servidor.

## Considerações finais

Conseguimos implementar os três cenários de comunicação entre tarefas em C++: threads no mesmo processo, comunicação entre processos diferentes no mesmo computador e comunicação em redes por TCP/IP. O principal aprendizado foi perceber que cada forma de comunicação possui regras e limitações específicas: threads compartilham memória, Pipes funcionam em um mesmo host e sockets TCP são ideais para comunicação entre máquinas diferentes.

Também foi possível observar que a configuração de ambiente com Docker é fundamental para garantir reprodutibilidade, padronização e facilidade de execução. A utilização de imagens baseadas em Ubuntu, compiladores e mapeamento de portas e volumes evitou divergências entre diferentes computadores e reduziu erros de execução.


## Vídeo de execução dos códigos
https://youtu.be/3H4wToCar1o
