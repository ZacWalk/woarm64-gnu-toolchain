#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <omp.h>
#include <setjmp.h>
#include <stdint.h>

#define UNICODE 1
#define _UNICODE 1

#include <windows.h>

#define STACK_MAX 100

static const uint32_t FNV_PRIME_32 = 16777619u;
static const uint32_t OFFSET_BASIS_32 = 2166136261u;

// simple fnv like hash.
// Good enough to guarantee uniqueness.
static uint32_t fnv1a(uint32_t hash, const char *data)
{
    const uint8_t *p = (const uint8_t *)(data);

    while (*p != 0)
    {
        hash ^= *p;
        hash *= FNV_PRIME_32;
        p += 1;
    }
    return hash;
}

// Test if large text is stored and addressed correctly.
// Basically hash a lot of strings and check the result.
static uint32_t hash_big_text()
{
    uint32_t hash = OFFSET_BASIS_32;

    // Print out more than one page of text.
    hash = fnv1a(hash, "The Commodore 64, also known as the C64, is an 8-bit home");
    hash = fnv1a(hash, "computer introduced in January 1982 by Commodore");
    hash = fnv1a(hash, "International (first shown at the Consumer Electronics");
    hash = fnv1a(hash, "Show, January 7-10, 1982, in Las Vegas). It has been");
    hash = fnv1a(hash, "listed in the Guinness World Records as the");
    hash = fnv1a(hash, "highest-selling single computer model of all time, with");
    hash = fnv1a(hash, "independent estimates placing the number sold between");
    hash = fnv1a(hash, "12.5 and 17 million units. Volume production started in");
    hash = fnv1a(hash, "early 1982, marketing in August for US$595 (equivalent to");
    hash = fnv1a(hash, "$1,800 in 2022). Preceded by the VIC-20 and Commodore");
    hash = fnv1a(hash, "PET, the C64 took its name from its 64 kilobytes (65,536");
    hash = fnv1a(hash, "bytes) of RAM. With support for multicolor sprites and a");
    hash = fnv1a(hash, "custom chip for waveform generation, the C64 could create");
    hash = fnv1a(hash, "superior visuals and audio compared to systems without");
    hash = fnv1a(hash, "such custom hardware.");
    hash = fnv1a(hash, "");
    hash = fnv1a(hash, "The C64 dominated the low-end computer market (except in");
    hash = fnv1a(hash, "the UK and Japan, lasting only about six months in Japan)");
    hash = fnv1a(hash, "for most of the later years of the 1980s. For a");
    hash = fnv1a(hash, "substantial period (1983-1986), the C64 had between 30%");
    hash = fnv1a(hash, "and 40% share of the US market and two million units sold");
    hash = fnv1a(hash, "per year, outselling IBM PC compatibles, Apple computers,");
    hash = fnv1a(hash, "and the Atari 8-bit family of computers. Sam Tramiel, a");
    hash = fnv1a(hash, "later Atari president and the son of Commodore's founder,");
    hash = fnv1a(hash, "said in a 1989 interview, When I was at Commodore we");
    hash = fnv1a(hash, "were building 400,000 C64s a month for a couple of");
    hash = fnv1a(hash, "years. In the UK market, the C64 faced competition from");
    hash = fnv1a(hash, "the BBC Micro, the ZX Spectrum, and later the Amstrad CPC");
    hash = fnv1a(hash, "464. but the C64 was still the second-most-popular");
    hash = fnv1a(hash, "computer in the UK after the ZX Spectrum. The Commodore");
    hash = fnv1a(hash, "64 failed to make any impact in Japan, as their market");
    hash = fnv1a(hash, "was dominated by Japanese computers, such as the NEC");
    hash = fnv1a(hash, "PC-8801, Sharp X1, Fujitsu FM-7, and MSX.");
    hash = fnv1a(hash, "");
    hash = fnv1a(hash, "Part of the Commodore 64's success was its sale in");
    hash = fnv1a(hash, "regular retail stores instead of only electronics or");
    hash = fnv1a(hash, "computer hobbyist specialty stores. Commodore produced");
    hash = fnv1a(hash, "many of its parts in-house to control costs, including");
    hash = fnv1a(hash, "custom integrated circuit chips from MOS Technology. In");
    hash = fnv1a(hash, "the United States, it has been compared to the Ford Model");
    hash = fnv1a(hash, "T automobile for its role in bringing a new technology to");
    hash = fnv1a(hash, "middle-class households via creative and affordable");
    hash = fnv1a(hash, "mass-production. Approximately 10,000 commercial software");
    hash = fnv1a(hash, "titles have been made for the Commodore 64, including");
    hash = fnv1a(hash, "development tools, office productivity applications, and");
    hash = fnv1a(hash, "video games. C64 emulators allow anyone with a modern");
    hash = fnv1a(hash, "computer, or a compatible video game console, to run");
    hash = fnv1a(hash, "these programs today. The C64 is also credited with");
    hash = fnv1a(hash, "popularizing the computer demoscene and is still used");
    hash = fnv1a(hash, "today by some computer hobbyists. In 2011, 17 years after");
    hash = fnv1a(hash, "it was taken off the market, research showed that brand");
    hash = fnv1a(hash, "recognition for the model was still at 87%.");

    return hash;
}

