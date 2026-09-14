#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <random>

// We seek the perfect algorithm, but life's best answers just don't compile.
//By Xaulin_Du_Grau. ;-;

using namespace std;
using namespace std::chrono;

// Implementacao de Semaforo para compatibilidade com C++11/14/17
class Semaforo {
private:
    int count;
    mutex mtx;
    condition_variable cv;
public:
    Semaforo(int c) : count(c) {}
    void acquire() {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]() { return count > 0; });
        count--;
    }
    void release() {
        unique_lock<mutex> lock(mtx);
        count++;
        cv.notify_one();
    }
};

struct Metricas {
    int refeicoes = 0;
    double maior_espera_ms = 0.0;
};

int main() {
    const int NUM_FILOSOFOS = 5;
    const int CICLOS_POR_FILOSOFO = 20;

    vector<mutex> garfos(NUM_FILOSOFOS);
    vector<Metricas> metricas(NUM_FILOSOFOS);

    // --- SOLUCAO A: ORDEM GLOBAL DE AQUISICAO ---
    cout << "=== SOLUCAO A: ORDEM GLOBAL HIERARQUIA DE RECURSOS ===\n";

    auto filosofo_ordem_global = [&](int id) {
        mt19937 rng(id ^ time(nullptr));
        uniform_int_distribution<int> dist_tempo(5, 15);

        int garfo_esq = id;
        int garfo_dir = (id + 1) % NUM_FILOSOFOS;

        // Regra da ordem global: sempre travar o garfo de menor ID primeiro
        int garfo_1 = min(garfo_esq, garfo_dir);
        int garfo_2 = max(garfo_esq, garfo_dir);

        for (int i = 0; i < CICLOS_POR_FILOSOFO; i++) {
            // Pensando mitigacao de starvation: tempo aleatorio permite alternancia
            this_thread::sleep_for(milliseconds(dist_tempo(rng)));

            auto inicio_espera = high_resolution_clock::now();

            // Aquisicao em ordem estrita evita a Espera Circular
            garfos[garfo_1].lock();
            garfos[garfo_2].lock();

            auto fim_espera = high_resolution_clock::now();
            duration<double, milli> tempo_espera = fim_espera - inicio_espera;

            metricas[id].maior_espera_ms = max(metricas[id].maior_espera_ms, tempo_espera.count());
            metricas[id].refeicoes++;

            // Comendo
            this_thread::sleep_for(milliseconds(dist_tempo(rng)));

            garfos[garfo_2].unlock();
            garfos[garfo_1].unlock();
        }
    };

    vector<thread> threads_a;
    for (int i = 0; i < NUM_FILOSOFOS; i++) threads_a.emplace_back(filosofo_ordem_global, i);
    for (auto& t : threads_a) t.join();

    cout << "Metricas da Solucao A:\n";
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        cout << "Filosofo " << i << " | Refeicoes: " << metricas[i].refeicoes 
             << " | Maior espera: " << fixed << setprecision(2) << metricas[i].maior_espera_ms << " ms\n";
    }

    // --- RESET DAS METRICAS ---
    for (int i = 0; i < NUM_FILOSOFOS; i++) metricas[i] = Metricas();

    // --- SOLUCAO B: SEMAFORO LIMITANDO A 4 FILOSOFOS ---
    cout << "\n=== SOLUCAO B: SEMAFORO LIMITANDO ACESSO ===\n";
    
    Semaforo garcom(NUM_FILOSOFOS - 1); // Permite no maximo 4 filosofos pegarem garfos simultaneamente

    auto filosofo_semaforo = [&](int id) {
        mt19937 rng(id ^ time(nullptr));
        uniform_int_distribution<int> dist_tempo(5, 15);

        int garfo_esq = id;
        int garfo_dir = (id + 1) % NUM_FILOSOFOS;

        for (int i = 0; i < CICLOS_POR_FILOSOFO; i++) {
            this_thread::sleep_for(milliseconds(dist_tempo(rng)));

            auto inicio_espera = high_resolution_clock::now();

            // O semaforo garante que sempre havera pelo menos 1 garfo livre para alguem
            garcom.acquire();
            
            garfos[garfo_esq].lock();
            garfos[garfo_dir].lock();

            auto fim_espera = high_resolution_clock::now();
            duration<double, milli> tempo_espera = fim_espera - inicio_espera;

            metricas[id].maior_espera_ms = max(metricas[id].maior_espera_ms, tempo_espera.count());
            metricas[id].refeicoes++;

            this_thread::sleep_for(milliseconds(dist_tempo(rng)));

            garfos[garfo_dir].unlock();
            garfos[garfo_esq].unlock();
            
            garcom.release();
        }
    };

    vector<thread> threads_b;
    for (int i = 0; i < NUM_FILOSOFOS; i++) threads_b.emplace_back(filosofo_semaforo, i);
    for (auto& t : threads_b) t.join();

    cout << "Metricas da Solucao B:\n";
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        cout << "Filosofo " << i << " | Refeicoes: " << metricas[i].refeicoes 
             << " | Maior espera: " << fixed << setprecision(2) << metricas[i].maior_espera_ms << " ms\n";
    }

    return 0;
}