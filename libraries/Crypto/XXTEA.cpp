// XXTEA as proposed in: https://en.wikipedia.org/wiki/XXTEA

#include <XXTEA.h>
#include "Crypto.h"

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

/**
 * \brief Constructs an XXTE block cipher with an initial key of 0.
 * [documentation](https://en.wikipedia.org/wiki/XXTEA)
 * Default blocksize is set to 128 bit (16 bytes), but this could be changed.
 * This constructor should be followed by a call to setKey() before the
 * block cipher can be used for encryption or decryption.
 */
XXTEA::XXTEA() {	
}

XXTEA::~XXTEA()
{
    clean(key);
}


size_t XXTEA::blockSize() const
{
    return blocksize;
}

void XXTEA::setBlockSize(size_t s) {
	blocksize = s;
}

size_t XXTEA::keySize() const {
	return 16;
}

bool XXTEA::setKey(const uint8_t *newkey, size_t len) {
	if(len !=16) return false;
	
	for(int i=0;i<16;i++) key[i] = newkey[i];
	return true;
}

void XXTEA::encryptBlock(uint8_t *output, const uint8_t *input) {
	for(int i=0; i<blocksize; i++) output[i] = input[i];
	btea(output, blocksize, key);
}

void XXTEA::decryptBlock(uint8_t *output, const uint8_t *input) {
	for(int i=0; i<blocksize; i++) output[i] = input[i];
	btea(output, -blocksize, key);
}

void XXTEA::clear() {
	clean(key);
}

void XXTEA::btea(uint8_t *v, int n, uint8_t const key[16]) {
    uint8_t y, z, sum;
    unsigned p, rounds, e;
    if (n > 1) {          /* Coding Part */
      rounds = 6 + 52/n;
      sum = 0;
      z = v[n-1];
      do {
        sum += DELTA;
        e = (sum >> 2) & 3;
        for (p=0; p<n-1; p++) {
          y = v[p+1]; 
          z = v[p] += MX;
        }
        y = v[0];
        z = v[n-1] += MX;
      } while (--rounds);
    } else if (n < -1) {  /* Decoding Part */
      n = -n;
      rounds = 6 + 52/n;
      sum = rounds*DELTA;
      y = v[0];
      do {
        e = (sum >> 2) & 3;
        for (p=n-1; p>0; p--) {
          z = v[p-1];
          y = v[p] -= MX;
        }
        z = v[n-1];
        y = v[0] -= MX;
        sum -= DELTA;
      } while (--rounds);
    }
  }
