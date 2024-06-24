from distutils.core import setup, Extension
import sys
from os import path

path_libs = '../../'


def main():

    if sys.version_info >= (3,):
        BOOST_LIB = 'boost_python3'
    else:
        BOOST_LIB = 'boost_python'

    setup(name="CryptoLW",
          version="0.2.0",
          description=
          """
          Python extension for the light-weight algorithms for the rweather Arduino Cryptography Library. 
          https://github.com/rweather/arduinolibs
          by Rhys Weatherley <rhys.weatherley@gmail.com>
          """,
          author="Mathieu Dugre",
          author_email="mathieu.dugre@mdugre.info",
          ext_modules=[Extension(
              "CryptoLW",
              [
                  path.join(path_libs, "Crypto/Crypto.cpp"),
                  path.join(path_libs, "Crypto/Cipher.cpp"),
                  path.join(path_libs, "Crypto/AuthenticatedCipher.cpp"),
                  path.join(path_libs, "CryptoLW/src/Acorn128.cpp"),
                  path.join(path_libs, "CryptoLW/src/Ascon128.cpp"),
                  "PythonWrapper.cpp",
              ],
              libraries=[BOOST_LIB],
              include_dirs=[
                  path.join(path_libs, "Crypto"),
                  path.join(path_libs, "Crypto/utility"),
                  path.join(path_libs, "CryptoLW/src"),
              ]
          )])


if __name__ == "__main__":
    main()
