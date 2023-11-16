// tests the hash table itself using souffle

#include "../src/hashy.h"
#include "../src/souffle.h"

TEST(basic_hash_tests, insert_get) {
    HashTable *ht = hashy_init();
    int *num = malloc(1 * sizeof(int));
    ASSERT_NOT_NULL(num);
    *num = 5;
    hashy_insert(ht, "Hello", num);
    ASSERT_EQ(5, *(int *)(hashy_get(ht, "Hello")));
    hashy_free(ht);
    free(num);
}

TEST(basic_hash_tests, insert_remove) {
    HashTable *ht = hashy_init();
    int *num = malloc(1 * sizeof(int));
    assert(num);
    int *num2 = malloc(1 * sizeof(int));
    assert(num2);
    *num = 5;
    *num2 = 3;
    hashy_insert(ht, "Hello", num);
    hashy_insert(ht, "Hello2", num2);
    ASSERT_EQ(5, *(int *)(hashy_get(ht, "Hello")));
    ASSERT_EQ(3, *(int *)(hashy_get(ht, "Hello2")));
    hashy_remove(ht, "Hello");
    ASSERT_NULL((hashy_get(ht, "Hello")));
    // can't remove twice
    ASSERT_FALSE(hashy_remove(ht, "Hello"));
    ASSERT_EQ(3, *(int *)(hashy_get(ht, "Hello2")));
    ASSERT_TRUE(hashy_remove(ht, "Hello2"));
    ASSERT_FALSE(hashy_remove(ht, "Hello2"));
    hashy_free(ht);
    free(num);
    free(num2);
}

TEST(basic_hash_tests, insert_insert) {
    HashTable *ht = hashy_init();
    int *num = malloc(1 * sizeof(int));
    assert(num);
    int *num2 = malloc(1 * sizeof(int));
    assert(num2);
    *num = 5;
    *num2 = 3;
    hashy_insert(ht, "Hello", num);
    // key already exists
    ASSERT_EQ(1, hashy_insert(ht, "Hello", num2));
    ASSERT_EQ(5, *(int *)hashy_get(ht, "Hello"));
    hashy_free(ht);
    free(num);
    free(num2);
}
