#include <iostream>
#include <random>
#include <vector>

using namespace std;


vector<int> produzir_dados() {
    vector<int> dados;

    random_device rd; // Gera uma semente para a geração dos números inteiros
    mt19937 gerador(rd()); // Gera os números aleatorios
    uniform_int_distribution<int> distribuicao(0, 110); // Transforma em uma sequência de 0 a 110

    for (int i = 0; i < 100; i++) {
        dados.push_back(distribuicao(gerador));
    }

    return dados;
}


void consumir_dados(vector<int> dados) {
    int resultado;
    for (int i = 0; i < 100; ++i) {
        resultado += dados[i];
    }
    cout << "Recebeu -> " << resultado << endl;
}
int main() {
    cout << "iniciou" << endl;
    vector<int> dados;
    dados = produzir_dados();
    consumir_dados(dados);
    cout << "finalizou" << endl;
    return 0;
}
