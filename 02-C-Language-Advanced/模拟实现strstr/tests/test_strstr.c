#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char* kmpStrstr(
    const char* haystack,
    size_t len1,
    const char* needle,
    size_t len2
);

char* naiveStrstr(
    const char* haystack,
    size_t len1,
    const char* needle,
    size_t len2
);

typedef char* (*StrstrFn)(
    const char* haystack,
    size_t len1,
    const char* needle,
    size_t len2
);

/* 防止 benchmark 中调用结果被优化掉。 */
static volatile uintptr_t g_sink = 0;

/* ============================================================================
 * 确定性随机数
 * ========================================================================== */

static uint32_t g_rng_state = 0x12345678u;

static uint32_t nextRandom(void)
{
    uint32_t x = g_rng_state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    g_rng_state = x;
    return x;
}

static void fillRandomString(
    char* str,
    size_t len,
    const char* alphabet
)
{
    size_t alphabetLen = strlen(alphabet);

    for (size_t i = 0; i < len; ++i) {
        str[i] = alphabet[nextRandom() % alphabetLen];
    }

    str[len] = '\0';
}

/* ============================================================================
 * 正确性测试
 * ========================================================================== */

typedef struct {
    const char* haystack;
    const char* needle;
} FixedCase;

static int checkOneCase(
    const char* haystack,
    const char* needle,
    size_t caseIndex
)
{
    size_t hayLen = strlen(haystack);
    size_t needleLen = strlen(needle);

    char* expected = strstr(haystack, needle);

    char* naiveResult =
        naiveStrstr(haystack, hayLen, needle, needleLen);

    char* kmpResult =
        kmpStrstr(haystack, hayLen, needle, needleLen);

    if (naiveResult != expected || kmpResult != expected) {
        printf("\n[FAIL] case %zu\n", caseIndex);
        printf("haystack : \"%s\"\n", haystack);
        printf("needle   : \"%s\"\n", needle);

        if (expected != NULL) {
            printf(
                "expected  : offset %td\n",
                expected - haystack
            );
        } else {
            printf("expected  : NULL\n");
        }

        printf(
            "naive     : %p\n",
            (void*)naiveResult
        );

        printf(
            "kmp       : %p\n",
            (void*)kmpResult
        );

        return 0;
    }

    return 1;
}

static int runFixedTests(void)
{
    static const FixedCase cases[] = {
        {"", ""},
        {"", "a"},
        {"abc", ""},
        {"abc", "a"},
        {"abc", "b"},
        {"abc", "c"},
        {"abc", "abc"},
        {"abc", "abcd"},

        {"aaaaa", "a"},
        {"aaaaa", "aa"},
        {"aaaaa", "aaa"},
        {"aaaaa", "aaaaa"},
        {"aaaaa", "aaaaaa"},

        {"abababac", "ababac"},
        {"abababab", "abab"},
        {"aaaaab", "aaab"},
        {"mississippi", "issip"},

        {
            "abcxabcdabxabcdabcdabcy",
            "abcdabcy"
        },

        {
            "aaaaaaaaaaaaaaaaaaaaaaaaab",
            "aaaaaaaab"
        },

        {
            "abcdabcabcdabcdab",
            "abcdab"
        }
    };

    size_t count = sizeof cases / sizeof cases[0];

    for (size_t i = 0; i < count; ++i) {
        if (!checkOneCase(
                cases[i].haystack,
                cases[i].needle,
                i)) {
            return 0;
        }
    }

    printf("固定测试通过: %zu 组\n", count);
    return 1;
}

