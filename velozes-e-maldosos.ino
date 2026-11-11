// ===========================================================
//  VELOZES E MALDOSOS
//  Jogo de corrida de carrinhos para dois jogadores
//  CESAR School - Design
//  Anna Olimpia | Leticia Sabino | Lucas Regis
//  Maria Laura de Mello | Pamella Capistrano
//
//  (c) 2026 Equipe Velozes e Maldosos. Todos os direitos reservados.
//  Este codigo esta publico para leitura e estudo. Copiar, modificar,
//  distribuir ou usar comercialmente so com autorizacao da equipe.
// ===========================================================

// ===== LIGACOES: qual fio vai em qual pino do Arduino =====
// Botoes
const int BTN[2]      = {2, 3};     // botao do jogador 1 (D2) e do jogador 2 (D3)
const int BTN_RESTART = 12;         // botao de restart: faz os carrinhos voltarem E comeca a contagem

// Motores: cada motor usa 2 pinos, um para girar para frente e outro para tras.
// Os pinos 5, 6, 9 e 10 tem o simbolo ~ (PWM), que permite controlar a velocidade.
const int MOTOR1_FRENTE = 5;        // motor 1 para frente  -> A-IA do modulo L9110
const int MOTOR1_TRAS   = 6;        // motor 1 para tras    -> A-IB do modulo L9110
const int MOTOR2_FRENTE = 9;        // motor 2 para frente  -> B-IA do modulo L9110
const int MOTOR2_TRAS   = 10;       // motor 2 para tras    -> B-IB do modulo L9110

// LEDs: sao eles que mostram quem venceu
const int LED[2] = {7, 8};          // LED do jogador 1 (verde) e do jogador 2 (vermelho)

// Buzzer: faz a contagem antes da largada e a musica de vitoria
const int BUZZER = 4;               // perna + do buzzer no D4, perna - no GND

// ===== AJUSTES DO JOGO (valem para os dois) =====
const int CLICKS_TO_WIN = 42;             // quantos cliques para vencer
const unsigned long TEMPO_PARAR_MS = 100; // quanto tempo o motor leva para parar de vez depois de desligado

// ===== AJUSTES DA CONTAGEM (pode mudar estes numeros) =====
const int TOM_PI = 1000;                  // som dos bipes curtos (Hz) - maior = mais agudo
const int TOM_JA = 2000;                  // som do bipe longo do "ja!" (Hz)
const unsigned long DURACAO_PI_MS = 150;  // duracao de cada "pi" curto
const unsigned long DURACAO_JA_MS = 800;  // duracao do "PIIIII" final
const unsigned long INTERVALO_MS = 1000;  // tempo entre um bipe e outro (1 segundo)

// ===== MUSICA DE VITORIA (pode mudar as notas e os tempos) =====
// Notas musicais (frequencia em Hz). PAUSA = silencio.
const int PAUSA = 0;
const int DO6  = 1047;
const int MI6  = 1319;
const int SOL6 = 1568;
const int LA6  = 1760;
const int SI6  = 1976;
const int DO7  = 2093;
const int RE7  = 2349;

// A musica: cada nota com a sua duracao (em milissegundos), na mesma ordem
const int MUSICA_NOTAS[] = {
  // 1. ta-ta-ta-TAAAN!
  DO6, MI6, SOL6, DO7, PAUSA,
  // 2. ha-ha-ha-ha-HA! (risada subindo, tirando onda)
  SOL6, PAUSA, LA6, PAUSA, SI6, PAUSA, DO7, PAUSA, RE7, PAUSA,
  // 3. ta... ta... TAAAAAAN!
  SOL6, PAUSA, SOL6, PAUSA, DO7
};
const int MUSICA_DURACOES[] = {
  120, 120, 120, 350, 100,
  80, 40, 80, 40, 80, 40, 80, 40, 80, 120,
  130, 50, 130, 50, 1000
};
// conta sozinho quantas notas a musica tem (nao precisa mexer)
const int MUSICA_TAMANHO = sizeof(MUSICA_NOTAS) / sizeof(MUSICA_NOTAS[0]);

// ===== AJUSTES DO MOTOR 1 (jogador 1) =====
const unsigned long MOTOR1_PULSE_MS = 40;        // quanto o motor gira a cada clique (ms)
const unsigned long MOTOR1_COMPENSACAO_MS = 10;  // giro extra quando parte do repouso (ms)
const int MOTOR1_SPEED = 130;                    // velocidade, de 0 a 255 (ida e volta)
const float MOTOR1_AJUSTE_VOLTA = 1.0;           // parou antes do inicio? aumente. passou? diminua.

