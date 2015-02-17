/*
 * Copyright (C) 2015 rbox
 */
#include <fcntl.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <stdio.h>
#include <string.h>

#define UNLOCK_SIZE     12
#define SIGNATURE_SIZE 256

// http://stackoverflow.com/questions/356090/how-to-compute-the-nth-root-of-a-very-big-integer
static BIGNUM *nearest_cuberoot(BIGNUM *in)
{
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);

    BIGNUM *three = BN_CTX_get(ctx);
    BIGNUM *high = BN_CTX_get(ctx);
    BIGNUM *mid = BN_CTX_get(ctx);
    BIGNUM *low = BN_CTX_get(ctx);
    BIGNUM *tmp = BN_CTX_get(ctx);

    BN_set_word(three, 3);                                         // Create the constant 3
    BN_set_word(high, 1);                                          // high = 1

    do
    {
        BN_lshift1(high, high);                                    // high = high << 1 (high * 2)
        BN_exp(tmp, high, three, ctx);                             // tmp = high^3
    } while (BN_ucmp(tmp, in) <= -1);                              // while (tmp < in)

    BN_rshift1(low, high);                                         // low = high >> 1 (high / 2)

    while (BN_ucmp(low, high) <= -1)                               // while (low < high)
    {
        BN_add(tmp, low, high);                                    // tmp = low + high
        BN_rshift1(mid, tmp);                                      // mid = tmp >> 1 (tmp / 2)
        BN_exp(tmp, mid, three, ctx);                              // tmp = mid^3
        if (BN_ucmp(low, mid) <= -1 && BN_ucmp(tmp, in) <= -1)     // if (low < mid && tmp < in)
            BN_copy(low, mid);                                     // low = mid
        else if (BN_ucmp(high, mid) >= 1 && BN_ucmp(tmp, in) >= 1) // else if (high > mid && tmp > in)
            BN_copy(high, mid);                                    // high = mid
        else
        {
            // subtract 1 from mid because 1 will be added after the loop
            BN_sub_word(mid, 1);                                   // mid -= 1
            break;
        }
    }

    BN_add_word(mid, 1);                                           // mid += 1

    BIGNUM *result = BN_dup(mid);

    BN_CTX_end(ctx);
    BN_CTX_free(ctx);

    return result;
}

int main(int argc, char **argv)
{
    char c_manfid[9], c_serial[11], c_unlock[SIGNATURE_SIZE], c_unlock_maximum[SIGNATURE_SIZE];
    int fd;
    unsigned long ul_manfid, ul_serial;
    fd = open("/sys/block/mmcblk0/device/manfid", O_RDONLY);
    read(fd, c_manfid, sizeof(c_manfid));
    close(fd);
    fd = open("/sys/block/mmcblk0/device/serial", O_RDONLY);
    read(fd, c_serial, sizeof(c_serial));
    close(fd);
    ul_manfid = strtoul(c_manfid, NULL, 16);
    ul_serial = strtoul(c_serial, NULL, 16);
    snprintf(c_unlock, sizeof(c_unlock), "0x%02lx%08lx", ul_manfid, ul_serial);

    // Calculate a maximum BIGNUM with FF trailing padding
    memset(c_unlock + UNLOCK_SIZE, 0xFF, SIGNATURE_SIZE - UNLOCK_SIZE);
    RSA_padding_add_PKCS1_type_1(c_unlock_maximum, SIGNATURE_SIZE,
        c_unlock, SIGNATURE_SIZE - RSA_PKCS1_PADDING_SIZE);
    BIGNUM *maxnum = BN_bin2bn(c_unlock_maximum, SIGNATURE_SIZE, NULL);

    // Find the nearest cube root for the maximum
    BIGNUM *cuberoot = nearest_cuberoot(maxnum);
    unsigned char unlock_code[SIGNATURE_SIZE] = { 0 };
    /* Turn cuberoot into a bin and place it at the correct location in unlock_code.
     * It should be at the bottom so add SIGNATURE_SIZE to get to the end and
     * then subtract the number of bytes in the cube to back track. */
    BN_bn2bin(cuberoot, unlock_code + SIGNATURE_SIZE - BN_num_bytes(cuberoot));

    // Base64 encode the code
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, unlock_code, SIGNATURE_SIZE);
    BIO_flush(b64);
    BUF_MEM *bptr;
    BIO_get_mem_ptr(b64, &bptr);

    printf("Unlocking...\n");
    char idme_cmd[512];
    snprintf(idme_cmd, sizeof(idme_cmd), "idme unlock_code %s\n", bptr->data);
    system(idme_cmd);

    printf("Rebooting...\n");
    system("reboot");

    return 0;
}