static int runRandomTests(size_t rounds)
{
    char haystack[513];
    char needle[129];

    static const char* alphabets[] = {
        "ab",
        "abcd",
        "abcdefghijklmnopqrstuvwxyz"
    };

    for (size_t round = 0; round < rounds; ++round) {

        size_t hayLen = nextRandom() % 513;
        size_t needleLen = nextRandom() % 129;

        const char* alphabet =
            alphabets[nextRandom() % 3];

        fillRandomString(
            haystack,
            hayLen,
            alphabet
        );

        /*
         * 一部分样本直接从 haystack 中截取 needle，
         * 确保测试中有足够多的“必然命中”情况。
         */
        if (
            needleLen <= hayLen &&
            needleLen > 0 &&
            (nextRandom() % 3 == 0)
        ) {
            size_t start =
                nextRandom() % (hayLen - needleLen + 1);

            memcpy(
                needle,
                haystack + start,
                needleLen
            );

            needle[needleLen] = '\0';
        } else {
            fillRandomString(
                needle,
                needleLen,
                alphabet
            );
        }

        if (!checkOneCase(
                haystack,
                needle,
                round)) {
            printf(
                "随机测试在第 %zu 轮失败\n",
                round
            );
            return 0;
        }

        if (
            (round + 1) % 1000 == 0
        ) {
            printf(
                "随机测试已通过 %zu 组\n",
                round + 1
            );
        }
    }

    printf(
        "随机测试全部通过: %zu 组\n",
        rounds
    );

    return 1;
}

/* ============================================================================
 * benchmark
 * ========================================================================== */

static double nowSeconds(void)
{
    struct timespec ts;

    if (timespec_get(&ts, TIME_UTC) != TIME_UTC) {
        fprintf(stderr, "timespec_get failed\n");
        exit(EXIT_FAILURE);
    }

    return
        (double)ts.tv_sec +
        (double)ts.tv_nsec / 1000000000.0;
}

static double runBatch(
    StrstrFn fn,
    const char* haystack,
    size_t hayLen,
    const char* needle,
    size_t needleLen,
    size_t repetitions
)
{
    double begin = nowSeconds();

    for (size_t i = 0; i < repetitions; ++i) {

        char* result =
            fn(
                haystack,
                hayLen,
                needle,
                needleLen
            );

        g_sink ^= (uintptr_t)result;
    }

    return nowSeconds() - begin;
}

static void sortFive(double values[5])
{
    for (size_t i = 1; i < 5; ++i) {

        double value = values[i];
        size_t j = i;

        while (
            j > 0 &&
            values[j - 1] > value
        ) {
            values[j] = values[j - 1];
            --j;
        }

        values[j] = value;
    }
}

/*
 * 自动增加重复次数，使单轮测试至少达到约 10 ms。
 * 最终取 5 次测试的中位数。
 */
static double measureFunction(
    StrstrFn fn,
    const char* haystack,
    size_t hayLen,
    const char* needle,
    size_t needleLen
)
{
    size_t repetitions = 1;

    while (1) {

        double elapsed =
            runBatch(
                fn,
                haystack,
                hayLen,
                needle,
                needleLen,
                repetitions
            );

        if (
            elapsed >= 0.010 ||
            repetitions >= (1u << 22)
        ) {
            break;
        }

        repetitions *= 2;
    }

    double samples[5];

    for (size_t i = 0; i < 5; ++i) {

        double elapsed =
            runBatch(
                fn,
                haystack,
                hayLen,
                needle,
                needleLen,
                repetitions
            );

        samples[i] =
            elapsed * 1e9 /
            (double)repetitions;
    }

    sortFive(samples);

    return samples[2];
}

/* ============================================================================
 * benchmark 数据集
 * ========================================================================== */

typedef enum {
    CASE_FAST_MISS,
    CASE_RANDOM_HIT,
    CASE_REPETITIVE_MISS,
    CASE_REPETITIVE_HIT
} BenchKind;

typedef struct {
    size_t hayLen;
    size_t needleLen;
} BenchSize;

typedef struct {
    const char* name;

    /*
     * 0 = 普通数据
     * 1 = 高重复数据
     */
    int family;

    size_t hayLen;
    size_t needleLen;

    /*
     * 我们之前定义的量：
     *
     *     (n - m + 1) * m
     * K = -----------------
     *          n + m
     *
     * 当 score <= K 时选择 naive。
     */
    double score;

    double naiveNs;
    double kmpNs;
} BenchResult;

static const char* getKindName(BenchKind kind)
{
    switch (kind) {
        case CASE_FAST_MISS:
            return "fast-miss";

        case CASE_RANDOM_HIT:
            return "random-hit";

        case CASE_REPETITIVE_MISS:
            return "repeat-miss";

        case CASE_REPETITIVE_HIT:
            return "repeat-hit";
    }

    return "unknown";
}

