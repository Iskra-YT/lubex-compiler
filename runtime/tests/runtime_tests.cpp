#include <gtest/gtest.h>

extern "C" {
    #include "lbxmem.h"
    #include "lbxint.h"
    #include "lbxstr.h"
    HEAP_BLOCK* __R_mainHeap;
}

class RuntimeTest : public ::testing::Test {
protected:
    void SetUp() override {
        __R_mainHeap = __R_malloc_init();
    }

    void TearDown() override {
        // In a real scenario, we might want to free the mmap'ed memory,
        // but for tests it's probably fine as the process will exit.
    }
};

TEST_F(RuntimeTest, MallocBasic) {
    void* ptr1 = _BI_malloc(10);
    ASSERT_NE(ptr1, nullptr);
    
    void* ptr2 = _BI_malloc(20);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr1, ptr2);
}

TEST_F(RuntimeTest, MemsetMemcpy) {
    char buffer[10];
    __R_memset(buffer, 'A', 10);
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(buffer[i], 'A');
    }

    char dest[10];
    __R_memcpy(dest, buffer, 10);
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(dest[i], 'A');
    }
}

TEST_F(RuntimeTest, NumberOperations) {
    _BI_Number a, b;
    _BI_Number_init(&a, 10.5);
    _BI_Number_init(&b, 20.5);

    _BI_Number* c = _BI_Number_add(&a, &b);
    ASSERT_EQ(c->value, 31.0);

    _BI_Number* d = _BI_Number_subtract(&b, &a);
    ASSERT_EQ(d->value, 10.0);

    _BI_Number* e = _BI_Number_multiply(&a, &b);
    ASSERT_EQ(e->value, 10.5 * 20.5);

    _BI_Number* f = _BI_Number_divide(&b, &a);
    ASSERT_NEAR(f->value, 20.5 / 10.5, 0.000001);
}

TEST_F(RuntimeTest, StringInit) {
    _BI_String s;
    _BI_String_init(&s, (char*)"Hello");
    ASSERT_EQ(s.size, 5);
    ASSERT_STREQ(s.data, "Hello");
}
