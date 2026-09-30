
```bash
cd docker
docker-compose build --no-cache
```

**Comunicação entre processos no mesmo host (Unix Domain Socket)**
- O exemplo usa `/tmp/demo.sock`. O `docker-compose` já monta `/tmp` para o serviço `ipc-server`.
- Para testar com containers (servidor e cliente separados):

```bash
cd docker
docker-compose up --build ipc-server &
docker-compose run --rm ipc-client
```

- Teste sem containers (compilar localmente):

```bash
g++ -std=c++17 -O2 -pthread src/cpp/ipc_server.cpp -o ipc_server
g++ -std=c++17 -O2 -pthread src/cpp/ipc_client.cpp -o ipc_client
./ipc_server &
./ipc_client
```

**Comunicação entre processos em computadores diferentes (TCP)**

```bash
docker-compose up --build tcp-server
```

```bash
g++ -pthread src/cpp/tcp_client.cpp -o tcp_client
./tcp_client 
```
