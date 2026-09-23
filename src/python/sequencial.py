from random import randint

def produzir_dados():
    # return randint(0, 100)
    # []
    # preenchimento de 100 (range(100))
    # para cada execução na interação, randint(0, 110)
    # dados = []
    # for _ in range(100):
    #    dados.append(randint(0, 110))
    dados = [randint(0, 110) for _ in range(100)]
    return dados

def consumir_dados(dados):
    resultado = sum(dados)
    print(f'recebeu -> {resultado}')

def principal():
    print("iniciou")
    dados = produzir_dados()
    consumir_dados(dados)
    print("finalizou")

if __name__ == '__main__':
    principal()