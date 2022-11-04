#include "PersistentBlockingBuffer.h"
#include <Arduino.h>

typedef struct log_acesso {
    // Código da tag como um número inteiro
    uint64_t tag;

    // Horário do acesso
    uint8_t dia;
    uint8_t mes;
    uint8_t ano;
    uint8_t hora;
    uint8_t minuto;
    uint8_t segundo;

    // acesso permitido ou acesso negado
    bool permitido;
} LogAcesso;

void imprimir_log(LogAcesso *log) {
    Serial.println(log->tag);
    Serial.println(log->dia);
    Serial.println(log->mes);
    Serial.println(log->ano);
    Serial.println(log->hora);
    Serial.println(log->minuto);
    Serial.println(log->segundo);
    Serial.println(log->permitido);
    Serial.println("------------");
}

PersistentBlockingBuffer<LogAcesso> buffer;

void setup() {
    Serial.begin(115200);
    buffer.begin("ssc-log-acesso", MAX_LOG_ACCESS_RECORDS);
}

bool adicionar = true;

void loop() {
    buffer.list("ssc-log-acesso", MAX_LOG_ACCESS_RECORDS, &imprimir_log);
    Serial.println("=============");

    delay(2000);

    LogAcesso log;
    log.tag = 9999;

    if (adicionar) {
        if (buffer.add(&log)) {
            Serial.println("Added!");
            adicionar = true;
        } else {
            Serial.println("Not added!");
            adicionar = false;
        }
    } else {
        if (buffer.remove()) {
            Serial.println("Removed");
            adicionar = false;
        } else {
            Serial.println("Not removed!");
            adicionar = true;
        }
    }
}
