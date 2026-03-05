#include <atomic>

#if !defined(__clang__) && defined(__GNUC__)
extern "C" {
using I16 = __uint128_t;

I16 __atomic_load_16(const volatile void *mem, int) {
    return __sync_fetch_and_add_16(((I16 *) mem), 0);
}
void __atomic_store_16(volatile void *mem, I16 value, int) {
    __sync_lock_test_and_set_16(((I16 *) mem), value);
}
I16 __atomic_exchange_16(volatile void *mem, I16 value, int) {
    return __sync_lock_test_and_set_16(((I16 *) mem), value);
}
bool __atomic_compare_exchange_16(volatile void *mem, void *expected, I16 desired, bool, int, int) {
    I16 expected_val = *((I16 *) expected);
    *((I16 *) expected) = __sync_val_compare_and_swap_16(((I16 *) mem), expected_val, desired);
    return *((I16 *) expected) == expected_val;
}
I16 __atomic_fetch_add_16(volatile void *mem, I16 value, int) {
    return __sync_fetch_and_add_16(((I16 *) mem), value);
}
I16 __atomic_fetch_sub_16(volatile void *mem, I16 value, int) {
    return __sync_fetch_and_sub_16(((I16 *) mem), value);
}
}
#endif
