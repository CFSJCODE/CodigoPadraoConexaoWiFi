# ESP32 WiFi Manager com Dashboard Web

Um projeto completo de gerenciamento de WiFi para ESP32 que combina conectividade de estação (STA), persistência de dados e um servidor web com dashboard em tempo real.

## 🌟 Funcionalidades

- **Gerenciador WiFi Inteligente**: Conecta automaticamente à última rede salva ou permite selecionar uma nova
- **Persistência de Dados**: Armazena credenciais WiFi na memória não-volátil (NVS) do ESP32
- **Access Point (AP)**: Transforma o ESP32 em um roteador WiFi
- **Dashboard Web**: Interface moderna estilo Grafana/Zabbix para monitorar status
- **Modo Dual**: Funciona como Station (STA) + Access Point (AP) simultaneamente
- **Interface Serial**: Menu interativo via Serial para gerenciar conexões

## 📋 Requisitos

### Hardware
- **Placa**: ESP32 (Dev Board ou variantes)
- **Conexão**: Cabo USB para programação e Serial Monitor

### Software
- **Platform IO** ou **Arduino IDE** com suporte ESP32
- **Bibliotecas** (incluídas no SDK ESP32):
  - `WiFi.h` (WiFi.h, WiFiAP.h, WiFiServer.h, etc)
  - `Preferences.h` (NVS - Non-Volatile Storage)

## 🚀 Como Usar

### 1. Upload do Código
```bash
pio run -t upload
```

### 2. Inicialização
- Abra o **Serial Monitor** a **115200 baud**
- O ESP32 fará:
  1. Escaneamento de redes WiFi disponíveis
  2. Listará as redes encontradas com sinal (RSSI)
  3. Solicitará a seleção via entrada numérica

### 3. Conexão WiFi
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
.....
[SUCESSO] Conectado!
IP obtido: 192.168.1.150
```

### 4. Acessar o Dashboard
- **Via WiFi do ESP32** (AP):
  1. Conecte dispo a: `WiFi_ESP32` (senha: `12345678`)
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
- `GuardaDadosWiFi(const String& ssid, const String& senha)`: Salva na NVS
- `RecuperaDadosWiFi(String& outSsid, String& outSenha)`: Recupera da NVS

#### `PontoDeAcesso`
Cria um Access Point e servidor web com dashboard.

**Métodos**:
- `iniciar()`: Ativa o AP e inicia o servidor na porta 80
- `VerificaClientes()`: Processa requisições HTTP dos clientes

**Servidor**: Port 80 (HTTP), modo AP na `192.168.4.1`

### Funções Principal

| Função | Descrição |
|--------|-----------|
| `conectarWiFi()` | Loop principal de conexão WiFi (STA) |
| `listarRedeWiFi(int totalRedes)` | Lista redes encontradas e retorna escolha |
| `solicitarSenhaWiFi()` | Solicita senha via Serial |
| `StatusEConexaoWiFi(String ssid, const String& senha)` | Testa conexão e persiste dados |

## 📊 Dashboard

O dashboard HTML exibe em tempo real:

| Card | Informação |
|------|-----------|
| System Status | Status de saúde do ESP32 |
| Network SSID | Nome da rede do AP |
| Local IP Address | IP do Access Point (192.168.4.1) |
| Router SSID (STA) | Nome da rede externa conectada |
| Router IP (STA) | IP atribuído pela rede externa |
| Gateway | Endereço do roteador |
| Connected Clients | Número de dispositivos conectados ao AP |

**Design**: Tema escuro Grafana/Zabbix, responsivo (desktop, tablet, mobile)

## 💾 Persistência (NVS)

As credenciais são armazenadas no namespace `"Dados_WiFi"` com chaves:
- `"SSID"`: Identificador da rede
- `"Senha"`: Senha da rede

Na primeira execução, o ESP32 escaneia e pede configuração. Nas próximas, usa os dados salvos automaticamente.

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
│   └── main.c           ← Código deste projeto
├── include/             ← Headers customizados
├── lib/                 ← Bibliotecas locais
├── platformio.ini       ← Configuração Platform IO
├── CMakeLists.txt       ← Build configuration
└── README.md            ← Este arquivo
```

## 🔗 Links Úteis

- [Documentação ESP32 WiFi](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html)
- [Preferences (NVS)](https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences)
- [WiFiServer API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)

## 📄 Licença

Código livre para uso pessoal e educacional.

---

**Versão**: 1.0 | **Data**: Abril 2026 | **Platform**: ESP32
