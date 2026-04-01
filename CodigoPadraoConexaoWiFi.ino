// Inclui as bibliotecas WiFi necessárias 
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
// Inclui biblioteca de persistência de dados
#include <Preferences.h>

// Cria a instância da biblioteca
Preferences preferences; 

/* ========================================================================
Escaneia as redes WiFi se conecta a rede WiFi escolhida e armazena seus dados em uma espécie de arquivo
* ======================================================================== */

/* ========================================================================
 * Cria classe WiFiDataManager para gerenciar as operações de escrita e leitura de memória
 * ======================================================================== */
class WiFiDataManager {
private:
    Preferences preferences;
    const char* namespaceName = "Dados_WiFi"; // Chave do NVS (máx 15 caracteres)
public:
    // Construtor
    WiFiDataManager() {}

    /**
     * @brief Salva as credenciais na memória não-volátil (NVS)
     * @param ssid Referência constante para evitar cópias desnecessárias na Heap (Otimização)
     * @param senha Referência constante para a senha
     */
    void GuardaDadosWiFi(const String& ssid, const String& senha) {
        // Abre o NVS em modo Read/Write (false)
        preferences.begin(namespaceName, false);
        
        preferences.putString("SSID", ssid);
        preferences.putString("Senha", senha);
        
        // Fecha o NVS para liberar recursos do sistema
        preferences.end();
    }

    /**
     * @brief Recupera as credenciais da memória não-volátil (NVS)
     * @param outSsid Referência onde o SSID será escrito (substitui o ponteiro C-style)
     * @param outSenha Referência onde a senha será escrita
     */
    void RecuperaDadosWiFi(String& outSsid, String& outSenha) {
        // Abre o NVS em modo Read-Only (true) - Mais rápido e seguro
        preferences.begin(namespaceName, true);
        
        // O segundo parâmetro ("") é o valor padrão caso a chave não exista
        outSsid = preferences.getString("SSID", "");
        outSenha = preferences.getString("Senha", "");
        
        preferences.end();
    }
};

// Instância global para gerenciar os dados da memória NVS
WiFiDataManager wifiData;

// Declaração prévia (protótipos) das funções
void conectarWiFi(); // Protótipo adicionado para manter corretude em C++

/* ========================================================================
 * Classe responsável por gerenciar o escaneamento, menu e conexão WiFi
 * ======================================================================== */
class ListaEConectaWiFi {
private:
  // ----------------------------------------------------------------------
  // FUNÇÕES PRIVADAS (Uso interno da classe)
  // ----------------------------------------------------------------------

  // Retorna o índice da rede escolhida (int) e recebe o total de redes (parâmetro)
  int listarRedeWiFi(int totalRedes) {
    for (int i = 0; i < totalRedes; i++) {
      Serial.print(i);
      Serial.print(" - ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.println(" dBm)");
    }
    Serial.println("Digite o numero da rede desejada:");
    while (Serial.available() == 0) { }
    int escolha = Serial.parseInt();
    
    // 🗑️ Lemos e descartamos o '\n' (Enter) que sobrou no buffer
    Serial.readStringUntil('\n'); 
    
    return escolha; 
  }

  // Implementação da função ausente: Solicita a senha via Serial
  String solicitarSenhaWiFi() {
    Serial.println("Digite a senha da rede selecionada:");
    while (Serial.available() == 0) { }
    String senha = Serial.readStringUntil('\n');
    senha.trim(); // Remove quebras de linha (\r ou \n) que vêm do terminal Serial
    return senha;
  }

  // Tenta a conexão usando os dados recebidos por parâmetro 
  void StatusEConexaoWiFi(String ssidEscolhido, const String &senhaDigitada) {
    Serial.print("Conectando a: ");
    Serial.println(ssidEscolhido);
    
    // 2. Iniciamos a tentativa de conexão
    WiFi.begin(ssidEscolhido.c_str(), senhaDigitada.c_str());
    int tentativas = 0;
    
    // 3. ESPERA: Aguardamos até 10 segundos (10 * 1000ms)
    // O hardware precisa desse tempo para negociar com o roteador
    while (WiFi.status() != WL_CONNECTED && tentativas < 10) {
      delay(1000);
      Serial.print(".");
      tentativas++;
    }
    
    // 4. VERIFICAÇÃO FINAL: Só checamos o status após o tempo de espera
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[SUCESSO] Conectado!");
      
      // 5. PERSISTÊNCIA: Chamamos a função de guardar dados apenas se conectou
      wifiData.GuardaDadosWiFi(ssidEscolhido, senhaDigitada);
    } else {
      Serial.println("\n[ERRO] Falha na conexao. Reiniciando escolha...");
    }
  }

