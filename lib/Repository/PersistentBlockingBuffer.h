#ifndef PERSISTENT_BLOCKING_BUFFER_H
#define PERSISTENT_BLOCKING_BUFFER_H

#include <Preferences.h>

template <class T>
class PersistentBlockingBuffer {
  private:
    SemaphoreHandle_t mutex;
    Preferences preferences;

    uint8_t head;
    uint8_t next;
    uint8_t capacity;
    uint8_t length;

    char namespace_key[21];
    char index_key[4];

  public:
    PersistentBlockingBuffer();
    ~PersistentBlockingBuffer();

    void begin(const char *namespace_key, const uint8_t capacity);

    void get(T *element);
    bool remove();
    bool add(T *element);

    void list(void (*print_func)(T *));
};

template <class T>
PersistentBlockingBuffer<T>::PersistentBlockingBuffer() : preferences() {
}

template <class T>
PersistentBlockingBuffer<T>::~PersistentBlockingBuffer() {
}

template <class T>
void PersistentBlockingBuffer<T>::begin(const char *namespace_key, const uint8_t capacity) {
    mutex = xSemaphoreCreateMutex();
    assert(mutex);

    this->capacity = capacity;
    strcpy(this->namespace_key, namespace_key);
    memset(this->index_key, 0, sizeof(this->index_key));

    preferences.begin(this->namespace_key, false);
    this->head = preferences.getUChar("head", 0);
    this->next = preferences.getUChar("next", 0);
    this->length = preferences.getUChar("length", 0);
    preferences.end();
}

template <class T>
void PersistentBlockingBuffer<T>::get(T *element) {
    xSemaphoreTake(mutex, portMAX_DELAY);

    if (this->length > 0) {
        preferences.begin(this->namespace_key, true);
        preferences.getBytes(utoa(this->head, this->index_key, 10), element, sizeof(T));
        preferences.end();
    }

    xSemaphoreGive(mutex);
}

template <class T>
bool PersistentBlockingBuffer<T>::remove() {
    bool success = false;

    xSemaphoreTake(mutex, portMAX_DELAY);

    if (this->length > 0) {
        preferences.begin(this->namespace_key, false);

        if (preferences.remove(utoa(this->head, this->index_key, 10))) {
            preferences.putUChar("length", --this->length);
            preferences.putUChar("head", (this->head = (this->head + 1) % this->capacity));
            success = true;
        }

        preferences.end();
    }

    xSemaphoreGive(mutex);

    return success;
}

template <class T>
bool PersistentBlockingBuffer<T>::add(T *element) {
    bool success = false;

    xSemaphoreTake(mutex, portMAX_DELAY);

    if (this->length < this->capacity) {
        preferences.begin(this->namespace_key, false);

        if (preferences.putBytes(utoa(this->next, this->index_key, 10), element, sizeof(T)) == sizeof(T)) {
            preferences.putUChar("length", ++this->length);
            preferences.putUChar("next", (this->next = (this->next + 1) % this->capacity));
            success = true;
        } else {
            preferences.remove(this->index_key);
        }

        preferences.end();
    }

    xSemaphoreGive(mutex);

    return success;
}

template <class T>
void PersistentBlockingBuffer<T>::list(void (*print_func)(T *)) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    preferences.begin(namespace_key, true);
    Serial.printf("Head: %u | Next: %u | Length: %u\n\r", preferences.getUChar("head", 0), preferences.getUChar("next", 0), preferences.getUChar("length", 0));
    preferences.end();
    xSemaphoreGive(mutex);
}

#endif
