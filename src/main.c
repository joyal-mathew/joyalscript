#include "hashmap.h"

int main() {
    HashMap hm;
    u64 a;

    hashmap_init(&hm);


    hashmap_put(&hm, "Hello", 20);
    hashmap_put(&hm, "This", 31);
    hashmap_put(&hm, "a", 12);
    hashmap_put(&hm, "Helslo", 13);
    hashmap_put(&hm, "a", 14);
    hashmap_put(&hm, "b", 15);
    hashmap_put(&hm, " ", 16);

    CHECK(hashmap_get(&hm, "Hello", &a));
    printf("%llu\n", a);
    CHECK(hashmap_get(&hm, "This", &a));
    printf("%llu\n", a);
    CHECK(hashmap_get(&hm, "a", &a));
    printf("%llu\n", a);
    CHECK(hashmap_get(&hm, "Helslo", &a));
    printf("%llu\n", a);
    CHECK(hashmap_get(&hm, "a", &a));
    printf("%llu\n", a);
    CHECK(hashmap_get(&hm, "b", &a));
    printf("%llu\n", a);
    CHECK(hashmap_get(&hm, " ", &a));
    printf("%llu\n", a);

    hashmap_deinit(&hm);

    return 0;
}
