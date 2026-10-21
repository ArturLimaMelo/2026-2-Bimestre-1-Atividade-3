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