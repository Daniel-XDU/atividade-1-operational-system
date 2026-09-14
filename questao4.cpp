#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

// We seek the perfect algorithm, but life's best answers just don't compile.
//By Xaulin_Du_Grau. ;-;

using namespace std;

// Estrutura do item processado
struct Item {
    int id;
    int valor;
};

// Classe auxiliar para instanciar as filas limitadas (Bounded Queues)
class FilaLimitada {
private:
    queue<Item> fila;
    int capacidade;
    mutex mtx;
    condition_variable cv_produtor;
    condition_variable cv_consumidor;

public:
    FilaLimitada(int cap) : capacidade(cap) {}

    void push(Item item) {
        unique_lock<mutex> lock(mtx);
        cv_produtor.wait(lock, [this]() { return fila.size() < capacidade; });
        fila.push(item);
        cv_consumidor.notify_one();
    }

    Item pop() {
        unique_lock<mutex> lock(mtx);
        cv_consumidor.wait(lock, [this]() { return !fila.empty(); });
        Item item = fila.front();
        fila.pop();
        cv_produtor.notify_one();
        return item;
    }
};

int main() {
    const int N = 50; // Total de itens a processar
    const int TAMANHO_FILA = 10;
    const int POISON_PILL = -1; // Sinal de encerramento limpo

    FilaLimitada fila_captura_processamento(TAMANHO_FILA);
    FilaLimitada fila_processamento_gravacao(TAMANHO_FILA);

    cout << "=== INICIANDO PIPELINE DE PROCESSAMENTO ===\n";

    // Captura
    auto captura = [&]() {
        for (int i = 1; i <= N; i++) {
            Item novo_item = {i, i * 10}; // Simula a geracao de um dado
            fila_captura_processamento.push(novo_item);
            this_thread::sleep_for(chrono::milliseconds(5)); 
        }
        
        // Envia a Poison Pill ao terminar
        fila_captura_processamento.push({POISON_PILL, 0});
        cout << "[CAPTURA] Encerrada. Poison Pill enviada.\n";
    };

    // Processamento
    auto processamento = [&]() {
        while (true) {
            Item item = fila_captura_processamento.pop();
            
            // Verifica a Poison Pill
            if (item.id == POISON_PILL) {
                fila_processamento_gravacao.push(item); // Repassa a pilula adiante
                cout << "[PROCESSAMENTO] Encerrado. Poison Pill repassada.\n";
                break;
            }

            // Simula processamento pesado
            item.valor += 5; 
            this_thread::sleep_for(chrono::milliseconds(10)); 
            
            fila_processamento_gravacao.push(item);
        }
    };

    // Gravacao
    auto gravacao = [&]() {
        int itens_gravados = 0;
        while (true) {
            Item item = fila_processamento_gravacao.pop();
            
            // Verifica a Poison Pill
            if (item.id == POISON_PILL) {
                cout << "[GRAVACAO] Encerrada. Poison Pill recebida.\n";
                break;
            }

            // Registra a gravacao
            itens_gravados++;
        }
        
        cout << "\n=== RELATORIO FINAL ===\n";
        cout << "Itens esperados: " << N << "\n";
        cout << "Itens gravados com sucesso: " << itens_gravados << "\n";
        
        if (itens_gravados == N) {
            cout << "[OK] Nenhum item foi perdido no pipeline.\n";
        } else {
            cout << "[ERRO] Ocorreu perda de dados!\n";
        }
    };

    // Dispara as threads
    thread t1(captura);
    thread t2(processamento);
    thread t3(gravacao);

    // Aguarda o encerramento limpo (se houver deadlock, o join() nunca terminara)
    t1.join();
    t2.join();
    t3.join();

    cout << "[OK] Pipeline finalizado sem deadlocks. Todas as threads retornaram limpas.\n";

    return 0;
}