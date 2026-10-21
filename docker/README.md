
```bash
cd docker
docker-compose build --no-cache
```

**Comunicação entre processos em computadores diferentes (TCP)**

```bash
docker-compose up --build tcp-server
```

```bash
g++ -pthread src/cpp/tcp_client.cpp -o tcp_client
./tcp_client 
```
