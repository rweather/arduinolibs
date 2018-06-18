
Arduino Cryptography Library
============================

This distribution contains a libraries and example applications to perform
cryptography operations on Arduino devices.  They are distributed under the
terms of the MIT license.

The [documentation](http://rweather.github.com/arduinolibs/crypto.html)
contains more information on the libraries and examples.

This repository used to contain a number of other examples and libraries
for other areas of Arduino functionality but most users are only interested
in the cryptography code.  The other projects have been moved to a
separate [repository](https://github.com/rweather/arduino-projects) and
only the cryptography code remains in this repository.

For more information on these libraries, to report bugs, or to suggest
improvements, please contact the author Rhys Weatherley via
[email](mailto:rhys.weatherley@gmail.com).

Recent significant changes to the library
-----------------------------------------

XXX 2018:

* Problem with BLAKE2s and BLAKE2b HMAC mode when the data was zero-length.
* KeyRing for storing key pairs and other key material in EEPROM or Flash.
* Add better functions to Curve25519 and Ed25519 for generating key pairs.
* Noise and NoiseLink protocols.

Apr 2018:

* Acorn128 and Ascon128 authenticated ciphers (finalists in the CAESAR AEAD
  competition in the light-weight category).
* Split the library into Crypto (core), CryptoLW (light-weight), and
  CryptoLegacy (deprecated algorithms).
* Tiny and small versions of AES for reducing memory requirements.
* Port the library to ESP8266 and ESP32.
* Make the RNG class more robust if the app doesn't call begin() or loop().

Nov 2017:

* Fix the AVR assembly version of Speck and speed it up a little.
* API improvements to the RNG class.
