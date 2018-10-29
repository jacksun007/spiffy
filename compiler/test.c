#include "jd.h"

struct other
{
    int a;
    int b;
};

enum foo_t
{
    FOO_1,
    FOO_2,
};

FSCONST(type=flag) alphabet
{
    A = 0x1,
    B = 0x2
};

typedef int apple_t;

#define NUM_FLOATS 5

FSSTRUCT() foo {
    int hello;
};

typedef FSSTRUCT() {
    int radius;
    int acidity;
} orange_t;

FSSTRUCT() outer
{
    unsigned a;
    
    // named inner struct with named field
    struct __attribute__((packed)) inner
    {
        int a;
        long const int unsigned volatile b;
        apple_t p, q;
        
        POINTER(repr=block, type=orange_t, when=r==ORANGE)
        POINTER(repr=block, type=struct foo, when=r==TANGERINE)
        orange_t const r;
    } b;
    
    // unnamed struct with named field
    struct
    {
        FIELD(type=enum alphabet)
        volatile char buf[8];
        enum foo_t a, b[3][2][5], c;
    } f;
    
    // unnamed union with anonymous field
    union __attribute__((packed))
    {
        short unsigned s;
        struct
        {
            POINTER(repr=block, type=struct foo)
            char x, y, z;
        } __attribute__((packed)) d;
    };
    
    struct other const e, g[6][9];
    
    VECTOR(name=blah, type=char, size=self.a);
    POINTER(name=d_implicit, repr=block, type=struct foo, expr="crazy");
} __attribute__((packed));

ADDRSPACE(name=block);
VECTOR(type=struct outer, name=foo_table, size=5)

typedef FSCONST(type=enum) {
    I_DIR,
    I_FILE,
    I_NONE = 3,
} inode_type;

typedef FSSUPER(location=0) __attribute__((packed))
{
    int a;
    struct
    {
        inode_type i_type;
        char i_name[16];
    } someone;
   
    POINTER(type=foo_table, repr=block)
    const volatile int b;
    
    FIELD(repr=enum alphabet)
    char flag;
} super_t ;

int main(void)
{
    struct outer x;
    (void)x;
    return 0;
}