public:
  // ----------------------------------------------------------------------
  // FUNÇÕES PÚBLICAS (Acessíveis pelo código principal)
  // ----------------------------------------------------------------------
  
  // Construtor vazio (Boa prática para inicialização da classe)
  ListaEConectaWiFi() {}

  // Função dedicada para lidar com o provisionamento e conexão de rede
  void conectarWiFi() {
    // Criamos as variáveis locais para receber os dados salvos
    String redeSalva = "";
    String senhaSalva = "";
    
    // Chamamos a função para preencher essas variáveis usando a instância global
    wifiData.RecuperaDadosWiFi(redeSalva, senhaSalva);

    // Verifica se os valores de nome de rede e senha salva lidos da memória contém informações
    if (redeSalva != "") {
        Serial.println("\n=================================");
        Serial.println("      MENU DE CONEXAO WIFI       ");
        Serial.println("=================================");
        Serial.print("1. Desejo me conectar a ultima rede conectada (");
        Serial.print(redeSalva);
        Serial.println(")");
        Serial.println("2. Desejo escanear as redes wifi e me conectar a uma nova rede");
        Serial.println("=================================");
        Serial.println("Digite a sua escolha (1 ou 2):");

        // Aguarda o usuário digitar algo no Monitor Serial
        while (Serial.available() == 0) { 
          delay(10); 
        }
        
        int escolhaMenu = Serial.parseInt();
        // Lemos e descartamos o '\n' (Enter) que sobrou no buffer
        Serial.readStringUntil('\n'); 

        if (escolhaMenu == 1) {
            // Se escolheu 1, tenta conectar com os dados salvos
            StatusEConexaoWiFi(redeSalva, senhaSalva); 
        } else {
            // Se escolheu 2 (ou digitou qualquer outra coisa), ignora a rede salva.
            Serial.println("\nOpcao 2 selecionada: Partindo para o escaneamento de novas redes...");
        }
      
    }

    // Laço principal de conexão: repetirá enquanto NÃO estiver conectado
    while (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n--- Iniciando Escaneamento ---");
      int numRedes = WiFi.scanNetworks(); // Realiza o scan
      
      if (numRedes == 0) {
        Serial.println("Nenhuma rede encontrada. Tentando novamente...");
      } else {
        // 1. Lista as redes e recebe o índice escolhido
        int escolha = listarRedeWiFi(numRedes); 
        
        // 2. Solicita a senha
        String senha = solicitarSenhaWiFi();
        
        // 3. Tenta efetivar a conexão convertendo o índice de 'escolha' para a String do SSID correspondente
        StatusEConexaoWiFi(WiFi.SSID(escolha), senha);
      }
    }
    
    Serial.println("\n[SISTEMA PRONTO] ESP32 Online!");
    Serial.print("IP obtido: ");
    Serial.println(WiFi.localIP()); // Exibe o endereço IP final
  }
};

ListaEConectaWiFi gerenciadorWiFi;

/* ========================================================================
* ======================================================================== */

/* ========================================================================
Criação de Servidor e Ponto de Acesso: Usar o WiFiAP 🌐 para transformar o ESP32 
em um roteador e o WiFiServer para hospedar uma página de controle onde você pode 
ligar componentes ou ler sensores pelo navegador do celular.
* ======================================================================== */
class PontoDeAcesso {
private:
  // 🔒 Variáveis de uso estritamente interno
  WiFiServer servidor;
  String htmlString;

