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