// ===== AJUSTES DO MOTOR 2 (jogador 2) =====
const unsigned long MOTOR2_PULSE_MS = 28;        // quanto o motor gira a cada clique (ms)
const unsigned long MOTOR2_COMPENSACAO_MS = 10;  // giro extra quando parte do repouso (ms)
const int MOTOR2_SPEED = 130;                    // velocidade, de 0 a 255 (ida e volta)
const float MOTOR2_AJUSTE_VOLTA = 1.0;           // parou antes do inicio? aumente. passou? diminua.

// junta os ajustes dos dois motores em listas (nao precisa mexer aqui)
const unsigned long PULSE_MS[2]       = {MOTOR1_PULSE_MS, MOTOR2_PULSE_MS};
const unsigned long COMPENSACAO_MS[2] = {MOTOR1_COMPENSACAO_MS, MOTOR2_COMPENSACAO_MS};
const int MOTOR_SPEED[2]              = {MOTOR1_SPEED, MOTOR2_SPEED};
const float AJUSTE_VOLTA[2]           = {MOTOR1_AJUSTE_VOLTA, MOTOR2_AJUSTE_VOLTA};

// ===== MEMORIA DO JOGO (nao precisa mexer) =====
const int ESPERANDO = 0;              // carrinhos no inicio, esperando o restart para a contagem
const int CORRENDO  = 1;              // corrida acontecendo
const int TERMINOU  = 2;              // alguem venceu, esperando o restart para voltar
const int VOLTANDO  = 3;              // carrinhos voltando para o inicio
int estado = ESPERANDO;

int clicks[2] = {0, 0};               // quantos cliques cada jogador ja deu
unsigned long runUntil[2] = {0, 0};   // ate que momento cada motor deve ficar ligado
unsigned long faltou[2] = {0, 0};     // giro que foi cortado quando a corrida acabou
unsigned long voltaAte[2] = {0, 0};   // ate que momento cada motor deve girar para tras

// Leitura dos botoes sem "tremidas": so aceita o botao quando ele fica estavel
const unsigned long DEBOUNCE_MS = 20;       // tempo que o botao precisa ficar parado para valer
bool estadoEstavel[2] = {HIGH, HIGH};       // estado confirmado do botao (HIGH = solto)
bool ultimaLeitura[2] = {HIGH, HIGH};       // ultima leitura do pino (pode estar tremendo)
unsigned long mudouEm[2] = {0, 0};          // quando a leitura mudou pela ultima vez
bool restartAnterior = HIGH;                // como o restart estava na ultima olhada

// Lista das funcoes que existem mais abaixo (o Arduino precisa conhece-las antes)
void startMotor(int i);
void reverseMotor(int i);
void stopMotor(int i);
void winner(int i, unsigned long now);
void resetGame();
void contagemRegressiva();
void musicaVitoria();

// setup roda UMA vez, quando o Arduino liga
void setup() {
  // botoes como entrada (sem precisar de resistor)
  pinMode(BTN[0], INPUT_PULLUP);
  pinMode(BTN[1], INPUT_PULLUP);
  pinMode(BTN_RESTART, INPUT_PULLUP);

  // as quatro linhas dos motores
  pinMode(MOTOR1_FRENTE, OUTPUT);
  pinMode(MOTOR1_TRAS, OUTPUT);
  pinMode(MOTOR2_FRENTE, OUTPUT);
  pinMode(MOTOR2_TRAS, OUTPUT);

  // LEDs como saida
  pinMode(LED[0], OUTPUT);
  pinMode(LED[1], OUTPUT);

  // buzzer como saida
  pinMode(BUZZER, OUTPUT);

  // comeca com os motores parados
  stopMotor(0);
  stopMotor(1);

  Serial.begin(9600);                 // liga a comunicacao com o Monitor Serial

  // espera o restart para a primeira corrida
  restartAnterior = digitalRead(BTN_RESTART);
  estado = ESPERANDO;
  Serial.println("Carrinhos prontos! Aperte o restart para comecar a contagem.");
}