static int getFamily(BenchKind kind)
{
    if (
        kind == CASE_REPETITIVE_MISS ||
        kind == CASE_REPETITIVE_HIT
    ) {
        return 1;
    }

    return 0;
}

static void buildBenchmarkCase(
    BenchKind kind,
    char* haystack,
    size_t hayLen,
    char* needle,
    size_t needleLen
)
{
    switch (kind) {

        /*
         * haystack 中没有 'z'，
         * needle 第一字符就是 'z'。
         *
         * naive 几乎每次比较一次就失败。
         */
        case CASE_FAST_MISS:

            fillRandomString(
                haystack,
                hayLen,
                "abcdefghijklmnopqrstuvwxy"
            );

            fillRandomString(
                needle,
                needleLen,
                "abcdefghijklmnopqrstuvwxy"
            );

            needle[0] = 'z';
            break;

        /*
         * 普通随机字符串，并保证 needle
         * 是 haystack 中间的一段。
         */
        case CASE_RANDOM_HIT: {

            fillRandomString(
                haystack,
                hayLen,
                "abcdefghijklmnopqrstuvwxyz"
            );

            size_t start =
                (hayLen - needleLen) / 2;

            memcpy(
                needle,
                haystack + start,
                needleLen
            );

            needle[needleLen] = '\0';
            break;
        }

        /*
         * 典型的 naive 最坏模式：
         *
         * haystack:
         * aaaaaaaaaaaaaaaaa...
         *
         * needle:
         * aaaaaaaab
         */
        case CASE_REPETITIVE_MISS:

            memset(
                haystack,
                'a',
                hayLen
            );

            haystack[hayLen] = '\0';

            memset(
                needle,
                'a',
                needleLen
            );

            needle[needleLen - 1] = 'b';
            needle[needleLen] = '\0';
            break;

        /*
         * 与上面类似，但最后能够命中。
         */
        case CASE_REPETITIVE_HIT:

            memset(
                haystack,
                'a',
                hayLen
            );

            haystack[hayLen - 1] = 'b';
            haystack[hayLen] = '\0';

            memset(
                needle,
                'a',
                needleLen
            );

            needle[needleLen - 1] = 'b';
            needle[needleLen] = '\0';
            break;
    }
}

/* ============================================================================
 * K 系数拟合
 * ========================================================================== */

static void findBestK(
    const BenchResult* results,
    size_t count,
    int family,
    const char* title
)
{
    double bestK = 0.0;
    double bestCost = DBL_MAX;

    size_t bestWrong = 0;

    double oracleCost = 0.0;

    for (size_t i = 0; i < count; ++i) {

        if (
            family != -1 &&
            results[i].family != family
        ) {
            continue;
        }

        oracleCost +=
            results[i].naiveNs <
                    results[i].kmpNs
                ? results[i].naiveNs
                : results[i].kmpNs;
    }

    /*
     * 分类结果只会在某个已有 score
     * 被越过时发生变化，因此没有必要
     * 连续暴力扫描 K。
     */
    for (size_t candidate = 0;
         candidate <= count;
         ++candidate) {

        double k =
            candidate == 0
                ? 0.0
                : results[candidate - 1].score;

        double cost = 0.0;
        size_t wrong = 0;

        for (size_t i = 0; i < count; ++i) {

            if (
                family != -1 &&
                results[i].family != family
            ) {
                continue;
            }

            int chooseNaive =
                results[i].score <= k;

            double selected =
                chooseNaive
                    ? results[i].naiveNs
                    : results[i].kmpNs;

            double optimal =
                results[i].naiveNs <
                        results[i].kmpNs
                    ? results[i].naiveNs
                    : results[i].kmpNs;

            cost += selected;

            if (selected != optimal) {
                ++wrong;
            }
        }

        if (cost < bestCost) {
            bestCost = cost;
            bestK = k;
            bestWrong = wrong;
        }
    }

    printf("\n%s\n", title);
    printf("------------------------------\n");
    printf("最佳经验系数 K : %.3f\n", bestK);
    printf(
        "相对逐案例最优开销 : %.3fx\n",
        bestCost / oracleCost
    );
    printf(
        "错误分流案例数     : %zu\n",
        bestWrong
    );
}

