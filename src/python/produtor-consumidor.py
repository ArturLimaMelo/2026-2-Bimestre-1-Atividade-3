from random import randint
import threading

dados = []

def produzir_dados():
    print('# produzir - iniciado')
    global dados
    dados = [randint(0, 110) for _ in range(100)]
    print(f'# produzir {dados}')
    print('# produzir - terminado')
    return dados

def consumir_dados():
    global dados
    print('### consumir - iniciado')
    print(f'### dados -> {dados}')
    resultado = sum(dados)
    print(f'### resultado -> {resultado}')
    print('### consumir - terminado')

def principal():
    print("iniciou")
    global dados
    # dados = produzir_dados()
    thread_produtor = threading.Thread(target=produzir_dados)
    thread_consumidor = threading.Thread(target=consumir_dados)

    thread_produtor.start()
    thread_consumidor.start()

    print("finalizou")

if __name__ == '__main__':
    principal()