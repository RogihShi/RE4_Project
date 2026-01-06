#undef UNICODE
#undef _UNICODE

#pragma comment(lib, "user32.lib")  // Ferramentas de Usuário (Teclado, Mouse, Janelas)
#pragma comment(lib, "psapi.lib")   // Ferramentas de Processos (Ler Memória RAM)
#pragma comment(lib, "shell32.lib") // Ferramentas do Shell (Executar Comandos do Sistema)

#include <iostream>   // Para escrever na tela (printf)
#include <windows.h>  // Funções principais do Windows API
#include <Psapi.h>    // Para pegar informações de módulos (.exe na memória)
#include <tlhelp32.h> // Para tirar "fotos" (Snapshots) da lista de processos
#include <string>     // Para trabalhar com textos de forma fácil
#include <shellapi.h> // Para usar comandos de executar programas

using namespace std;

// ==================================================================================
// FUNÇÃO: PegarPID
// OBJETIVO: Encontrar o ID numérico do jogo (PID) procurando pelo nome "bio4.exe".
// ==================================================================================
int PegarPID(const char* nomeDoProcesso) {
    int pid = 0;
    
    // Tira uma "foto instantânea" de todos os programas abertos no PC agora
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    // Se a foto foi tirada com sucesso...
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 entradaProcesso;
        entradaProcesso.dwSize = sizeof(PROCESSENTRY32); // Define o tamanho da estrutura

        // Pega o primeiro processo da lista da foto
        if (Process32First(snapshot, &entradaProcesso)) {
            do {
                // Compara: O nome desse processo é igual ao que procuramos?
                if (strcmp(entradaProcesso.szExeFile, nomeDoProcesso) == 0) {
                    pid = entradaProcesso.th32ProcessID; // Achou! Guarda o ID.
                    break; // Para de procurar.
                }
            } while (Process32Next(snapshot, &entradaProcesso)); // Vai para o próximo da lista
        }
        CloseHandle(snapshot); // Joga a foto fora para liberar memória
    }
    return pid;
}

