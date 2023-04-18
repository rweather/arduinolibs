Python extension for the light-weight algorithms for the rweather Arduino Cryptography Library. 
https://github.com/rweather/arduinolibs

### Installation

To install using Python 3 on Debian/Ubuntu: first ensure you have python3-dev, pip3. 
Then install the Boost-Python library and the python setup-tools.

Example installation for Ubuntu and Debian:

* `sudo apt install -y python3-dev python3-pip libboost-python-dev`
* `sudo pip3 install setuptools`

From the libraries/CryptoLW/python directory, run:
 
* `sudo python3 setup.py install`

### Usage

#### Encryption
```
from CryptoLW import Acorn128
import secrets
# Note: secrets is new in Python 3.6. You can use a different library 
# for random bytes, e.g. pycryptodome.Random.

# Sample data
header = b"header"
data = b"secret"
key = secrets.token_bytes(16)
iv = secrets.token_bytes(16)

# Prepare cipher
cipher = Acorn128()
cipher.clear()       # Required if reusing the cipher
cipher.setKey(key)   # Shared secret key
cipher.setIV(iv)     # Initialization Vector

# Add data
cipher.addAuthData(header)
ciphertext = cipher.encrypt(data)

# Compute tag
tag = cipher.computeTag()
```
The initialisation vector (IV) must not be reused between encryption sessions and should
not be an easily predictable value (e.g. do not use iv++).
The header, tag, iv and ciphertext can be transmitted to the destination over an unsafe medium.
The shared secret key must not be transmitted, it must be handled prior to using this algorithm.

#### Decryption
```
from CryptoLW import Acorn128

# Prepare cipher
cipher = Acorn128()
cipher.clear()       # Required if reusing the cipher
cipher.setKey(key)   # Shared secret key
cipher.setIV(iv)     # Initialization Vector

# Initialise with unencrypted content
cipher.addAuthData(header)

# Apply encrypted content to recover cleartext
cleartext = cipher.decrypt(ciphertext)

try:
    cipher.checkTag(tag)  # Use tag produced by encryption process
    print("The decryption process is successful")
except ValueError:
    print("The process failed, header cannot be trusted and cleartext is likely invalid")
```
A ValueError exception is thrown when the tag does not match the content that is applied
on the cipher.