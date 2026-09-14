#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

// We seek the perfect algorithm, but life's best answers just don't compile.
//By Xaulin_Du_Grau. ;-;


using namespace std;
using namespace std::chrono;

int main() {
    mutex recurso_A;
    mutex recurso_B;
    
    atomic<int> progresso_t1(0);
    atomic<int> progresso_t2(0);

    // Ordem Total de Travamento
    cout << "=== EXECUCAO SEGURA ORDEM TOTAL DE AQUISICAO ===\n";
    
    auto thread_segura_1 = [&]() {
        for (int i = 0; i < 5; i++) {
            // O std::scoped_lock (C++17) aplica internamente um algoritmo de prevencao de deadlock
            // garantindo que todos os mutexes passados sejam travados na mesma ordem global.
            scoped_lock lock(recurso_A, recurso_B);
            
            progresso_t1++;
            cout << "[Segura 1] Adquiriu A e B, processando etapa " << i + 1 << "...\n";
            this_thread::sleep_for(milliseconds(50));
        }
    };

    auto thread_segura_2 = [&]() {
        for (int i = 0; i < 5; i++) {
            // Mesmo com os parametros invertidos (B, A), o compilador resolve a ordem total
            scoped_lock lock(recurso_B, recurso_A); 
            
            progresso_t2++;
            cout << "[Segura 2] Adquiriu B e A, processando etapa " << i + 1 << "...\n";
            this_thread::sleep_for(milliseconds(50));
        }
    };

    thread ts1(thread_segura_1);
    thread ts2(thread_segura_2);
    ts1.join();
    ts2.join();
    
    cout << "[OK] Fase segura concluida com sucesso. Nenhum travamento detectado.\n\n";

    // EXECUCAO INSEGURA Deadlock Proposital
    cout << "=== EXECUCAO INSEGURA (DEADLOCK PROPOSITAL) ===\n";
    progresso_t1 = 0;
    progresso_t2 = 0;

    auto thread_insegura_1 = [&]() {
        // Tenta travar A, depois B
        recurso_A.lock();
        cout << "[Insegura 1] Adquiriu Recurso A. Aguardando Recurso B...\n";
        
        // Pausa milimetrica para garantir que a Thread 2 trave o Recurso B
        this_thread::sleep_for(milliseconds(10)); 
        
        recurso_B.lock(); // DEADLOCK: Fica presa aqui
        
        progresso_t1++;
        
        recurso_B.unlock();
        recurso_A.unlock();
    };

    auto thread_insegura_2 = [&]() {
        // Tenta travar B, depois A
        recurso_B.lock();
        cout << "[Insegura 2] Adquiriu Recurso B. Aguardando Recurso A...\n";
        
        this_thread::sleep_for(milliseconds(10));
        
        recurso_A.lock(); // DEADLOCK: Fica presa aqui
        
        progresso_t2++;
        
        recurso_A.unlock();
        recurso_B.unlock();
    };

    // WATCHDOG Monitor de Progresso
    auto watchdog = [&]() {
        const int T_SEGUNDOS = 2;
        int ultimo_t1 = progresso_t1.load();
        int ultimo_t2 = progresso_t2.load();
        
        cout << "[WATCHDOG] Iniciando monitoramento...\n";
        
        while (true) {
            this_thread::sleep_for(seconds(T_SEGUNDOS));
            
            int atual_t1 = progresso_t1.load();
            int atual_t2 = progresso_t2.load();
            
            // Verifica a ausencia de progresso
            if (atual_t1 == ultimo_t1 && atual_t2 == ultimo_t2) {
                cout << "\n======================================================\n";
                cout << "[WATCHDOG ALERTA ERRO FATAL] Ausencia de progresso por " << T_SEGUNDOS << " segundos!\n";
                cout << ">>> RELATORIO DE DIAGNOSTICO <<<\n";
                cout << "Status da Thread 1: Travada (Possui 'recurso_A', aguarda 'recurso_B')\n";
                cout << "Status da Thread 2: Travada (Possui 'recurso_B', aguarda 'recurso_A')\n";
                cout << "Conclusao: CONDICAO DE ESPERA CIRCULAR (DEADLOCK) CONFIRMADA.\n";
                cout << "Acao: Interrompendo processo principal para evitar congelamento da CPU.\n";
                cout << "======================================================\n";
                
                // Finaliza o programa de forma aspera para matar as threads zumbis
                exit(1); 
            }
            
            ultimo_t1 = atual_t1;
            ultimo_t2 = atual_t2;
        }
    };

    thread t1(thread_insegura_1);
    thread t2(thread_insegura_2);
    thread tw(watchdog);

    // Na fase insegura, os joins nunca serao alcancados devido ao exit(1) do Watchdog.
    t1.join();
    t2.join();
    tw.join();

    return 0;
}