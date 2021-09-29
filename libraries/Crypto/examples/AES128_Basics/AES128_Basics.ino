/*
 * Copyright (C) 2015 Southern Storm Software, Pty Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*
This example explains basic AES128 implementation.
In Arduino serial monitor text appears to be non readable characters, but if you use any other serial terminals you can see the hex values
Example contributor: Aswin
*/

#include <Crypto.h>
#include <AES.h>
#include <string.h>

//key[16] cotain 16 byte key(128 bit) for encryption
byte key[16]={0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
//plaintext[16] contain the text we need to encrypt
byte plaintext[16]={0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
//cypher[16] stores the encrypted text
byte cypher[16];
//decryptedtext[16] stores decrypted text after decryption
byte decryptedtext[16];
//creating an object of AES128 class
AES128 aes128;


void setup() {
  Serial.begin(9600);
  aes128.setKey(key,16);// Setting Key for AES
  
  Serial.print("Before Encryption:");
  for(int i=0; i<sizeof(plaintext); i++){
    Serial.write(plaintext[i]);
   }
   
  aes128.encryptBlock(cypher,plaintext);//cypher->output block and plaintext->input block
  Serial.println();
  Serial.print("After Encryption:");
  for(int j=0;j<sizeof(cypher);j++){
      Serial.write(cypher[j]);
    }
    
   aes128.decryptBlock(decryptedtext,cypher);
   
  Serial.println();
  Serial.print("After Dencryption:");
  for(int i=0; i<sizeof(decryptedtext); i++){
    Serial.write(decryptedtext[i]);
   }

}

void loop() {
  // put your main code here, to run repeatedly:

}
