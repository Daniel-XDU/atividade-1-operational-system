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

// Estrutura do item
struct Item {
    int id;
    high_resolution_clock::time_point timestamp;
};

int main() {
    // Parâmetros
    const int N = 10; // Tamanho do buffer (Altere para testar o experimento no relatório)
    const int NUM_PRODUTORES = 3;
    const int NUM_CONSUMIDORES = 3;
    const int ITENS_POR_PRODUTOR = 50;
    
    // Variáveis do Buffer Circular
    vector<Item> buffer(N);
    int head = 0, tail = 0, count = 0;
    
    mutex mtx;
    condition_variable cv_prod, cv_cons;
    
    // Métricas
    double tempo_total_espera = 0;
    int itens_consumidos = 0;
    
    auto start_time = high_resolution_clock::now();

    // Lambda do Produtor
    auto produtor = [&](int id) {
        mt19937 rng(id ^ time(nullptr));
        uniform_int_distribution<int> dist(10, 30); // Tempo de produção (10-30ms)
        
        for (int i = 0; i < ITENS_POR_PRODUTOR; i++) {
            this_thread::sleep_for(milliseconds(dist(rng))); 
            
            Item novo_item = {i + (id * 1000), high_resolution_clock::now()};
            
            unique_lock<mutex> lock(mtx);
            // Espera ativa zero: a thread dorme até count < N
            cv_prod.wait(lock, [&]() { return count < N; }); 
            
            buffer[tail] = novo_item;
            tail = (tail + 1) % N;
            count++;
            
            cv_cons.notify_one(); // Acorda um consumidor
        }
    };

    // Lambda do Consumidor
    auto consumidor = [&](int id) {
        mt19937 rng(id ^ time(nullptr) ^ 0xFFFF);
        uniform_int_distribution<int> dist(20, 50); // Tempo de consumo (20-50ms)
        
        int itens_para_consumir = (NUM_PRODUTORES * ITENS_POR_PRODUTOR) / NUM_CONSUMIDORES;
        
        for (int i = 0; i < itens_para_consumir; i++) {
            unique_lock<mutex> lock(mtx);
            // Espera ativa zero: a thread dorme até count > 0
            cv_cons.wait(lock, [&]() { return count > 0; }); 
            
            Item item_removido = buffer[head];
            head = (head + 1) % N;
            count--;
            
            auto agora = high_resolution_clock::now();
            duration<double, milli> tempo_espera = agora - item_removido.timestamp;
            
            tempo_total_espera += tempo_espera.count();
            itens_consumidos++;
            
            cv_prod.notify_one(); // Acorda um produtor
            lock.unlock(); // libera o lock antes do sleep para não travar a fila
            
            this_thread::sleep_for(milliseconds(dist(rng))); // Simula o processamento do item
        }
    };

    // Criação e execução das threads
    vector<thread> produtores, consumidores;
    for (int i = 0; i < NUM_PRODUTORES; i++) produtores.emplace_back(produtor, i + 1);
    for (int i = 0; i < NUM_CONSUMIDORES; i++) consumidores.emplace_back(consumidor, i + 1);

    for (auto& p : produtores) p.join();
    for (auto& c : consumidores) c.join();

    // Cálculos finais
    auto end_time = high_resolution_clock::now();
    duration<double> tempo_execucao = end_time - start_time;

    cout << "=== ESTATISTICAS DO EXPERIMENTO (Buffer N=" << N << ") ===\n";
    cout << "Itens processados: " << itens_consumidos << "\n";
    cout << "Tempo total de execucao: " << tempo_execucao.count() << " s\n";
    cout << "Throughput: " << (itens_consumidos / tempo_execucao.count()) << " itens/segundo\n";
    cout << "Tempo medio de espera no buffer: " << (tempo_total_espera / itens_consumidos) << " ms\n";

    return 0;
}