// loop repete sem parar, milhares de vezes por segundo
void loop() {
  unsigned long now = millis();       // quanto tempo se passou desde que o Arduino ligou

  if (estado == ESPERANDO) {
    // --- carrinhos no inicio: espera alguem APERTAR o restart para comecar ---
    stopMotor(0);
    stopMotor(1);
    bool restart = digitalRead(BTN_RESTART);
    // so comeca quando o botao passa de solto para apertado
    // (se alguem estiver segurando o restart desde a volta, precisa soltar e apertar de novo)
    if (restartAnterior == HIGH && restart == LOW) {
      contagemRegressiva();           // pi... pi... pi... PIIIII e a corrida comeca
    }
    restartAnterior = restart;

  } else if (estado == CORRENDO) {
    // --- olha o botao de cada jogador ---
    for (int i = 0; i < 2; i++) {
      bool leitura = digitalRead(BTN[i]);

      // a leitura mudou? anota a hora (pode ser so tremida)
      if (leitura != ultimaLeitura[i]) {
        mudouEm[i] = now;
        ultimaLeitura[i] = leitura;
      }

      // a leitura ficou parada tempo suficiente e e diferente do estado confirmado?
      if (now - mudouEm[i] >= DEBOUNCE_MS && leitura != estadoEstavel[i]) {
        estadoEstavel[i] = leitura;

        // so conta clique quando o botao passa de solto para APERTADO
        if (estadoEstavel[i] == LOW) {
          clicks[i]++;                // soma 1 clique para esse jogador

          if (now < runUntil[i]) {
            // motor ainda girando: so soma mais um pulso (do ajuste DESTE motor)
            runUntil[i] += PULSE_MS[i];
          } else {
            // motor estava desligado: soma o pulso + compensacao de arranque.
            // Quanto mais tempo parado, maior a compensacao (ate o maximo).
            unsigned long parado = now - runUntil[i];
            if (parado > TEMPO_PARAR_MS) parado = TEMPO_PARAR_MS;
            unsigned long extra = COMPENSACAO_MS[i] * parado / TEMPO_PARAR_MS;
            runUntil[i] = now + PULSE_MS[i] + extra;
          }

          Serial.print("Jogador ");
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.println(clicks[i]);

          // chegou ao numero de cliques? entao venceu: a LUZ decide
          if (clicks[i] >= CLICKS_TO_WIN) {
            winner(i, now);
            break;
          }
        }
      }
    }

    // se alguem acabou de vencer, sai daqui sem religar os motores
    if (estado != CORRENDO) return;

    // --- liga o motor se ainda tem tempo de giro sobrando; senao, desliga ---
    for (int i = 0; i < 2; i++) {
      if (now < runUntil[i]) startMotor(i);
      else stopMotor(i);
    }

  } else if (estado == TERMINOU) {
    // --- corrida acabou: motores parados, LED do vencedor aceso, esperando o restart ---
    stopMotor(0);
    stopMotor(1);

    if (digitalRead(BTN_RESTART) == LOW) {
      Serial.println("Voltando ao inicio...");
      digitalWrite(LED[0], LOW);      // apaga os LEDs
      digitalWrite(LED[1], LOW);
      // cada carrinho volta o que andou, com os ajustes do proprio motor
      for (int i = 0; i < 2; i++) {
        unsigned long tempoVolta = 0;
        if (clicks[i] > 0) tempoVolta = clicks[i] * PULSE_MS[i] + COMPENSACAO_MS[i];
        if (tempoVolta > faltou[i]) tempoVolta -= faltou[i];
        else tempoVolta = 0;
        tempoVolta = (unsigned long)(tempoVolta * AJUSTE_VOLTA[i]);
        voltaAte[i] = now + tempoVolta;
      }
      estado = VOLTANDO;
    }

  } else if (estado == VOLTANDO) {
    // --- cada motor gira para tras ate completar o seu tempo, depois para ---
    bool algumGirando = false;
    for (int i = 0; i < 2; i++) {
      if (now < voltaAte[i]) {
        reverseMotor(i);
        algumGirando = true;
      } else {
        stopMotor(i);
      }
    }
    // os dois chegaram no inicio: zera tudo e espera o restart
    if (!algumGirando) {
      resetGame();
    }
  }
}

