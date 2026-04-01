# ESP32 WiFi Manager com Dashboard Web

Um projeto completo de gerenciamento de WiFi para ESP32 que combina conectividade de estação (STA), persistência de dados e um servidor web com dashboard em tempo real.

## 🌟 Funcionalidades

- **Gerenciador WiFi Inteligente**: Escaneia redes, conecta automaticamente à última rede salva ou permite selecionar uma nova via menu interativo
- **Persistência de Dados**: Armazena credenciais WiFi na memória não-volátil (NVS) do ESP32
- **Access Point (AP)**: Transforma o ESP32 em um roteador WiFi com servidor web
- **Dashboard Web**: Interface moderna estilo Grafana/Zabbix para monitorar status em tempo real
- **Modo Dual**: Funciona como Station (STA) + Access Point (AP) simultaneamente
- **Interface Serial**: Menu interativo via Serial para gerenciar conexões

## 📋 Requisitos

### Hardware
- **Placa**: ESP32 (Dev Board ou variantes)
- **Conexão**: Cabo USB para programação e Serial Monitor

### Software
- **Platform IO** ou **Arduino IDE** com suporte ESP32
- **Bibliotecas** (incluídas no SDK ESP32):
  - `WiFi.h` e suas variantes (WiFiAP.h, WiFiClient.h, etc.)
  - `Preferences.h` (NVS - Non-Volatile Storage)

## 🚀 Como Usar

### 1. Upload do Código
```bash
pio run -t upload
```

### 2. Inicialização
- Abra o **Serial Monitor** a **115200 baud**
- O ESP32 iniciará no modo `WIFI_AP_STA` e tentará conectar à rede salva, se existir.

### 3. Menu de Conexão WiFi
Se houver credenciais salvas na NVS:
```
=================================
      MENU DE CONEXAO WIFI
=================================
1. Desejo me conectar a ultima rede conectada (NomeDaRedeSalva)
2. Desejo escanear as redes wifi e me conectar a uma nova rede
=================================
Digite a sua escolha (1 ou 2):
```

- **Opção 1**: Conecta automaticamente à rede salva.
- **Opção 2** ou se não houver rede salva: Inicia escaneamento.

### 4. Escaneamento e Conexão
```
--- Iniciando Escaneamento ---
0 - MinhaCasa (-45 dBm)
1 - Vizinho (-67 dBm)
2 - Restaurante (-82 dBm)
Digite o numero da rede desejada:
0
Digite a senha da rede selecionada:
minha_senha_segura
Conectando a: MinhaCasa
..........
[SUCESSO] Conectado!
IP obtido: 192.168.1.150
```

### 5. Acessar o Dashboard
- **Via WiFi do ESP32** (AP):
  1. Conecte-se à rede: `WiFi_<ModeloDoChip>` (senha: `12345678`)
  2. Abra navegador em: `http://192.168.4.1`

- **Via WiFi da Casa** (STA):
  1. Acesse de qualquer dispositivo na mesma rede
  2. Abra: `http://<IP_DO_ESP32>` (ex: `http://192.168.1.150`)

## 🏗️ Arquitetura

### Classes

#### `WiFiDataManager`
Gerencia persistência de credenciais na memória NVS.

```cpp
// Salva credenciais
wifiData.GuardaDadosWiFi("MinhaCasa", "senha123");

// Recupera credenciais
String ssid, senha;
wifiData.RecuperaDadosWiFi(ssid, senha);
```

**Métodos**:
- `GuardaDadosWiFi(const String& ssid, const String& senha)`: Salva SSID e senha na NVS
- `RecuperaDadosWiFi(String& outSsid, String& outSenha)`: Recupera SSID e senha da NVS

#### `ListaEConectaWiFi`
Gerencia escaneamento, menu e conexão WiFi.

**Métodos**:
- `conectarWiFi()`: Loop principal que verifica rede salva, mostra menu e conecta

**Funções Internas**:
- `listarRedeWiFi(int totalRedes)`: Lista redes e retorna índice escolhido
- `solicitarSenhaWiFi()`: Solicita senha via Serial
- `StatusEConexaoWiFi(String ssid, const String& senha)`: Tenta conexão e salva se sucesso

#### `PontoDeAcesso`
Cria um Access Point e servidor web com dashboard.

**Métodos**:
- `iniciar()`: Ativa o AP com nome `WiFi_<ModeloDoChip>` e senha `12345678`, inicia servidor na porta 80
- `VerificaClientes()`: Processa requisições HTTP e envia dashboard

