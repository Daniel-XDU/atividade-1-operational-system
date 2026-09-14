# Relatório de Sistemas Operacionais

## Guia de Execução e Pré-requisitos
Os códigos deste repositório utilizam recursos modernos do C++ (como `std::scoped_lock` e lambdas) e a biblioteca de concorrência POSIX/C++11. Para compilar sem erros de versão, é necessário um compilador GCC atualizado com suporte à flag `-std=c++23`.

### Como instalar a versão mais recente do g++ (Windows)
Se estiver avaliando no Windows, a forma mais limpa de obter a versão mais recente do compilador GCC é via MSYS2:
1. Baixe o instalador no site oficial: [msys2.org](https://www.msys2.org/).
2. Após a instalação, abra o terminal do MSYS2 (UCRT64) e execute o comando:
   `pacman -S mingw-w64-ucrt-x86_64-gcc`
3. Confirme a instalação pressionando `Y`.
4. Adicione o caminho `C:\msys64\ucrt64\bin` nas Variáveis de Ambiente do Windows (Path).
5. Reinicie o terminal e verifique com `g++ --version`.

*(Nota para Linux/Ubuntu: Basta rodar `sudo apt update && sudo apt install g++`)*

### Padrão de Compilação
Para compilar e executar qualquer uma das 10 questões no PowerShell/CMD, utilize o seguinte formato (a flag `-pthread` é obrigatória para lincagem de threads, mesmo no C++ moderno):

**Para compilar a Questão 1 (C):**
```bash
cls; gcc questao1.c -pthread; ./a

```bash
cls; g++ questao(numero_da_questão).cpp -std=c++23 -pthread; ./a