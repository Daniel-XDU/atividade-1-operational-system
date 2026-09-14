#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

// We seek the perfect algorithm, but life's best answers just don't compile.
//By Xaulin_Du_Grau. ;-;


using namespace std;
using namespace std::chrono;

int main() {
    const int N = 20; // Capacidade maxima do buffer
    const int NUM_PRODUTORES = 3;
    const int NUM_CONSUMIDORES = 2;
    const int TEMPO_SIMULACAO_MS = 5000;

    vector<int> buffer(N);
    int head = 0, tail = 0, count = 0;
    
    mutex mtx;
    condition_variable cv_prod, cv_cons;
    
    bool rodando = true;
    vector<int> ocupacao_log; // Registra o tamanho do buffer ao longo do tempo

    // PRODUTOR com Rajadas/Bursts e Ociosidade
    auto produtor = [&](int id) {
        mt19937 rng(id ^ time(nullptr));
        uniform_int_distribution<int> dist_burst_size(5, 12); // Itens gerados rapidamente na rajada
        uniform_int_distribution<int> dist_fast(1, 5);        // Tempo entre itens na rajada (ms)
        uniform_int_distribution<int> dist_idle(400, 800);    // Tempo ocioso entre rajadas (ms)
        
        while (rodando) {
            int burst_size = dist_burst_size(rng);
            
            // Inicio da Rajada Burst
            for (int i = 0; i < burst_size && rodando; i++) {
                unique_lock<mutex> lock(mtx);
                
                // Monitoramento da aplicacao da trava
                if (count == N && rodando) {
                    cout << "[BACKPRESSURE] Buffer cheio! Produtor " << id << " bloqueado temporariamente.\n";
                }
                
                // Espera ativa zero bloqueio do SO
                cv_prod.wait(lock, [&]() { return count < N || !rodando; });
                
                if (!rodando) break;
                
                buffer[tail] = 1; // 1 representa um item genérico
                tail = (tail + 1) % N;
                count++;
                
                cv_cons.notify_one();
                lock.unlock();
                
                this_thread::sleep_for(milliseconds(dist_fast(rng))); // Insercao muito rapida
            }
            
            // Fim da Rajada -> Entra em Ociosidade
            if (rodando) {
                this_thread::sleep_for(milliseconds(dist_idle(rng)));
            }
        }
    };

    // CONSUMIDOR Taxa Constante
    auto consumidor = [&](int id) {
        mt19937 rng(id ^ time(nullptr) ^ 0xABCD);
        uniform_int_distribution<int> dist_cons(40, 80); // Consumo constante e moderado
        
        while (rodando) {
            unique_lock<mutex> lock(mtx);
            cv_cons.wait(lock, [&]() { return count > 0 || !rodando; });
            
            if (!rodando && count == 0) break;
            
            if (count > 0) {
                head = (head + 1) % N;
                count--;
                cv_prod.notify_one();
            }
            lock.unlock();
            
            this_thread::sleep_for(milliseconds(dist_cons(rng)));
        }
    };

    // MONITOR DE OCUPACAO Amostragem
    auto monitor = [&]() {
        while (rodando) {
            this_thread::sleep_for(milliseconds(250)); // Coleta amostra a cada 250ms
            lock_guard<mutex> lock(mtx);
            ocupacao_log.push_back(count);
        }
    };

    cout << "=== INICIANDO SIMULACAO DE BURSTS E BACKPRESSURE ===\n";
    cout << "Simulando por " << TEMPO_SIMULACAO_MS / 1000 << " segundos...\n\n";

    vector<thread> produtores, consumidores;
    for (int i = 0; i < NUM_PRODUTORES; i++) produtores.emplace_back(produtor, i + 1);
    for (int i = 0; i < NUM_CONSUMIDORES; i++) consumidores.emplace_back(consumidor, i + 1);
    thread thread_monitor(monitor);

    // Aguarda o tempo da simulacao
    this_thread::sleep_for(milliseconds(TEMPO_SIMULACAO_MS));
    
    // Procedimento de encerramento
    {
        lock_guard<mutex> lock(mtx);
        rodando = false;
    }
    cv_prod.notify_all();
    cv_cons.notify_all();

    for (auto& p : produtores) p.join();
    for (auto& c : consumidores) c.join();
    thread_monitor.join();

    cout << "\n=== HISTORICO DE OCUPACAO DO BUFFER (Capacidade Maxima: " << N << ") ===\n[ ";
    for (int c : ocupacao_log) {
        cout << c << " ";
    }
    cout << "]\n";

    return 0;
}