static int runBenchmark(void)
{
    static const BenchSize sizes[] = {
        {64,     4},
        {64,    16},
        {256,    8},
        {256,   64},
        {1024,  16},
        {1024, 128},
        {4096,  32},
        {4096, 256},
        {16384, 64},
        {16384, 512}
    };

    enum {
        KIND_COUNT = 4,
        SIZE_COUNT =
            sizeof sizes / sizeof sizes[0],
        RESULT_COUNT =
            KIND_COUNT * SIZE_COUNT
    };

    BenchResult results[RESULT_COUNT];
    size_t resultIndex = 0;

    printf("\n");
    printf("========================================\n");
    printf("strstr benchmark\n");
    printf("========================================\n");

    for (size_t s = 0; s < SIZE_COUNT; ++s) {

        size_t n = sizes[s].hayLen;
        size_t m = sizes[s].needleLen;

        char* haystack =
            malloc(n + 1);

        char* needle =
            malloc(m + 1);

        if (
            haystack == NULL ||
            needle == NULL
        ) {
            free(haystack);
            free(needle);

            fprintf(
                stderr,
                "benchmark malloc failed\n"
            );

            return 0;
        }

        for (
            BenchKind kind = CASE_FAST_MISS;
            kind <= CASE_REPETITIVE_HIT;
            ++kind
        ) {
            buildBenchmarkCase(
                kind,
                haystack,
                n,
                needle,
                m
            );

            /*
             * benchmark 前再验证一次，
             * 避免给错误实现测性能。
             */
            char* expected =
                strstr(
                    haystack,
                    needle
                );

            char* naiveResult =
                naiveStrstr(
                    haystack,
                    n,
                    needle,
                    m
                );

            char* kmpResult =
                kmpStrstr(
                    haystack,
                    n,
                    needle,
                    m
                );

            if (
                naiveResult != expected ||
                kmpResult != expected
            ) {
                printf(
                    "benchmark case correctness failed\n"
                );

                free(haystack);
                free(needle);
                return 0;
            }

            double naiveNs =
                measureFunction(
                    naiveStrstr,
                    haystack,
                    n,
                    needle,
                    m
                );

            double kmpNs =
                measureFunction(
                    kmpStrstr,
                    haystack,
                    n,
                    needle,
                    m
                );

            double score =
                (
                    (double)(n - m + 1) *
                    (double)m
                ) /
                (double)(n + m);

            BenchResult* result =
                &results[resultIndex++];

            result->name =
                getKindName(kind);

            result->family =
                getFamily(kind);

            result->hayLen = n;
            result->needleLen = m;

            result->score = score;

            result->naiveNs = naiveNs;
            result->kmpNs = kmpNs;

            printf(
                "%-12s "
                "n=%6zu "
                "m=%4zu "
                "score=%8.2f "
                "naive=%9.1f ns "
                "kmp=%9.1f ns "
                "%s\n",
                result->name,
                n,
                m,
                score,
                naiveNs,
                kmpNs,
                naiveNs <= kmpNs
                    ? "naive"
                    : "KMP"
            );
        }

        free(haystack);
        free(needle);
    }

    /*
     * findBestK 的候选 K 来自 results 中的 score。
     *
     * result 当前按照 size + kind 排列，
     * 这并不影响正确性，因为所有 score
     * 都会被尝试。
     */
    findBestK(
        results,
        RESULT_COUNT,
        0,
        "普通数据集"
    );

    findBestK(
        results,
        RESULT_COUNT,
        1,
        "高重复数据集"
    );

    findBestK(
        results,
        RESULT_COUNT,
        -1,
        "综合数据集"
    );

    /*
     * 防止编译器认为 g_sink 无意义。
     */
    printf(
        "\nbenchmark sink: %llu\n",
        (unsigned long long)g_sink
    );

    return 1;
}

/* ============================================================================
 * main
 * ========================================================================== */

int main(int argc, char* argv[])
{
    printf("开始 strstr 正确性测试...\n\n");

    if (!runFixedTests()) {
        return EXIT_FAILURE;
    }

    if (!runRandomTests(10000)) {
        return EXIT_FAILURE;
    }

    printf("\n");
    printf("========================================\n");
    printf("正确性测试全部通过\n");
    printf("========================================\n");

    if (
        argc >= 2 &&
        strcmp(argv[1], "--bench") == 0
    ) {
        if (!runBenchmark()) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
