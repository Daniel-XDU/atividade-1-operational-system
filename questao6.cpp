#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <fstream>
#include <iomanip>

// We seek the perfect algorithm, but life's best answers just don't compile.
//By Xaulin_Du_Grau. ;-;


using namespace std;
using namespace std::chrono;

int main() {
    const string NOME_ARQUIVO = "dados_grandes.txt";
    const int TOTAL_NUMEROS = 50000000; // 50 milhoes de inteiros para garantir carga na CPU

    // Geracao do arquivo de teste executado apenas se o arquivo nao existir
    ifstream check_file(NOME_ARQUIVO);
    if (!check_file.is_open()) {
        cout << "Criando arquivo de teste com " << TOTAL_NUMEROS << " inteiros (isso pode levar alguns segundos)...\n";
        ofstream out_file(NOME_ARQUIVO);
        mt19937 rng(42);
        uniform_int_distribution<int> dist(1, 100);
        for (int i = 0; i < TOTAL_NUMEROS; i++) {
            out_file << dist(rng) << "\n";
        }
        out_file.close();
        cout << "Arquivo gerado com sucesso.\n\n";
    } else {
        check_file.close();
    }

    // Leitura do arquivo para a memoria
    // Para medir o speedup real das threads CPU, lemos do disco apenas uma vez.
    cout << "Carregando dados para a memoria...\n";
    vector<int> dados;
    dados.reserve(TOTAL_NUMEROS);
    ifstream in_file(NOME_ARQUIVO);
    int num;
    while (in_file >> num) {
        dados.push_back(num);
    }
    in_file.close();
    cout << "Dados carregados. Iniciando processamento MapReduce...\n\n";

    // Experimento de Speedup
    vector<int> threads_teste = {1, 2, 4, 8};
    double tempo_base = 0.0;

    cout << "--------------------------------------------------------\n";
    cout << setw(5) << "P" << setw(15) << "Soma Global" << setw(15) << "Tempo(ms)" << setw(15) << "Speedup\n";
    cout << "--------------------------------------------------------\n";

    for (int P : threads_teste) {
        long long soma_global = 0;
        unordered_map<int, int> hist_global;
        mutex mtx_reduce;

        auto start_time = high_resolution_clock::now();

        auto map_reduce_worker = [&](int id) {
            // Fase de MAP Variaveis locais, sem lock
            long long soma_local = 0;
            unordered_map<int, int> hist_local;

            size_t chunk_size = dados.size() / P;
            size_t inicio = id * chunk_size;
            size_t fim = (id == P - 1) ? dados.size() : inicio + chunk_size;

            for (size_t i = inicio; i < fim; i++) {
                soma_local += dados[i];
                hist_local[dados[i]]++;
            }

            // Fase de REDUCE Atualizacao global com exclusao mutua
            lock_guard<mutex> lock(mtx_reduce);
            soma_global += soma_local;
            for (const auto& par : hist_local) {
                hist_global[par.first] += par.second;
            }
        };

        vector<thread> pool;
        for (int i = 0; i < P; i++) {
            pool.emplace_back(map_reduce_worker, i);
        }
        for (auto& t : pool) {
            t.join();
        }

        auto end_time = high_resolution_clock::now();
        duration<double, milli> tempo_execucao = end_time - start_time;

        double tempo_ms = tempo_execucao.count();
        double speedup = 1.0;

        if (P == 1) {
            tempo_base = tempo_ms;
        } else {
            speedup = tempo_base / tempo_ms;
        }

        cout << setw(5) << P 
             << setw(15) << soma_global 
             << setw(15) << fixed << setprecision(2) << tempo_ms 
             << setw(14) << speedup << "x\n";
    }
    cout << "--------------------------------------------------------\n";

    return 0;
}