#ifndef __SHA3_H__
#define __SHA3_H__

#include <cstring>
#include <cstdint>

namespace vitessa
{
    const int KECCAK_ROUNDS = 24;

    const uint64_t keccakf_rndc[24] =
    {
        0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
        0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
        0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
        0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
        0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
        0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
        0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
        0x8000000000008080, 0x0000000080000001, 0x8000000080008008,
    };

    const int keccakf_rotc[24] =
    {
         1,  3,  6, 10, 15, 21, 28, 36, 45, 55,  2, 14,
        27, 41, 56,  8, 25, 43, 62, 18, 39, 61, 20, 44,
    };

    const int keccakf_piln[24] =
    {
        10,  7, 11, 17, 18,  3,  5, 16,  8, 21, 24,  4,
        15, 23, 19, 13, 12,  2, 20, 14, 22,  9,  6,  1,
    };

    class sha3
    {
    public:
        template<typename T>
        static std::array<unsigned char, 28> sum224(T* in, int inlen)
        {
            std::array<unsigned char, 28> retval;
            sha3::hash((unsigned char*)in, inlen, retval.data(), 28);
            return retval;
        }

        template<typename T>
        static std::array<unsigned char, 32> sum256(T* in, int inlen)
        {
            std::array<unsigned char, 32> retval;
            sha3::hash((unsigned char*)in, inlen, retval.data(), 32);
            return retval;
        }

        template<typename T>
        static std::array<unsigned char, 48> sum384(T* in, int inlen)
        {
            std::array<unsigned char, 48> retval;
            sha3::hash((unsigned char*)in, inlen, retval.data(), 48);
            return retval;
        }

        template<typename T>
        static std::array<unsigned char, 64> sum512(T* in, int inlen)
        {
            std::array<unsigned char, 64> retval;
            sha3::hash((unsigned char*)in, inlen, retval.data(), 64);
            return retval;
        }

        /*
         * @hash   安全哈希散列
         *     \in       输入数据
         *     \inlen    输入数据的长度
         *     \md       输出数据
         *     \mdlen    输出数据的长度
         *     \iterate  重载参数，增加函数迭代次数
         *
         ********************************************************************
         *
         * @note mdlen说明
         *     \224 bits  ---  28
         *     \256 bits  ---  32
         *     \384 bits  ---  48
         *     \512 bits  ---  64
         */
        static void hash(const unsigned char* in, int inlen, unsigned char* md, int mdlen)
        {
            uint64_t st[25];
            uint8_t temp[144];
            int i, rsiz, rsizw;

            rsiz = 200 - 2 * mdlen;
            rsizw = rsiz / 8;

            memset(st, 0, sizeof(st));

            for ( ; inlen >= rsiz; inlen -= rsiz, in += rsiz) {
                for (i = 0; i < rsizw; i++) {
                    st[i] ^= ((uint64_t *) in)[i];
                }
                keccakf(st, KECCAK_ROUNDS);
            }

            // last block and padding
            memcpy(temp, in, inlen);
            temp[inlen++] = 6;
            memset(temp + inlen, 0, rsiz - inlen);
            temp[rsiz - 1] |= 0x80;

            for (i = 0; i < rsizw; i++) {
                st[i] ^= ((uint64_t *) temp)[i];
            }

            keccakf(st, KECCAK_ROUNDS);

            memcpy(md, st, mdlen);
        }

        static void hash(const unsigned char* in, int inlen, unsigned char* md, int mdlen, int iterate)
        {
            hash(in, inlen, md, mdlen);

            while( --iterate > 0 ) {
                hash(md, mdlen, md, mdlen);
            }
        }

    private:
        static inline uint64_t ROTL64(uint64_t lhs, int rhs)
        {
            return (lhs << rhs) | (lhs >> (64 - rhs));
        }
        
        static void keccakf(uint64_t st[25], int rounds)
        {
            int i, j, round;
            uint64_t t, bc[5];

            for (round = 0; round < rounds; round++) {

                // Theta
                for (i = 0; i < 5; i++) {
                    bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];
                }

                for (i = 0; i < 5; i++) {
                    t = bc[(i + 4) % 5] ^ ROTL64(bc[(i + 1) % 5], 1);
                    for (j = 0; j < 25; j += 5) {
                        st[j + i] ^= t;
                    }
                }

                // Rho Pi
                t = st[1];
                for (i = 0; i < 24; i++) {
                    j = keccakf_piln[i];
                    bc[0] = st[j];
                    st[j] = ROTL64(t, keccakf_rotc[i]);
                    t = bc[0];
                }

                //  Chi
                for (j = 0; j < 25; j += 5) {
                    for (i = 0; i < 5; i++) {
                        bc[i] = st[j + i];
                    }
                    for (i = 0; i < 5; i++) {
                        st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
                    }
                }

                //  Iota
                st[0] ^= keccakf_rndc[round];
            }
        }
    };
}

#endif

