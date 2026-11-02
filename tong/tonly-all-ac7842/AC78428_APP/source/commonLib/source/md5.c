#include "md5.h"

#define F(x, y, z)  (((x) & (y)) | (~(x) & (z)))
#define G(x, y, z)  (((x) & (z)) | ((y) & ~(z)))
#define H(x, y, z)  ((x) ^ (y) ^ (z))
#define I(x, y, z)  ((y) ^ ((x) | ~(z)))

#define ROTL(x, n)  (((x) << (n)) | ((x) >> (32 - (n))))

#define STEP(f, a, b, c, d, x, s, t) \
    do { \
        a += f(b, c, d) + x + t; \
        a = ROTL(a, s); \
        a += b; \
    } while (0)

static const uint32_t T[] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

static const uint8_t shift[] = {7,12,17,22, 5,9,14,20, 4,11,16,23, 6,10,15,21};

static void md5_transform(uint32_t state[4], const uint8_t block[64])
{
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t x[16];
    int i;

    for (i = 0; i < 16; i++)
        x[i] = (uint32_t)block[i*4] | ((uint32_t)block[i*4+1] << 8)
             | ((uint32_t)block[i*4+2] << 16) | ((uint32_t)block[i*4+3] << 24);

    // Round 1
    STEP(F, a,b,c,d, x[0], shift[0], T[0]);
    STEP(F, d,a,b,c, x[1], shift[1], T[1]);
    STEP(F, c,d,a,b, x[2], shift[2], T[2]);
    STEP(F, b,c,d,a, x[3], shift[3], T[3]);
    STEP(F, a,b,c,d, x[4], shift[0], T[4]);
    STEP(F, d,a,b,c, x[5], shift[1], T[5]);
    STEP(F, c,d,a,b, x[6], shift[2], T[6]);
    STEP(F, b,c,d,a, x[7], shift[3], T[7]);
    STEP(F, a,b,c,d, x[8], shift[0], T[8]);
    STEP(F, d,a,b,c, x[9], shift[1], T[9]);
    STEP(F, c,d,a,b, x[10],shift[2],T[10]);
    STEP(F, b,c,d,a, x[11],shift[3],T[11]);
    STEP(F, a,b,c,d, x[12],shift[0],T[12]);
    STEP(F, d,a,b,c, x[13],shift[1],T[13]);
    STEP(F, c,d,a,b, x[14],shift[2],T[14]);
    STEP(F, b,c,d,a, x[15],shift[3],T[15]);

    // Round 2
    STEP(G, a,b,c,d, x[1], shift[4], T[16]);
    STEP(G, d,a,b,c, x[6], shift[5], T[17]);
    STEP(G, c,d,a,b, x[11],shift[6],T[18]);
    STEP(G, b,c,d,a, x[0], shift[7], T[19]);
    STEP(G, a,b,c,d, x[5], shift[4], T[20]);
    STEP(G, d,a,b,c, x[10],shift[5],T[21]);
    STEP(G, c,d,a,b, x[15],shift[6],T[22]);
    STEP(G, b,c,d,a, x[4], shift[7], T[23]);
    STEP(G, a,b,c,d, x[9], shift[4], T[24]);
    STEP(G, d,a,b,c, x[14],shift[5],T[25]);
    STEP(G, c,d,a,b, x[3], shift[6], T[26]);
    STEP(G, b,c,d,a, x[8], shift[7], T[27]);
    STEP(G, a,b,c,d, x[13],shift[4],T[28]);
    STEP(G, d,a,b,c, x[2], shift[5], T[29]);
    STEP(G, c,d,a,b, x[7], shift[6], T[30]);
    STEP(G, b,c,d,a, x[12],shift[7],T[31]);

    // Round3
    STEP(H, a,b,c,d, x[5], shift[8], T[32]);
    STEP(H, d,a,b,c, x[8], shift[9], T[33]);
    STEP(H, c,d,a,b, x[11],shift[10],T[34]);
    STEP(H, b,c,d,a, x[14],shift[11],T[35]);
    STEP(H, a,b,c,d, x[1], shift[8], T[36]);
    STEP(H, d,a,b,c, x[4], shift[9], T[37]);
    STEP(H, c,d,a,b, x[7], shift[10],T[38]);
    STEP(H, b,c,d,a, x[10],shift[11],T[39]);
    STEP(H, a,b,c,d, x[13],shift[8], T[40]);
    STEP(H, d,a,b,c, x[0], shift[9], T[41]);
    STEP(H, c,d,a,b, x[3], shift[10],T[42]);
    STEP(H, b,c,d,a, x[6], shift[11],T[43]);
    STEP(H, a,b,c,d, x[9], shift[8], T[44]);
    STEP(H, d,a,b,c, x[12],shift[9], T[45]);
    STEP(H, c,d,a,b, x[15],shift[10],T[46]);
    STEP(H, b,c,d,a, x[2], shift[11],T[47]);

    // Round4
    STEP(I, a,b,c,d, x[0], shift[12],T[48]);
    STEP(I, d,a,b,c, x[7], shift[13],T[49]);
    STEP(I, c,d,a,b, x[14],shift[14],T[50]);
    STEP(I, b,c,d,a, x[5], shift[15],T[51]);
    STEP(I, a,b,c,d, x[12],shift[12],T[52]);
    STEP(I, d,a,b,c, x[3], shift[13],T[53]);
    STEP(I, c,d,a,b, x[10],shift[14],T[54]);
    STEP(I, b,c,d,a, x[1], shift[15],T[55]);
    STEP(I, a,b,c,d, x[8], shift[12],T[56]);
    STEP(I, d,a,b,c, x[15],shift[13],T[57]);
    STEP(I, c,d,a,b, x[6], shift[14],T[58]);
    STEP(I, b,c,d,a, x[13],shift[15],T[59]);
    STEP(I, a,b,c,d, x[4], shift[12],T[60]);
    STEP(I, d,a,b,c, x[11],shift[13],T[61]);
    STEP(I, c,d,a,b, x[2], shift[14],T[62]);
    STEP(I, b,c,d,a, x[9], shift[15],T[63]);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

void md5_init(MD5_CTX *ctx)
{
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
    ctx->len = 0;
}

void md5_update(MD5_CTX *ctx, const uint8_t *data, size_t len)
{
    size_t i, pos = ctx->len % MD5_BLOCK_LEN;
    ctx->len += len;

    for (i = 0; i < len; i++)
    {
        ctx->buf[pos++] = data[i];
        if (pos == MD5_BLOCK_LEN)
        {
            md5_transform(ctx->state, ctx->buf);
            pos = 0;
        }
    }
}

void md5_final(MD5_CTX *ctx, uint8_t digest[MD5_DIGEST_LEN])
{
    uint8_t pad[64] = {0x80};
    uint64_t bits = ctx->len << 3;
    size_t pos = ctx->len % MD5_BLOCK_LEN;
    int i;

    md5_update(ctx, pad, MD5_BLOCK_LEN - pos - 8);

    for (i = 0; i < 8; i++)
        ctx->buf[56 + i] = (bits >> (i * 8)) & 0xFF;
    md5_transform(ctx->state, ctx->buf);

    for (i = 0; i < 4; i++)
    {
        digest[i*4+0] = (ctx->state[i] >> 0) & 0xFF;
        digest[i*4+1] = (ctx->state[i] >> 8) & 0xFF;
        digest[i*4+2] = (ctx->state[i] >> 16) & 0xFF;
        digest[i*4+3] = (ctx->state[i] >> 24) & 0xFF;
    }
}

void md5_calc(const uint8_t *data, size_t len, uint8_t digest[MD5_DIGEST_LEN])
{
    MD5_CTX ctx;
    md5_init(&ctx);
    md5_update(&ctx, data, len);
    md5_final(&ctx, digest);
}

void md5_bin2str(const uint8_t digest[MD5_DIGEST_LEN], char str[33])
{
    static const char hex[] = "0123456789abcdef";
    int i;
    for (i = 0; i < 16; i++)
    {
        str[i*2]   = hex[(digest[i] >> 4) & 0x0F];
        str[i*2+1] = hex[digest[i] & 0x0F];
    }
    str[32] = '\0';
}

