#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cmath>

// We seek the perfect algorithm, but life's best answers just don't compile.
//By Xaulin_Du_Grau. ;-;

using namespace std;

int main() {
    const int NUM_THREADS = 4;
    
    queue<int> fila_tarefas;
    mutex mtx_fila;
    mutex mtx_print;
    condition_variable cv;
    bool stop_pool = false;

    atomic<int> tarefas_enviadas(0);
    atomic<int> tarefas_concluidas(0);

    // Teste de Primalidade
    auto eh_primo = [](int n) {
        if (n <= 1) return false;
        if (n <= 3) return true;
        if (n % 2 == 0 || n % 3 == 0) return false;
        for (int i = 5; i * i <= n; i += 6) {
            if (n % i == 0 || n % (i + 2) == 0)
                return false;
        }
        return true;
    };

    // Lambda do Worker thread do Pool
    auto worker = [&](int id) {
        while (true) {
            int tarefa = 0;
            
            // Regiao Critica da Fila
            {
                unique_lock<mutex> lock(mtx_fila);
                // Espera ate ter tarefa ou receber sinal de parada
                cv.wait(lock, [&]() { return stop_pool || !fila_tarefas.empty(); });
                
                // Se mandou parar e a fila esvaziou, a thread pode morrer em paz
                if (stop_pool && fila_tarefas.empty()) {
                    break; 
                }
                
                tarefa = fila_tarefas.front();
                fila_tarefas.pop();
            } // Destrava o mutex da fila automaticamente aqui

            // Processamento pesado (livre de travas, permite paralelismo total)
            bool primo = eh_primo(tarefa);
            
            // Regiao Critica do Print
            {
                lock_guard<mutex> lock_print(mtx_print);
                cout << "[Thread " << id << "] Processou o numero " << tarefa 
                     << (primo ? " -> PRIMO\n" : " -> NAO PRIMO\n");
            }
            
            tarefas_concluidas++;
        }
    };

    // Inicializa o Pool de Threads
    vector<thread> pool;
    for (int i = 0; i < NUM_THREADS; i++) {
        pool.emplace_back(worker, i + 1);
    }

    // Leitura da entrada padrao
    cout << "=== POOL DE THREADS INICIADO ===\n";
    cout << "Digite numeros para testar a primalidade. (Pressione Ctrl+Z no Windows ou Ctrl+D no Linux para finalizar/EOF)\n";

    int numero;
    while (cin >> numero) {
        {
            lock_guard<mutex> lock(mtx_fila);
            fila_tarefas.push(numero);
        }
        tarefas_enviadas++;
        cv.notify_one(); // Acorda uma thread ociosa
    }

    // Rotina de Encerramento (Sinalizacao Apropriada)
    cout << "\n[MAIN] EOF detectado. Iniciando encerramento do pool...\n";
    {
        lock_guard<mutex> lock(mtx_fila);
        stop_pool = true;
    }
    cv.notify_all(); // Acorda TODAS as threads para que percebam o stop_pool

    // Aguarda todas terminarem
    for (auto& t : pool) {
        t.join();
    }

    // Relatorio e Prova de Integridade
    cout << "\n=== RELATORIO FINAL DO POOL ===\n";
    cout << "Tarefas enviadas: " << tarefas_enviadas << "\n";
    cout << "Tarefas concluidas: " << tarefas_concluidas << "\n";
    
    if (tarefas_enviadas == tarefas_concluidas) {
        cout << "[SUCESSO] A fila eh thread-safe e NENHUMA tarefa foi perdida!\n";
    } else {
        cout << "[FALHA] Discrepancia no numero de tarefas.\n";
    }

    return 0;
}