#define MAX_SAMPLES 0x1000000

// There were problems with BSS sections previously.
// Check we can read and write from them.
static int check_bss()
{
    static uint8_t values[MAX_SAMPLES];
    static uint8_t sorted[MAX_SAMPLES];
    static uint8_t ref[MAX_SAMPLES];

    memset(values, 22, sizeof(values));
    memset(sorted, 33, sizeof(sorted));
    memset(ref, 44, sizeof(ref));

    for (int i = 0; i < MAX_SAMPLES; i++)
    {
        if (values[i] != 22)
            return 0;
        if (sorted[i] != 33)
            return 0;
        if (ref[i] != 44)
            return 0;
    }

    return 1;
}

/*
   Tests stack probing
*/

#define BIG_STACK_ALLOC_SIZE 300000

void write_to_big_stack_alloc(char *stack_data)
{
    stack_data[0] = 18;
    stack_data[BIG_STACK_ALLOC_SIZE - 1] = 19;
}

int check_big_stack_allocation()
{
    char stack_data[BIG_STACK_ALLOC_SIZE];
    write_to_big_stack_alloc(stack_data);
    return stack_data[BIG_STACK_ALLOC_SIZE - 1] == 19;
}

/*
   Tests using the function imported from a DLL
   IMPORT_API must be defined externally
*/

// Declare imported function.
// These functions were exported from the dll using __declspec( dllexport )
__declspec(dllimport) int __cdecl add_c_export(int a, int b);
__declspec(dllimport) int __stdcall add_std_export(int a, int b);

// These functions were exported via a def file.
int __cdecl add_c_def(int a, int b);
int __stdcall add_std_def(int a, int b);

// call a pointer to a function
int test_func_pointer(int __cdecl (*f)(int, int))
{
    return f(6, 23);
}

// Test calling a DLL with methods exported in different ways
// Return a number representing each check to make
// diagnosis easier
int check_dll()
{
    if (add_c_export(7, 3) != 10)
        return 1;
    if (add_std_export(7, 3) != 10)
        return 2;
    if (add_c_def(7, 3) != 10)
        return 3;
    if (add_std_def(7, 3) != 10)
        return 4;
    if (test_func_pointer(add_c_export) != 29)
        return 5;
    if (test_func_pointer(add_c_def) != 29)
        return 6;
    return 0;
}

double nested_add_squares(double a, double b)
{
    double square(double z) { return z * z; }
    return square(a) + square(b);
}

int walk_stack_rtl()
{
    CONTEXT ctx;
    memset(&ctx, 0, sizeof(CONTEXT));
    ctx.ContextFlags = CONTEXT_ALL;
    RtlCaptureContext(&ctx);

    HANDLE hp = GetCurrentProcess();
    HANDLE ht = GetCurrentThread();

    UNWIND_HISTORY_TABLE ms_history;
    DISPATCHER_CONTEXT disp_context;
    ULONG64 ip;

    disp_context.ContextRecord = &ctx;
    disp_context.HistoryTable = &ms_history;

    int i;

    for (i = 0; i < STACK_MAX; i++)
    {
#ifdef _M_AMD64
        ip = ctx.Rip;
#elif defined(_M_ARM64)
        ip = ctx.Pc;
#endif

        disp_context.ControlPc = ip;
        disp_context.FunctionEntry = RtlLookupFunctionEntry(ip, &disp_context.ImageBase, &ms_history);

        if (!disp_context.FunctionEntry)
            return i;

        disp_context.LanguageHandler = RtlVirtualUnwind(0, disp_context.ImageBase, ip,
                                                        disp_context.FunctionEntry, &ctx,
                                                        &disp_context.HandlerData,
                                                        &disp_context.EstablisherFrame, NULL);

        if (ip == 0)
            return i;
    }

    return i;
}

int level4(void)
{
    return walk_stack_rtl();
}
int level3(void) { level4(); }
int level2(void) { level3(); }
int level1(void) { level2(); }

int count_stack_frames()
{
    return level1();
}

static jmp_buf env;

int jumper(int val)
{
    longjmp(env, val);
}

int sjlj_test()
{
    int val = setjmp(env);
    int result = 1;

    if (val == 0)
    {
        result += 3;
        jumper(99);
    }
    else
    {
        result += 11;
    }

    return result;
}

/*
   Tests reading double from a string
*/

