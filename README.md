
Arduino Cryptography Library
============================

This distribution contains a libraries and example applications to perform
cryptography operations on Arduino devices.  They are distributed under the
terms of the MIT license.

The [documentation](http://rweather.github.io/arduinolibs/crypto.html)
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

Apr 2023:

Brad Bock contributed a RNG back end for newer AVR chips that uses
Custom Configurable Logic (CCL) to generate an unstable clock source
instead of using the CPU watchdog as on older AVR chips.

Feb 2023:

NIST has selected ASCON as the winner of the Lightweight Cryptography
Competition.  This repository has an older implementation of ASCON-128
which should be compatible with the final winning version.  Let me know
if you have any issues.

The winning version has additional AEAD cipher and hashing modes that
this repository does not implement yet.  However, my companion repository
[ASCON Suite](https://github.com/rweather/ascon-suite) does implement
all of the additional modes.

NIST is in the process of finalising the standard.  Once the standard is
published, I will move Ascon128 from CryptoLW to Crypto and implement the
extra modes in this repository.  In the meantime, please use ASCON Suite if
you need support for ASCON in your Arduino project.

Mar 2022:

* HMAC-BLAKE2b and HMAC-BLAKE2s were giving incorrect results when the
message being authenticated was zero-length.

Jan 2022:

* All-in-one hmac() function in Hash.h for simplified HMAC computations.
* New API for the HKDF hash-based key derivation function.
* Make the ESP32 version of AES less dependent on include file locations.

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
