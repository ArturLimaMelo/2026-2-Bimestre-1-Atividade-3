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