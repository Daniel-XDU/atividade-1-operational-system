#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iomanip>

// We seek the perfect algorithm, but life's best answers just don't compile.
//By Xaulin_Du_Grau. ;-;


using namespace std;
using namespace std::chrono;

class Barreira {
private:
    mutex mtx;
    condition_variable cv;
    int limite;
    int contador;
    int geracao;
    int rodadas_concluidas;
    bool abortado;

public:
    Barreira(int threads) : limite(threads), contador(threads), geracao(0), rodadas_concluidas(0), abortado(false) {}

    // Retorna 'true' se a barreira passou normalmente, 'false' se foi abortada
    bool esperar() {
        unique_lock<mutex> lock(mtx);
        
        // Se a barreira ja foi mandada parar, a thread sai imediatamente
        if (abortado) return false;
        
        int gen_atual = geracao;
        contador--;
        
        if (contador == 0) {
            geracao++;
            contador = limite;
            rodadas_concluidas++;
            cv.notify_all();
            return true;
        } else {
            // A thread dorme ate a geracao mudar OU a barreira ser abortada
            cv.wait(lock, [this, gen_atual]() { return gen_atual != geracao || abortado; });
            return !abortado; // Se acordou por causa do aborto, retorna false
        }
    }

    // Interrompe o processo e acorda todas as threads presas
    void abortar() {
        unique_lock<mutex> lock(mtx);
        abortado = true;
        geracao++; // Muda a geracao para forcar o desperte de quem esta no wait()
        cv.notify_all();
    }

    int get_rodadas() const {
        return rodadas_concluidas;
    }
};

int main() {
    vector<int> tamanhos_equipe = {2, 4, 8, 16};
    const int TEMPO_TESTE_SEGUNDOS = 2; 

    cout << "=== SIMULADOR DE CORRIDA DE REVEZAMENTO (BARREIRAS) ===\n";
    cout << "Testando sincronizacao para diferentes tamanhos de equipe...\n\n";
    cout << "--------------------------------------------------\n";
    cout << setw(10) << "Equipe(K)" << setw(20) << "Rodadas no Teste" << setw(20) << "RPM Estimado\n";
    cout << "--------------------------------------------------\n";

    for (int K : tamanhos_equipe) {
        Barreira barreira(K);

        auto corredor = [&](int id) {
            while (true) {
                // Simula o tempo da corrida com um laco que consome CPU 
                for (int i = 0; i < 50000; i++) {} 

                // Se esperar() retornar false, a barreira foi abortada e o laco eh quebrado
                if (!barreira.esperar()) {
                    break;
                }
            }
        };

        vector<thread> equipe;
        for (int i = 0; i < K; i++) {
            equipe.emplace_back(corredor, i);
        }

        // Aguarda o tempo do teste para esta equipe
        this_thread::sleep_for(seconds(TEMPO_TESTE_SEGUNDOS));
        
        // Dispara o encerramento seguro atrelado ao mutex da barreira
        barreira.abortar();

        // Aguarda a equipe voltar
        for (auto& t : equipe) {
            t.join();
        }

        int rodadas = barreira.get_rodadas();
        long long rpm = ((long long)rodadas * 60) / TEMPO_TESTE_SEGUNDOS;

        cout << setw(10) << K << setw(20) << rodadas << setw(20) << rpm << "\n";
    }
    cout << "--------------------------------------------------\n";

    return 0;
}