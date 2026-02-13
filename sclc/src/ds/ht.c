#include "ds/ht.h"
#include "common.h"
#include "utils.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/*
 * @brief: check if x is prime.
 */
static u32 is_prime(const u32 x) {
  if (x < 2) {
    return -1;
  }
  if (x < 4) {
    return 1;
  }
  if ((x % 2) == 0) {
    return 0;
  }
  for (u32 i = 3; i <= floor(sqrt((double)x)); i += 2) {
    if ((x % i) == 0) {
      return 0;
    }
  }
  return 1;
}

/*
 * @brief: find the next prime number which is greater than x.
 */
static u32 next_prime(u32 x) {
  while (is_prime(x) != 1) {
    x++;
  }
  return x;
}

/*
 * @brief: a simple hash function.
 *
 * @param s: string to hash
 * @param prime: a prime number
 * @param m: size of the hash table
 */
static inline u32 ht_hash(const char *s, const u32 prime, const u32 m) {
  long hash = 0;
  const u32 len_s = strlen(s);
  long p_pow = 1;

  for (u32 i = 0; i < len_s; i++) {
    hash = (hash + s[i] * p_pow) % m;
    p_pow = (p_pow * prime) % m;
  }

  return (u32)hash;
}

/*
 * @brief: implements double-hashing to avoid collissions.
 *
 * @param s: string to hash
 * @param num_buckets: size of the hash table
 * @param attempts: number of attempts it took to hash the current string
 */
static u32 ht_get_hash(const char *s, const u32 num_buckets,
                       const u32 attempt) {
#define HT_PRIME_1 0x21914047
#define HT_PRIME_2 0x1b873593
  const u32 hash_a = ht_hash(s, HT_PRIME_1, num_buckets);
  const u32 hash_b = ht_hash(s, HT_PRIME_2, num_buckets);
  return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
#undef HT_PRIME_1
#undef HT_PRIME_2
}

/*
 * @brief: allocate and initialize a new ht_item and return its memory address.
 *
 * @param k: key string literal.
 * @param v: pointer to the value to be stored.
 * @param value_size: number of bytes to be copied.
 */
static ht_item *ht_new_item(const char *k, const void *v,
                            const u64 value_size) {
  ht_item *i = scu_checked_malloc(sizeof(ht_item));
  i->key = strdup(k);
  i->value = scu_checked_malloc(value_size);
  memcpy(i->value, v, value_size);
  return i;
}

/*
 * @brief: free / delete an existing ht_item.
 *
 * @param i: pointer to the item to be freed.
 */
static void ht_del_item(ht_item *i) {
  free(i->key);
  free(i->value);
  free(i);
}

/*
 * @brief: represents a deleted or NULL hash table item.
 */
static ht_item HT_DELETED_ITEM = {NULL, NULL};

/*
 * @brief : create a new sized hash table
 *
 * @param base_capacity
 * @param value_size: number of bytes the value will occupy.
 */
static ht *ht_new_sized(const u64 base_capacity, const u64 value_size) {
  ht *table = scu_checked_malloc(sizeof(ht));

  table->base_capacity = base_capacity;
  table->capacity = next_prime(table->base_capacity);
  table->count = 0;
  table->items = scu_checked_malloc(table->capacity * sizeof(ht_item *));
  table->value_size = value_size;

  return table;
}

ht *ht_create(const u64 value_size) { return ht_new_sized(53, value_size); }

void ht_destroy(ht *table) {
  if (table == NULL)
    return;

  if (table->items != NULL) {
    for (u64 i = 0; i < table->capacity; i++) {
      ht_item *item = table->items[i];
      if (item != NULL && item != &HT_DELETED_ITEM) { // Skip sentinel
        ht_del_item(item);
      }
    }
    free(table->items);
  }
  free(table);
}

void ht_init(ht *table, const u64 value_size) {
  table->base_capacity = 53;
  table->capacity = next_prime(table->base_capacity);
  table->count = 0;
  table->items = scu_checked_malloc(table->capacity * sizeof(ht_item *));
  table->value_size = value_size;
}

