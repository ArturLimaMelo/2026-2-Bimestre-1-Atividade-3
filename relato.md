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
 && g++ -pthread src/ipc_server.cpp -o ipc_server \
 && g++ -pthread src/ipc_client.cpp -o ipc_client \
 && g++ -pthread src/tcp_server.cpp -o tcp_server \
 && g++ -pthread src/tcp_client.cpp -o tcp_client || true

CMD ["/app/threads_demo"]
```

E a configuração dos serviços no Docker Compose foi:

```yaml
services:
  ipc-server:
    build:
      context: ..
      dockerfile: docker/Dockerfile.cpp
    image: exemplo_cpp
    command: ["/app/ipc_server"]
    volumes:
      - /tmp:/tmp

  ipc-client:
    build:
      context: ..
      dockerfile: docker/Dockerfile.cpp
    image: exemplo_cpp
    command: >
      sh -c "until [ -S /tmp/demo.sock ]; do sleep 0.2; done; /app/ipc_client"
    volumes:
      - /tmp:/tmp
    depends_on:
      - ipc-server

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
    command: >
      bash -lc "until bash -lc 'echo > /dev/tcp/tcp-server/9090' >/dev/null 2>&1; do sleep 0.2; done; /app/tcp_client"
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

Neste caso, foi implementado um modelo de comunicação por socket Unix, usando um processo servidor e um processo cliente na mesma máquina. O servidor fica aguardando conexões em um socket localizado em `/tmp/demo.sock`; o cliente envia uma requisição e recebe uma mensagem do servidor.

```cpp
// ipc_server.cpp
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int main() {
    const char *socket_path = "/tmp/demo.sock";
    unlink(socket_path);

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == -1) { perror("bind"); return 1; }
    if (listen(server_fd, 5) == -1) { perror("listen"); return 1; }

    std::cout << "IPC server listening on " << socket_path << std::endl;

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client == -1) { perror("accept"); break; }
        const char *msg = "Hello from ipc_server\n";
        write(client, msg, strlen(msg));
        close(client);
    }

    close(server_fd);
    unlink(socket_path);
    return 0;
}
```

```cpp
// ipc_client.cpp
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int main() {
    const char *socket_path = "/tmp/demo.sock";
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }

    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) == -1) { perror("connect"); return 1; }

    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf)-1);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << "Client received: " << buf;
    }
    close(fd);
    return 0;
}
```

### Como foi executado

Os serviços foram executados no Docker Compose com os containers `ipc-server` e `ipc-client`. O servidor foi configurado para criar o socket `/tmp/demo.sock`, enquanto o cliente esperou a criação desse arquivo antes de tentar a conexão.

A execução foi validada com o comando:

```bash
docker compose -f docker/docker-compose.yml up --build ipc-server ipc-client
```

Saída obtida:

```text
ipc-server-1  | IPC server listening on /tmp/demo.sock
ipc-client-1  | Client received: Hello from ipc_server
ipc-client-1 exited with code 0
```

### Problemas e soluções

- Problema: ao tentar conectar antes que o servidor criasse o socket, o cliente retornava `connect: no such file or directory`.
- Causa: o cliente tentava acessar o arquivo do socket em `/tmp` antes do servidor estar disponível e sem o compartilhamento do diretório entre os containers.
- Solução: foi adicionado o `volumes: - /tmp:/tmp` no cliente e uma espera do tipo `until [ -S /tmp/demo.sock ]` antes da execução do cliente.

### Comunicação entre tarefas em processos diferentes em computadores diferentes

Neste cenário, a comunicação foi realizada com sockets TCP/IP. O servidor foi configurado para escutar na porta `9090`, enquanto o cliente se conecta ao endereço `127.0.0.1` na mesma porta.

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

    sockaddr_in addr;
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

int main() {
    const char *server_ip = "127.0.0.1";
    int port = 9090;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &addr.sin_addr);

    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("connect"); return 1; }
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

```yaml
ports:
  - "9090:9090"
```

E o cliente foi ajustado para aguardar o servidor responder antes de tentar a conexão:

```yaml
command: >
  bash -lc "until bash -lc 'echo > /dev/tcp/tcp-server/9090' >/dev/null 2>&1; do sleep 0.2; done; /app/tcp_client"
```

### Problemas e soluções

- Problema inicial: o cliente TCP tentava conectar antes que o servidor tivesse seu socket pronto e recebia `connect: Connection refused`.
- Causa: `depends_on` garante apenas a ordem de criação dos containers, não a disponibilidade da porta de rede.
- Solução: foi inserida uma espera ativa na porta `9090` antes de chamar o cliente.

## Considerações finais

Conseguimos implementar os três cenários de comunicação entre tarefas em C++: threads no mesmo processo, comunicação entre processos diferentes no mesmo computador e comunicação em redes por TCP/IP. O principal aprendizado foi perceber que cada forma de comunicação possui regras e limitações específicas: threads compartilham memória, sockets Unix funcionam em um mesmo host e sockets TCP são ideais para comunicação entre máquinas diferentes.

Também foi possível observar que a configuração de ambiente com Docker é fundamental para garantir reprodutibilidade, padronização e facilidade de execução. A utilização de imagens baseadas em Ubuntu, compiladores e mapeamento de portas e volumes evitou divergências entre diferentes computadores e reduziu erros de execução.

Como recomendação para próximos alunos, vale enfatizar o uso de boas práticas de sincronização, a validação da ordem de inicialização dos serviços e o cuidado com o compartilhamento de recursos entre containers. Esses pontos são fundamentais para a construção de sistemas concorrentes e distribuídos de forma confiável.

## Vídeo de execução dos códigos
https://youtu.be/3H4wToCar1o
