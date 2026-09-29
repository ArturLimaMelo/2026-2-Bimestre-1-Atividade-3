#include <iostream>
#include <vector>
#include <random>
#include <thread>

using namespace std;

vector<int> dados;

void produzir_dados() {
    cout << "# produzir - iniciado" << endl;

    random_device rd;
    mt19937 gerador(rd());
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