**Servidor**: Porta 80 (HTTP), IP padrão do AP: `192.168.4.1`

## 📊 Dashboard

O dashboard HTML exibe em tempo real (atualiza a cada 1 segundo):

| Card | Informação | Descrição |
|------|------------|-----------|
| System Status | Healthy | Status de saúde do ESP32 |
| System Uptime | 1d 2h 30m 45s | Tempo desde o último boot |
| Network SSID | WiFi_ESP32 | Nome da rede do AP |
| Local IP Address | 192.168.4.1 | IP do Access Point |
| Router SSID (STA) | MinhaCasa | Nome da rede externa conectada |
| Router IP (STA) | 192.168.1.150 | IP atribuído pela rede externa |
| Gateway | 192.168.1.1 | Endereço do roteador |
| Connected Clients | 3 hosts | Número de dispositivos conectados ao AP |

**Design**: Tema escuro estilo Grafana/Zabbix, responsivo (desktop, tablet, mobile), com auto-refresh.

## 💾 Persistência (NVS)

As credenciais são armazenadas no namespace `"Dados_WiFi"` com chaves:
- `"SSID"`: Identificador da rede
- `"Senha"`: Senha da rede

Na primeira execução, o ESP32 escaneia e pede configuração. Nas próximas, oferece menu para usar rede salva ou escanear nova.

## 🔌 Modo Dual WiFi

O ESP32 opera em `WIFI_AP_STA`:

```
┌─────────────────────────────────────┐
│         ESP32 Dual Mode             │
├─────────────────────────────────────┤
│  Station (STA)    │   Access Point  │
│  ↓                │   ↓             │
│  Conecta em:      │   Ativa:        │
│  MinhaCasa        │   WiFi_ESP32    │
│  192.168.1.150    │   192.168.4.1   │
└─────────────────────────────────────┘
```

## ⚙️ Configurações Ajustáveis

### Credenciais do AP (linha ~290)
```cpp
String nomeDaRede = "WiFi_" + String(ESP.getChipModel()); // Nome customizado
String senhaDaRede = "12345678";  // Mude a senha!
```

### Namespace NVS (linha ~34)
```cpp
const char* namespaceName = "Dados_WiFi"; // Máx 15 caracteres
```

### Timeout de Conexão (linha ~213)
```cpp
while (WiFi.status() != WL_CONNECTED && tentativas < 10) {
    delay(1000);  // 10 segundos × 1000ms
    tentativas++;
}
```

## 🐛 Troubleshooting

| Problema | Solução |
|----------|---------|
| Não encontra redes | Verifique antena WiFi, reinicie router |
| "Falha na conexão" | Verifique senha digitada, intensidade de sinal |
| Dashboard não carrega | Verifique IP do ESP32, status da conexão na Serial |
| Credenciais não salvam | Limpe NVS com `partition --erase-all` |
| Muitos pontos no Serial | Timeout normal, ESP32 negocia com roteador |
| Menu não aparece | Rede salva pode estar corrompida, force escaneamento |

## 📝 Notas Técnicas

### Otimizações
- **Referências const**: Para SSID e senha, evita cópias desnecessárias na Heap
- **NVS Read-Only**: Modo `true` mais rápido para leitura
- **Limpeza de buffer**: `Serial.readStringUntil('\n')` remove "\r\n"
- **HTML estático**: Armazenado via `R"=====(...)====="` para economizar RAM

### Segurança
- ⚠️ Senha AP em texto claro (mude conforme necessário)
- ⚠️ Sem autenticação no dashboard (recomendado adicionar)
- ✅ NVS protegido pelo chip (dados não acessíveis via Serial)

## 📦 Estrutura de Arquivos

```
Projeto01/
├── src/
│   └── main.c           ← Código principal (C++)
├── include/             ← Headers customizados
├── lib/                 ← Bibliotecas locais
├── platformio.ini       ← Configuração Platform IO
├── CMakeLists.txt       ← Build configuration
└── README.md            ← Este arquivo
```

## 🔗 Links Úteis

- [Documentação ESP32 WiFi](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html)
- [Biblioteca Preferences](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html)
- [PlatformIO ESP32](https://docs.platformio.org/en/latest/platforms/espressif32.html)

- [Documentação ESP32 WiFi](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html)
- [Preferences (NVS)](https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences)
- [WiFiServer API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)

## 📄 Licença

Código livre para uso pessoal e educacional.

---

**Versão**: 1.0 | **Data**: Abril 2026 | **Platform**: ESP32