void ht_free(ht *table) {
  if (table == NULL)
    return;

  if (table->items != NULL) {
    for (u64 i = 0; i < table->capacity; i++) {
      ht_item *item = table->items[i];
      if (item != NULL && item != &HT_DELETED_ITEM) { // Skip sentinel
        ht_del_item(item);
      }
    }
    free(table->items);
  }
}

/*
 * @brief: resize an existing hash table to avoid high collission rates and keep
 * storing more key-value pairs.
 *
 * @param table: pointer to an initialized ht struct.
 * @param base_capacity
 */
static void ht_resize(ht *table, const u64 base_capacity) {
  if (base_capacity < 53) {
    return;
  }

  ht *new_ht = ht_new_sized(base_capacity, table->value_size);
  for (u64 i = 0; i < table->capacity; i++) {
    ht_item *item = table->items[i];
    if (item != NULL && item != &HT_DELETED_ITEM) {
      ht_insert(new_ht, item->key, item->value);
    }
  }

  table->base_capacity = new_ht->base_capacity;
  table->count = new_ht->count;

  const u64 tmp_size = table->capacity;
  table->capacity = new_ht->capacity;
  new_ht->capacity = tmp_size;

  ht_item **tmp_items = table->items;
  table->items = new_ht->items;
  new_ht->items = tmp_items;

  ht_destroy(new_ht);
}

/*
 * @brief: wrapper function to make the hash table bigger.
 *
 * @param table: pointer to an initialized ht struct.
 */
static void ht_resize_up(ht *table) {
  const u64 new_size = table->base_capacity * 2;
  ht_resize(table, new_size);
}

/*
 * @brief: wrapper function to make the hash table smaller.
 *
 * @param table: pointer to an initialized ht struct.
 */
static void ht_resize_down(ht *table) {
  const u64 new_size = table->base_capacity / 2;
  ht_resize(table, new_size);
}

void ht_insert(ht *table, const char *key, const void *value) {
  if (table == NULL || key == NULL)
    return;

  const u64 load = table->count * 100 / table->capacity;
  if (load > 70) {
    ht_resize_up(table);
  }

  ht_item *item = ht_new_item(key, value, table->value_size);

  u64 i = 0;
  u64 max_probes = table->capacity;

  u32 index = ht_get_hash(item->key, table->capacity, i);
  ht_item *current = table->items[index];

  while (current != NULL && i < max_probes) {
    if (current != &HT_DELETED_ITEM) {
      if (strcmp(current->key, key) == 0) {
        ht_del_item(current);
        table->items[index] = item;
        return;
      }
    }
    i++;
    index = ht_get_hash(item->key, table->capacity, (int)i);
    current = table->items[index];
  }

  if (i >= max_probes) {
    ht_del_item(item);
    ht_resize_up(table);
    ht_insert(table, key, value);
    return;
  }

  table->items[index] = item;
  table->count++;
}

void *ht_search(ht *table, const char *key) {
  u64 index = ht_get_hash(key, table->capacity, 0);
  ht_item *item = table->items[index];

  u64 i = 1;
  while (item != NULL) {
    if (item != &HT_DELETED_ITEM) {
      if (strcmp(item->key, key) == 0) {
        return item->value;
      }
    }
    index = ht_get_hash(key, table->capacity, i);
    item = table->items[index];
    i++;
  }

  return NULL;
}

void ht_delete(ht *table, const char *key) {
  u64 index = ht_get_hash(key, table->capacity, 0);
  ht_item *item = table->items[index];

  u64 i = 1;
  while (item != NULL) {
    if (item != &HT_DELETED_ITEM) {
      if (strcmp(item->key, key) == 0) {
        ht_del_item(item);
        table->items[index] = &HT_DELETED_ITEM;
        table->count--;

        const u64 load = table->count * 100 / table->capacity;
        if (load < 10)
          ht_resize_down(table);

        return;
      }
    }
    index = ht_get_hash(key, table->capacity, i);
    item = table->items[index];
    i++;
  }
}
