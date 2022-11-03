#ifndef PERSISTENT_BLOCKING_BUFFER_H
#define PERSISTENT_BLOCKING_BUFFER_H

#include <Preferences.h>

template <class T>
class PersistentBlockingBuffer {
  private:
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

    static void list(const char *namespace_key, const uint8_t capacity, void (*print_func)(T *));
};

template <class T>
PersistentBlockingBuffer<T>::PersistentBlockingBuffer() : preferences() {
}

template <class T>
PersistentBlockingBuffer<T>::~PersistentBlockingBuffer() {
}

template <class T>
void PersistentBlockingBuffer<T>::begin(const char *namespace_key, const uint8_t capacity) {
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
    if (this->length > 0) {
        preferences.begin(this->namespace_key, true);
        preferences.getBytes(utoa(this->head, this->index_key, 10), element, sizeof(T));
        preferences.end();
    }
}

template <class T>
bool PersistentBlockingBuffer<T>::remove() {
    bool success = false;

    if (this->length > 0) {
        preferences.begin(this->namespace_key, false);

        if (preferences.remove(utoa(this->head, this->index_key, 10))) {
            preferences.putUChar("length", --this->length);
            preferences.putUChar("head", (this->head = (this->head + 1) % this->capacity));
            success = true;
        }

        preferences.end();
    }

    return success;
}

template <class T>
bool PersistentBlockingBuffer<T>::add(T *element) {
    bool success = false;

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

    return success;
}

template <class T>
void PersistentBlockingBuffer<T>::list(const char *namespace_key, const uint8_t capacity, void (*print_func)(T *)) {
    Preferences prefs;
    char index_key[4] = {'\0'};
    T buffer;

    prefs.begin(namespace_key, true);

    Serial.printf("Head: %u | Next: %u | Length: %u\n\r", prefs.getUChar("head", 0), prefs.getUChar("next", 0), prefs.getUChar("length", 0));

    // for (uint8_t i = 0; i < capacity; i++) {
    //     if (prefs.isKey(utoa(i, index_key, 10))) {
    //         prefs.getBytes(index_key, &buffer, sizeof(T));

    //         Serial.printf("Key: %u\n\r", i);
    //         print_func(&buffer);
    //     } else {
    //         Serial.printf("Empty Key: %u\n\r", i);
    //     }
    // }

    prefs.end();
}

#endif