  // Função interna para entregar o visual
  void PaginaWEB(WiFiClient& cliente) {
    cliente.println(htmlString); 
  }

public:
  // 🔓 Construtor: Inicializa a porta 80 e guarda o HTML de forma segura
  PontoDeAcesso() : servidor(80) {
    htmlString = R"=====(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta http-equiv="refresh" content="1">
  <title>Dashboard | ESP32 Monitor</title>
  <style>
    /* Tema Escuro estilo Grafana/Zabbix */
    body {
      background-color: #111217;
      color: #c8c9ca;
      font-family: 'Inter', 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
      margin: 0;
      padding: 0;
      display: flex;
      flex-direction: column;
      height: 100vh;
    }
    /* Barra Superior de Navegação */
    .navbar {
      background-color: #181b1f;
      border-bottom: 1px solid #2c3235;
      padding: 10px 20px;
      display: flex;
      align-items: center;
      justify-content: space-between;
    }
    .navbar h1 {
      margin: 0;
      font-size: 18px;
      color: #e0e0e0;
      font-weight: 500;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    .status-badge {
      background: rgba(41, 163, 41, 0.1);
      color: #56d64d;
      border: 1px solid #29a329;
      padding: 2px 8px;
      border-radius: 2px;
      font-size: 11px;
      text-transform: uppercase;
      font-weight: bold;
    }
    /* Área Principal do Dashboard */
    .container {
      padding: 20px;
      flex-grow: 1;
      overflow-y: auto;
    }
    /* Layout em Grade para os Cards - 3 colunas fixas para evitar estouro horizontal */
    .dashboard-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 16px;
    }
    /* Estilo dos Painéis (Cards) */
    .panel {
      background-color: #181b1f;
      border: 1px solid #2c3235;
      border-radius: 2px;
      display: flex;
      flex-direction: column;
      position: relative;
      min-height: 140px;
    }
    /* Cores de borda superior para simular Grafana */
    .panel-info    { border-top: 3px solid #3274d9; }
    .panel-success { border-top: 3px solid #29a329; }
    .panel-warn    { border-top: 3px solid #f2cc0c; }
    .panel-purple  { border-top: 3px solid #9c4dc9; }
    
    .panel-header {
      padding: 8px 12px;
      border-bottom: 1px solid #2c3235;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .panel-title {
      font-size: 12px;
      font-weight: 600;
      color: #ccccdc;
      text-transform: uppercase;
      letter-spacing: 0.05em;
    }
    .panel-body {
      flex-grow: 1;
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      padding: 20px;
      text-align: center;
    }
    .panel-value {
      font-size: 36px;
      font-weight: 400;
      color: #ffffff;
      line-height: 1;
      /* Garante que textos longos (ex: SSID) quebrem linha em vez de estourar o card */
      word-break: break-all;
    }
    .panel-unit {
      font-size: 14px;
      color: #8e8f94;
      margin-left: 4px;
    }
    .panel-footer {
      padding: 8px 12px;
      font-size: 11px;
      color: #7b7c82;
      background: rgba(0,0,0,0.1);
    }
    /* Efeito de Hover */
    .panel:hover {
      border-color: #464c54;
      box-shadow: 0 4px 8px rgba(0,0,0,0.3);
    }
    @media (max-width: 900px) {
      .dashboard-grid { grid-template-columns: repeat(2, 1fr); }
    }
    @media (max-width: 600px) {
      .dashboard-grid { grid-template-columns: 1fr; }
      .panel-value { font-size: 28px; }
    }
  </style>
</head>
<body>
  <div class="navbar">
    <h1><span>📊</span> ESP32 Infrastructure Monitor</h1>
    <div class="status-badge">Live</div>
  </div>
  <div class="container">
    <div class="dashboard-grid">
      <div class="panel panel-success">
        <div class="panel-header">
          <span class="panel-title">System Status</span>
          <span>🟢</span>
        </div>
        <div class="panel-body">
          <div class="panel-value">Healthy</div>
        </div>
        <div class="panel-footer">Status: Online</div>
      </div>
      
      <div class="panel panel-success">
        <div class="panel-header">
          <span class="panel-title">System Uptime</span>
          <span>⏱️</span>
        </div>
        <div class="panel-body">
          <div class="panel-value" style="font-size: 28px;">%UPTIME%</div>
        </div>
        <div class="panel-footer">Tempo desde o último boot</div>
      </div>
      
      <div class="panel panel-info">
        <div class="panel-header">
          <span class="panel-title">Network SSID</span>
          <span>📡</span>
        </div>
        <div class="panel-body">
          <div class="panel-value">%SSID%</div>
        </div>
        <div class="panel-footer">Mode: Access Point (AP)</div>
      </div>
      <div class="panel panel-info">
        <div class="panel-header">
          <span class="panel-title">Local IP Address</span>
          <span>🌐</span>
        </div>
        <div class="panel-body">
          <div class="panel-value">%IP%</div>
        </div>
        <div class="panel-footer">Gateway: Port 80</div>
      </div>
      <div class="panel panel-info">
        <div class="panel-header">
          <span class="panel-title">Router SSID (STA)</span>
          <span>🔗</span>
        </div>
        <div class="panel-body">
          <div class="panel-value">%SSID_STA%</div>
        </div>
        <div class="panel-footer">Rede externa conectada</div>
      </div>
      <div class="panel panel-info">
        <div class="panel-header">
          <span class="panel-title">Router IP (STA)</span>
          <span>🛜</span>
        </div>
        <div class="panel-body">
          <div class="panel-value">%IP_STA%</div>
        </div>
        <div class="panel-footer">IP atribuído pelo roteador</div>
      </div>
      <div class="panel panel-purple">
        <div class="panel-header">
          <span class="panel-title">Gateway</span>
          <span>🚪</span>
        </div>
        <div class="panel-body">
          <div class="panel-value">%GATEWAY%</div>
        </div>
        <div class="panel-footer">Endereço do roteador na rede</div>
      </div>
      <div class="panel panel-warn">
        <div class="panel-header">
          <span class="panel-title">Connected Clients</span>
          <span>👥</span>
        </div>
        <div class="panel-body">
          <div class="panel-value">%CLIENTES%<span class="panel-unit">hosts</span></div>
        </div>
        <div class="panel-footer">Real-time device count</div>
      </div>
    </div>
  </div>
</body>
</html>
)=====";
  }

  // Função chamada no setup() para ligar a rede
  void iniciar() {
    String nomeDaRede = "WiFi_" + String(ESP.getChipModel());
    String senhaDaRede = "12345678"; 
    WiFi.softAP(nomeDaRede, senhaDaRede);
    servidor.begin();
  }

  // Função chamada no loop() para verificar os visitantes
  void VerificaClientes() {
    WiFiClient cliente = servidor.available();
    
    if (cliente) {
      String requisicao = cliente.readStringUntil('\n');
      Serial.println("Novo acesso! Pedido: " + requisicao);
      
      cliente.println("HTTP/1.1 200 OK");
      cliente.println("Content-type:text/html");
      cliente.println(); 
      
      // Toda esta lógica para gerar o uptime estava perfeita!
      unsigned long tempoAtual = millis();
      unsigned long segundos = (tempoAtual / 1000) % 60;
      unsigned long minutos = (tempoAtual / 60000) % 60;
      unsigned long horas = (tempoAtual / 3600000) % 24;
      unsigned long dias = tempoAtual / 86400000;

      String tempoUptime = "";
      if (dias > 0) tempoUptime += String(dias) + "d ";
      tempoUptime += String(horas) + "h " + String(minutos) + "m " + String(segundos) + "s";

      // Substitui os placeholders pelos dados reais do ESP32 antes de enviar
      String paginaFinal = htmlString;
      paginaFinal.replace("%SSID%",     WiFi.softAPSSID());
      paginaFinal.replace("%SSID_STA%", WiFi.SSID());
      paginaFinal.replace("%IP%",       WiFi.softAPIP().toString());
      paginaFinal.replace("%IP_STA%",   WiFi.localIP().toString());
      paginaFinal.replace("%GATEWAY%",  WiFi.gatewayIP().toString());
      paginaFinal.replace("%CLIENTES%", String(WiFi.softAPgetStationNum()));
      paginaFinal.replace("%UPTIME%",   tempoUptime); // Injeta a String no novo painel criado!
      
      // Envia a página com os valores já injetados no lugar dos placeholders
      cliente.println(paginaFinal);
      cliente.stop();
      Serial.println("Cliente desconectado.");
    }
  }
};

PontoDeAcesso AcessPoint;
  
// Configura todos os dispositivos e funções usadas
void setup() {
  Serial.begin(115200);
  
  // Define o modo como Station (estação) para buscar redes
  WiFi.mode(WIFI_AP_STA);
  
  // Inicializa o processo de conexão
  gerenciadorWiFi.conectarWiFi();
  
  // Acionamos o controle "iniciar" do nosso objeto.
  // Ele vai configurar o nome, a senha e ligar o servidor por conta própria!
  AcessPoint.iniciar();
}

void loop() {
  // Acionamos o controle "VerificaClientes" continuamente.
  // Ele cuida de escutar a porta 80, ler o pedido e enviar o HTML automaticamente.
  AcessPoint.VerificaClientes();
}