// Contagem de largada: "pi... pi... pi... PIIIIII"
// Enquanto ela toca, os botoes dos jogadores nao contam (ninguem sai antes da hora).
void contagemRegressiva() {
  Serial.println("Preparar...");

  // tres bipes curtos: 1... 2... 3...
  for (int n = 1; n <= 3; n++) {
    Serial.print(n);
    Serial.println("...");
    tone(BUZZER, TOM_PI, DURACAO_PI_MS);   // toca o "pi"
    delay(INTERVALO_MS);                   // espera ate o proximo bipe
  }

  // bipe longo e mais agudo: JA!
  // (ele continua tocando sozinho enquanto a corrida ja comeca)
  tone(BUZZER, TOM_JA, DURACAO_JA_MS);
  Serial.println("JA! Aperte o botao!");

  // anota como os botoes estao AGORA: quem estava segurando o botao
  // durante a contagem precisa soltar e apertar de novo para contar
  unsigned long agora = millis();
  for (int i = 0; i < 2; i++) {
    bool leitura = digitalRead(BTN[i]);
    estadoEstavel[i] = leitura;
    ultimaLeitura[i] = leitura;
    mudouEm[i] = agora;
  }

  estado = CORRENDO;                       // valendo!
}

// Musica de vitoria: toca as notas da lista, uma depois da outra
void musicaVitoria() {
  for (int n = 0; n < MUSICA_TAMANHO; n++) {
    if (MUSICA_NOTAS[n] == PAUSA) {
      noTone(BUZZER);                                   // silencio
      delay(MUSICA_DURACOES[n]);
    } else {
      tone(BUZZER, MUSICA_NOTAS[n], MUSICA_DURACOES[n]); // toca a nota
      delay(MUSICA_DURACOES[n] * 1.1);                  // espera a nota + uma pausinha bem curta
    }
  }
  noTone(BUZZER);                                       // garante o silencio no final
}

// Gira o motor para frente (puxa a linha), na velocidade DESTE motor.
// i = 0 e o motor 1, i = 1 e o motor 2.
void startMotor(int i) {
  if (i == 0) {
    analogWrite(MOTOR1_TRAS, 0);
    analogWrite(MOTOR1_FRENTE, MOTOR_SPEED[0]);
  } else {
    analogWrite(MOTOR2_TRAS, 0);
    analogWrite(MOTOR2_FRENTE, MOTOR_SPEED[1]);
  }
}

// Gira o motor para tras (solta a linha), na velocidade DESTE motor
void reverseMotor(int i) {
  if (i == 0) {
    analogWrite(MOTOR1_FRENTE, 0);
    analogWrite(MOTOR1_TRAS, MOTOR_SPEED[0]);
  } else {
    analogWrite(MOTOR2_FRENTE, 0);
    analogWrite(MOTOR2_TRAS, MOTOR_SPEED[1]);
  }
}

// Para o motor
void stopMotor(int i) {
  if (i == 0) {
    analogWrite(MOTOR1_FRENTE, 0);
    analogWrite(MOTOR1_TRAS, 0);
  } else {
    analogWrite(MOTOR2_FRENTE, 0);
    analogWrite(MOTOR2_TRAS, 0);
  }
}

// Chamada quando alguem vence: para os DOIS motores, acende o LED e toca a musica
void winner(int i, unsigned long now) {
  estado = TERMINOU;
  for (int m = 0; m < 2; m++) {
    // anota quanto giro ficou faltando, para a volta descontar
    if (runUntil[m] > now) faltou[m] = runUntil[m] - now;
    else faltou[m] = 0;
    runUntil[m] = 0;
    stopMotor(m);
  }
  digitalWrite(LED[i], HIGH);
  Serial.print(">>> Jogador ");
  Serial.print(i + 1);
  Serial.println(" venceu! Aperte o restart para os carrinhos voltarem.");
  musicaVitoria();                    // ta-ta-ta-TAAAN! ha-ha-ha-HA! TAAAAAN!
}

// Zera tudo e deixa o jogo esperando o restart para a proxima corrida
void resetGame() {
  for (int i = 0; i < 2; i++) {
    clicks[i] = 0;
    runUntil[i] = 0;
    faltou[i] = 0;
    voltaAte[i] = 0;
    digitalWrite(LED[i], LOW);
    stopMotor(i);
  }
  delay(500);
  // anota como o restart esta agora: se alguem ainda estiver segurando,
  // a contagem so comeca depois de soltar e apertar de novo
  restartAnterior = digitalRead(BTN_RESTART);
  estado = ESPERANDO;
  Serial.println("Carrinhos prontos! Aperte o restart para comecar a contagem.");
}
