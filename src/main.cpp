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

void producer(void *p) {
    LogAcesso log;
    log.tag = 9999;

    while (true) {
        delay(random(0, 1000));

        if (buffer.add(&log))
            Serial.println("Added!");
        else
            Serial.println("Buffer is full!");
    }
}

void consumer(void *p) {
    LogAcesso log;

    while (true) {
        delay(random(0, 1000));

        buffer.get(&log);
        Serial.printf("Get %llu\n\r", log.tag);

        if (buffer.remove())
            Serial.println("Removed!");
        else
            Serial.println("Buffer is empty!");
    }
}

void setup() {
    Serial.begin(115200);
    buffer.begin("ssc-log-acesso", MAX_LOG_ACCESS_RECORDS);

    xTaskCreatePinnedToCore(
        producer,         /* Function to implement the task */
        "producer",       /* Name of the task */
        2048,             /* Stack size in words */
        NULL,             /* Task input parameter */
        tskIDLE_PRIORITY, /* Priority of the task */
        NULL,             /* Task handle. */
        PRO_CPU_NUM);     /* Core where the task should run */

    xTaskCreatePinnedToCore(
        consumer,         /* Function to implement the task */
        "consumer",       /* Name of the task */
        2048,             /* Stack size in words */
        NULL,             /* Task input parameter */
        tskIDLE_PRIORITY, /* Priority of the task */
        NULL,             /* Task handle. */
        APP_CPU_NUM);     /* Core where the task should run */
}

void loop() {
    delay(500);
    buffer.list(&imprimir_log);
}
