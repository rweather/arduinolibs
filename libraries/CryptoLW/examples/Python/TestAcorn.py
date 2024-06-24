from CryptoLW import Acorn128, Ascon128
import binascii
import secrets

header = b"header"
data = b"secret"
key = secrets.token_bytes(16)   # get_random_bytes(16)
iv = secrets.token_bytes(16)

cipher = Acorn128()
cipher.clear()
cipher.setKey(key)
cipher.setIV(iv)
cipher.addAuthData(header)
ciphertext = cipher.encrypt(data)
tag = cipher.computeTag()

print("Cipher Text : %s" % binascii.hexlify(ciphertext).decode('utf-8'))
print("Tag : %s" % binascii.hexlify(tag).decode('utf-8'))

# Tester le decryptage

cipher.clear()
cipher.setKey(key)
cipher.setIV(iv)
cipher.addAuthData(header)
cleartext = cipher.decrypt(ciphertext)

print("Clear text : %s" % cleartext.decode('utf-8'))
cipher.checkTag(tag)
print("Tag OK!")

cipher.clear()
cipher.setKey(key)
cipher.setKey(iv)
cipher.addAuthData(header)
cleartext = cipher.decrypt(ciphertext)
try:
    cipher.checkTag(iv)  # Mauvais tag (IV)
    print("Une erreur aurait du etre souleve (***incorrect***)")
except ValueError:
    print("Echec verification (correct)")


# Test avec donnees du Arduino
header = binascii.unhexlify(b"0102030405060708")
key = binascii.unhexlify(b"233952DEE4D5ED5F9B9C6D6FF80FF478")
iv = binascii.unhexlify(b"2D2B10316ABE7766AABADFEFB8E139EA")
tag = binascii.unhexlify(b"3E67F10B7FD68BAA8206C62BD0026B1B")
buffer_crypte = binascii.unhexlify(b"136A14EC1F53E0C7CB19AADC38E70D274774E3CAC147223E")

cipher.clear()
cipher.setKey(key)
cipher.setIV(iv)
cipher.addAuthData(header)
cleartext = cipher.decrypt(buffer_crypte)

print("Clear text : %s" % binascii.hexlify(cleartext).decode('utf-8'))
cipher.checkTag(tag)
print("Tag OK!")
