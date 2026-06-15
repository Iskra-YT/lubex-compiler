#include <gtest/gtest.h>
#include <cstring>

extern "C" {
    #include "lbxmem.h"
    #include "lbxint.h"
    #include "lbxstr.h"
    #include "lbxvoid.h"
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

TEST_F(RuntimeTest, Strlen) {
    ASSERT_EQ(__R_strlen(""), 0);
    ASSERT_EQ(__R_strlen("a"), 1);
    ASSERT_EQ(__R_strlen("hello"), 5);
    ASSERT_EQ(__R_strlen("Hello, World!"), 13);
}

TEST_F(RuntimeTest, MultipleAllocations) {
    const int NUM_ALLOCS = 20;
    void* ptrs[NUM_ALLOCS];

    for (int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = _BI_malloc(8);
        ASSERT_NE(ptrs[i], nullptr);
    }

    for (int i = 0; i < NUM_ALLOCS; i++) {
        for (int j = i + 1; j < NUM_ALLOCS; j++) {
            ASSERT_NE(ptrs[i], ptrs[j]);
        }
    }

    for (int i = 0; i < NUM_ALLOCS; i++) {
        __R_memset(ptrs[i], (i % 256), 8);
    }

    for (int i = 0; i < NUM_ALLOCS; i++) {
        char expected = (i % 256);
        ASSERT_EQ(*(char*)ptrs[i], expected);
    }
}

TEST_F(RuntimeTest, LargeAllocation) {
    void* ptr = _BI_malloc(200);
    ASSERT_NE(ptr, nullptr);

    __R_memset(ptr, 0xAB, 200);

    ASSERT_EQ(((unsigned char*)ptr)[0], 0xAB);
    ASSERT_EQ(((unsigned char*)ptr)[199], 0xAB);
}

TEST_F(RuntimeTest, NumberDivideByOne) {
    _BI_Number a, b;
    _BI_Number_init(&a, 42.0);
    _BI_Number_init(&b, 1.0);

    _BI_Number* result = _BI_Number_divide(&a, &b);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->value, 42.0);
}

TEST_F(RuntimeTest, NumberSubtractToNegative) {
    _BI_Number a, b;
    _BI_Number_init(&a, 5.0);
    _BI_Number_init(&b, 10.0);

    _BI_Number* result = _BI_Number_subtract(&a, &b);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->value, -5.0);
}

TEST_F(RuntimeTest, VoidInit) {
    _BI_Void v;
    _BI_Void* result = _BI_Void_init(&v);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result, &v);
}

TEST_F(RuntimeTest, StringEmpty) {
    _BI_String s;
    _BI_String_init(&s, (char*)"");
    ASSERT_EQ(s.size, 0);
    ASSERT_STREQ(s.data, "");
}

TEST_F(RuntimeTest, StringLonger) {
    _BI_String s;
    const char* longStr = "This is a longer string for testing purposes";
    _BI_String_init(&s, (char*)longStr);
    ASSERT_EQ(s.size, (long)std::strlen(longStr));
    ASSERT_STREQ(s.data, longStr);
}

TEST_F(RuntimeTest, NumberMultiplyByZero) {
    _BI_Number a, b;
    _BI_Number_init(&a, 123.456);
    _BI_Number_init(&b, 0.0);

    _BI_Number* result = _BI_Number_multiply(&a, &b);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->value, 0.0);
}

TEST_F(RuntimeTest, MemsetLarge) {
    const size_t SIZE = 256;
    char buffer[SIZE];
    __R_memset(buffer, 0xFF, SIZE);

    for (size_t i = 0; i < SIZE; i++) {
        ASSERT_EQ((unsigned char)buffer[i], 0xFF);
    }
}

TEST_F(RuntimeTest, MemcpyOverlap) {
    char src[20];
    __R_memset(src, 'B', 20);

    char dst[20];
    __R_memset(dst, 'A', 20);
    __R_memcpy(dst, src, 20);

    for (int i = 0; i < 20; i++) {
        ASSERT_EQ(dst[i], 'B');
    }
}
