#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <random>
#include <numeric>
#include <cassert>

// We seek the perfect algorithm, but life's best answers just don't compile.
//By Xaulin_Du_Grau. ;-;

using namespace std;

int main() {
    const int M = 10; // Numero de contas
    const int T = 4;  // Numero de threads
    const int TRANSFERENCIAS = 100000; // Transferencias por thread
    const int SALDO_INICIAL = 1000;

    vector<int> contas(M, SALDO_INICIAL);
    vector<mutex> travas(M);

    int saldo_global_esperado = M * SALDO_INICIAL;

    // modo com travas
    cout << "=== INICIANDO SIMULACAO SEGURA COM TRAVAS ===\n";

    auto transferencia_segura = [&](int id) {
        mt19937 rng(id ^ time(nullptr));
        uniform_int_distribution<int> dist_conta(0, M - 1);
        uniform_int_distribution<int> dist_valor(1, 100);

        for (int i = 0; i < TRANSFERENCIAS; i++) {
            int de = dist_conta(rng);
            int para = dist_conta(rng);
            while (de == para) para = dist_conta(rng);
            
            int valor = dist_valor(rng);

            // scoped_lock C++17 garante a ordem de travamento para evitar Deadlock caba é bom dms
            scoped_lock lock(travas[de], travas[para]);
            
            if (contas[de] >= valor) {
                contas[de] -= valor;
                contas[para] += valor;
            }
        }
    };

    vector<thread> threads_seguras;
    for (int i = 0; i < T; i++) threads_seguras.emplace_back(transferencia_segura, i);
    for (auto& t : threads_seguras) t.join();

    int saldo_final_seguro = accumulate(contas.begin(), contas.end(), 0);
    
    cout << "Saldo global esperado: " << saldo_global_esperado << "\n";
    cout << "Saldo global apos simulacao segura: " << saldo_final_seguro << "\n";
    
    // Assercao para provar matematicamente a integridade
    assert(saldo_global_esperado == saldo_final_seguro && "ERRO: O saldo global mudou na simulacao segura!");
    cout << "[OK] Assercao validada. Nenhum centavo foi perdido ou criado.\n\n";

    // modo sem travas
    cout << "=== INICIANDO SIMULACAO INSEGURA SEM TRAVAS ===\n";
    
    // Resetando os saldos
    fill(contas.begin(), contas.end(), SALDO_INICIAL);

    auto transferencia_insegura = [&](int id) {
        mt19937 rng(id ^ time(nullptr));
        uniform_int_distribution<int> dist_conta(0, M - 1);
        uniform_int_distribution<int> dist_valor(1, 100);

        for (int i = 0; i < TRANSFERENCIAS; i++) {
            int de = dist_conta(rng);
            int para = dist_conta(rng);
            while (de == para) para = dist_conta(rng);
            
            int valor = dist_valor(rng);

            // Condicao de corrida severa: lendo e escrevendo sem protecao
            if (contas[de] >= valor) {
                contas[de] -= valor; 
                contas[para] += valor;
            }
        }
    };

    vector<thread> threads_inseguras;
    for (int i = 0; i < T; i++) threads_inseguras.emplace_back(transferencia_insegura, i);
    for (auto& t : threads_inseguras) t.join();

    int saldo_final_inseguro = accumulate(contas.begin(), contas.end(), 0);
    
    cout << "Saldo global esperado: " << saldo_global_esperado << "\n";
    cout << "Saldo global apos simulacao insegura: " << saldo_final_inseguro << "\n";
    
    if (saldo_global_esperado != saldo_final_inseguro) {
        cout << "[FALHA] Condicao de corrida detectada! Dinheiro sumiu ou brotou do nada.\n";
    }

    return 0;
}