int sscanf_double()
{
    double v[3];
    sscanf("0.0 1.0 0.7", "%lf %lf %lf", v, v + 1, v + 2);
    printf("%lf %lf %lf\n", v[0], v[1], v[2]);

    if (v[0] != 0. || v[1] != 1. || v[2] != 0.7)
        return 0;

    return 1;
}

/*
   Tests data in a structure is addressed correctrly
*/

struct aaa
{
    int a;
    int b;
    int space[2000];
    int c;
    int d;
};

struct bbb
{
    int space[2000];
    int a;
    int b;
    int c;
    int d;
};

void copy_parts(struct aaa *a, struct bbb *b)
{
    b->a = a->a;
    b->b = a->b;
    b->c = a->c;
    b->d = a->d;
}

// This checks elements in a struct
// are accessed correctly by copying across values.
int check_struct()
{
    struct aaa a = {11, 22, {0}, 33, 44};
    struct bbb *b = (struct bbb *)malloc(sizeof(struct bbb));

    copy_parts(&a, b);

    // check all values are the same
    int success = b->a == 11 &&
                  b->b == 22 &&
                  b->c == 33 &&
                  b->d == 44;

    return success;
}

static void va_list_print(char *buf, size_t length, const char *fmt, ...)
{
    va_list argv;
    va_start(argv, fmt);
    register int retval = _vsnprintf(buf, length, fmt, argv);
    va_end(argv);
}

void test_va_list(char *buf, size_t length)
{
    va_list_print(buf, length, "%s %d %x %f", "string", 11, 0x1919, 111.111);
}

void test_sprintf(char *buf, size_t length)
{
    sprintf(buf, "%s %d %x %f", "string", 11, 0x1919, 111.111);
}

static inline __attribute__((__always_inline__)) void va_arg_pack_printf(char *buf, const char *format, ...)
{
    sprintf(buf, format, __builtin_va_arg_pack());
}

void test_va_arg_pack(char *buf, size_t length)
{
    va_arg_pack_printf(buf, "%s %d %x %f", "string", 11, 0x1919, 111.111);
}

void assert_string_eq(const char *name, const char *actual, const char *expected)
{
    if (strcmp(actual, expected) == 0)
    {
        printf("Test %s: OK\n", name);
    }
    else 
    {
        printf("Test %s: FAIL ('%s' != '%s')\n", name, actual, expected);
    }
}

void assert_int_eq(const char *name, const int actual, const int expected)
{
    char actual1[100], expected1[100];
    itoa(actual, actual1, 10);
    itoa(expected, expected1, 10);
    assert_string_eq(name, actual1, expected1);
}

void assert_float_eq(const char *name, const double actual, const double expected)
{
    char actual1[100], expected1[100];    
    sprintf(actual1, "%f", actual);
    sprintf(expected1, "%f", expected);
    assert_string_eq(name, actual1, expected1);
}

int main()
{
    // Test if the va_list builtin can be passed to CRT methods.
    // I would expect this to fail until the va_list builtin is implemented for
    // the Windows ABI.
    char sz[100];
    test_va_list(sz, 100);
    assert_string_eq("Va List", sz, "string 11 1919 111.111000");

    // Test if sprintf can receive variadic parameters.
    // In most cases this is inlined and should work.
    // In the case of UCRT it is a direct call to the CRT
    // and I would expect it to fail until the variadic
    // ABI is correctly implemented.
    test_sprintf(sz, 100);
    assert_string_eq("SPrintf", sz, "string 11 1919 111.111000");

    // This test if the GCC va_arg_pack
    // builtin can pass variadic parameters.
    test_va_arg_pack(sz, 100);
    assert_string_eq("Va Arg Pack", sz, "string 11 1919 111.111000");

    // This test asserts the number stack frames that can be unwound using RtlVirtualUnwind.
    // This is a good proxy to determine if pdata entries are correct.
    assert_int_eq("Unwind Stack", count_stack_frames(), 11);

    assert_int_eq("check dll", check_dll(), 0);

    assert_float_eq("Nested Function", nested_add_squares(3.0, 7.0), 3.0 * 3.0 + 7.0 * 7.0);

    assert_int_eq("sjlj", sjlj_test(), 12);

    assert_int_eq("check struct", check_struct(), 1);

    assert_int_eq("hash big text", hash_big_text(), 2659567138);

    assert_int_eq("check big stack allocation", check_big_stack_allocation(), 1);

    double d1, d2, d3;
    sscanf("0.0 1.0 0.7", "%lf %lf %lf", &d1, &d2, &d3);
    assert_float_eq("sscanf double 1", d1, 0.0);
    assert_float_eq("sscanf double 2", d2, 1.0);
    assert_float_eq("sscanf double 3", d3, 0.7);

    _Float128 d128 = 1.2Q + 1.2F128;
    assert_float_eq("128bit float", d128, 2.4);

    return 0;
}
