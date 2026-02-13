/*
 * ht: contains the hash table struct and its interface.
 */

#ifndef HT_H
#define HT_H

#include "common.h"

/*
 * @struct ht_item: represents an individual item inside a hash table.
 */
typedef struct ht_item {
  char *key;
  void *value;
} ht_item;

/*
 * @struct ht: represents the hash table.
 */
typedef struct ht {
  u64 base_capacity;
  u64 capacity;
  u64 count;

  ht_item **items;
  u64 value_size;
} ht;

/*
 * @brief: create a new hash table.
 *
 * @param value_size: size of the value (void *) of each item.
 *
 * @return: pointer to the newly initialized hash table.
 */
ht *ht_create(const u64 value_size);

/*
 * @brief: destroy an existing hash table.
 *
 * @param table: pointer to an initialized ht (hash table) struct.
 */
void ht_destroy(ht *table);

/*
 * @brief: initialize a new hash table.
 *
 * @param table: pointer to an already allocated ht
 * @param value_size: size of the value (void *) of each item.
 */
void ht_init(ht *table, const u64 value_size);

/*
 * @brief: free all memory associated with a ht
 *
 * @param table: pointer to an already allocated and initialized ht
 */
void ht_free(ht *table);

/*
 * @brief: insert a key value pair into the hash table.
 *
 * @param table: pointer to an initialized ht (hash table) struct.
 * @param key: key string literal.
 * @param value: pointer to the value which is to be stored (will be copied).
 */
void ht_insert(ht *table, const char *key, const void *value);

/*
 * @brief: search for a key inside the hash table and retrieve the stored value.
 *
 * @param table: pointer to an initialized ht (hash table) struct.
 * @param key: key string literal
 */
void *ht_search(ht *table, const char *key);

/*
 * @brief: delete a key-value pair from the hash table.
 *
 * @param table: pointer to an initialized ht (hash table) struct.
 * @param key: key string literal.
 */
void ht_delete(ht *table, const char *key);

#endif // !HT_H
