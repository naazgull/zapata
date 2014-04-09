#include <base/str_map.h>

unsigned int zapata::djb2(string key) {
    unsigned int hash = 5381;

    for (unsigned int i = 0; i < key.length(); i++)
        hash = ((hash << 5) + hash) + (unsigned int) key[i];

    return hash % HASH_SIZE;
}
