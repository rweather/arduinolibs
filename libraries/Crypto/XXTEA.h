// XXTEA as proposed in: https://en.wikipedia.org/wiki/XXTEA

#ifndef CRYPTO_XXTEA_H

#include <BlockCipher.h>

class XXTEA : public BlockCipher {
public:
    XXTEA();
    virtual ~XXTEA();

    size_t blockSize() const;
	void setBlockSize(size_t s);
    size_t keySize() const;

    bool setKey(const uint8_t *key, size_t len);

    void encryptBlock(uint8_t *output, const uint8_t *input);
    void decryptBlock(uint8_t *output, const uint8_t *input);

    void clear();

private:
   	void btea(uint8_t *v, int n, uint8_t const key[16]) ;
	size_t blocksize=16;
   	unsigned char key[16]={0};
};

#endif