// FUNÇÃO PRINCIPAL 
int main()
{
  //O COMANDO MÁGICO (ABRIR STEAM) ---
    // Em vez de abrir o .exe direto (que a Steam bloqueia), mandamos a Steam abrir o ID do jogo.
    // ID 254700 = Resident Evil 4 (Versão 2005/UHD) na loja Steam.
    // O comando "start" faz isso sem travar o nosso código.
    system("start steam://rungameid/254700");

    printf("[2] Aguardando 'bio4.exe' iniciar...\n");
    
    // --- 3. LOOP DE ESPERA (AGUARDANDO O JOGO) ---
    // O código fica preso aqui (travado propositalmente) até o jogo aparecer.
    int PID = 0;
    while (PID == 0) {
        PID = PegarPID("bio4.exe"); // Tenta achar o jogo
        
        // Faz uma animaçãozinha de pontos "..." para mostrar que está vivo
        printf("."); 
        Sleep(1000); // Verifica a cada 1 segundo (para não pesar o processador)
    }
    
    // IMPORTANTE: Espera 4 segundos. 
    // O jogo precisa desse tempo para carregar o mapa e a memória RAM. 
    // Se tentarmos ler antes, dá erro.
    Sleep(4000); 

    // --- 5. CONECTANDO AO JOGO (HOOK) ---
    // Pedimos permissão total ao Windows para mexer na memória desse PID
    HANDLE processoResidentEvil = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION , FALSE, PID);
    HMODULE moduloResidentEvil = NULL;
    DWORD obrigatorio;
    
    // Se não conseguiu abrir (falta de permissão ADM ou erro)
    if(processoResidentEvil == NULL){
        printf("\n[ERRO] não foi possivel abrir o jogo.\n");
        system("pause");
        exit(1);
    }

    // --- 6. MAPEANDO A MEMÓRIA ---
    // Descobre onde o jogo começa na memória RAM (Base Address)
    if (EnumProcessModules(processoResidentEvil, &moduloResidentEvil, sizeof(moduloResidentEvil), &obrigatorio)) {
        MODULEINFO informacoesDoModulo;

        if(GetModuleInformation(processoResidentEvil, moduloResidentEvil, &informacoesDoModulo, sizeof(informacoesDoModulo))) {
            
            DWORD memoriaBase = (DWORD)informacoesDoModulo.lpBaseOfDll;
            
            // --- CÁLCULO DOS ENDEREÇOS (OFFSETS) ---
            // Usamos os endereços que achamos no Cheat Engine + a Base da memória
            DWORD enderecoPonteiroHandgun = memoriaBase + 0x00870FE8; // Ponteiro da Arma
            DWORD enderecoDinheiro = memoriaBase + 0x0085F708;        // Endereço do Dinheiro
            
            DWORD valorSegundaPosicao;

            // Lê o ponteiro para descobrir onde a arma está realmente
            if (ReadProcessMemory(processoResidentEvil, (LPCVOID)enderecoPonteiroHandgun, &valorSegundaPosicao, sizeof(valorSegundaPosicao), NULL)) {
                
                // Matemática de Ponteiro: Valor Lido + Offset 0x13 = Endereço da Bala
                int* ptrHandgun = (int *)valorSegundaPosicao;
                ptrHandgun = (int *)((char *)ptrHandgun + 0x13); 
                
                // Toca dois Bips sonoros para avisar: "Tudo pronto, pode jogar!"
                Beep(1000, 100); Beep(2000, 100);
                
                printf("\n--- TUDO PRONTO! BOM JOGO ---\n");
                printf("[AUTOMATICO] Municao Infinita: ATIVA\n");
                printf("[SENHA] Digite 'MONEY' no jogo para ganhar dinheiro.\n");
                printf("(Pode minimizar esta janela)\n");

                // Configurações do Cheat
                DWORD valorMunicaoHandgun = 3360826880; // Valor especial (Munição Infinita)
                int estagioSenha = 0; // Controle da senha M-O-N-E-Y

                // --- 7. LOOP INFINITO (O MOTOR DO HACK) ---
                while (1) {
                    // SEGURANÇA: Se o jogo fechar (PID sumir), o Launcher fecha junto.
                    if (PegarPID("bio4.exe") == 0) return 0;

                    // CHEAT A: MUNIÇÃO
                    // Escreve na memória o tempo todo
                    WriteProcessMemory(processoResidentEvil, ptrHandgun, &valorMunicaoHandgun, sizeof(valorMunicaoHandgun), NULL);
                    
                    // CHEAT B: DINHEIRO (DIGITAÇÃO)
                    // GetAsyncKeyState verifica o teclado mesmo dentro do jogo.
                    // O '& 1' garante que só pegue o clique inicial.
                    
                    // Sequência Lógica:
                    if (GetAsyncKeyState('M') & 1) estagioSenha = 1; // Começou com M...
                    else if (GetAsyncKeyState('O') & 1) { 
                        if (estagioSenha == 1) estagioSenha = 2; else estagioSenha = 0; // Veio O depois do M?
                    }
                    else if (GetAsyncKeyState('N') & 1) { 
                        if (estagioSenha == 2) estagioSenha = 3; else estagioSenha = 0; 
                    }
                    else if (GetAsyncKeyState('E') & 1) { 
                        if (estagioSenha == 3) estagioSenha = 4; else estagioSenha = 0; 
                    }
                    else if (GetAsyncKeyState('Y') & 1) {
                        // Se chegou no Y e estava no estágio 4 (MONEY completo)
                        if (estagioSenha == 4) {
                            DWORD dinheiroAtual = 0;
                            // 1. Lê dinheiro atual
                            ReadProcessMemory(processoResidentEvil, (LPCVOID)enderecoDinheiro, &dinheiroAtual, sizeof(dinheiroAtual), NULL);
                            // 2. Soma 100k
                            DWORD novoDinheiro = dinheiroAtual + 100000;
                            // 3. Salva
                            WriteProcessMemory(processoResidentEvil, (LPVOID)enderecoDinheiro, &novoDinheiro, sizeof(novoDinheiro), NULL);
                            
                            Beep(750, 200); // Som de sucesso
                            estagioSenha = 0; // Reseta senha
                        } else estagioSenha = 0;
                    }
                    Sleep(100); // Pausa leve para não travar o PC
                }
            }
        }
    }
    return 0;
}