
.PHONY: all clean check

TOPDIR = ../..
SRCDIR = $(TOPDIR)/libraries/Crypto
SRCDIR2 = $(TOPDIR)/libraries/NewHope
SRCDIR3 = $(TOPDIR)/libraries/CryptoLW
SRCDIR4 = $(TOPDIR)/libraries/CryptoLegacy

#VPATH = $(SRCDIR)
vpath %.cpp $(SRCDIR)
vpath %.cpp $(SRCDIR2)
vpath %.cpp $(SRCDIR3)/src
vpath %.cpp $(SRCDIR4)/src
vpath %.o .
vpath %.ino $(SRCDIR)/examples
vpath %.ino $(SRCDIR2)/examples
vpath %.ino $(SRCDIR3)/examples
vpath %.ino $(SRCDIR4)/examples
vpath %.sketch .

LIBRARY = libCrypto.a

CPPFLAGS = \
	-I$(TOPDIR)/host/emulation \
	-I$(TOPDIR)/libraries/Crypto \
	-I$(TOPDIR)/libraries/CryptoLW/src \
	-I$(TOPDIR)/libraries/CryptoLegacy/src \
	-I$(TOPDIR)/libraries/NewHope \
	-DHOST_BUILD

CXXFLAGS = -g -Wall $(CPPFLAGS)

SOURCES =   \
	Acorn128.cpp \
	AES128.cpp \
	AES192.cpp \
	AES256.cpp \
	AESCommon.cpp \
	Ascon128.cpp \
	AuthenticatedCipher.cpp \
	BigNumberUtil.cpp \
	BLAKE2b.cpp \
	BLAKE2s.cpp \
	BlockCipher.cpp \
	CBC.cpp \
	CFB.cpp \
	ChaCha.cpp \
	ChaChaPoly.cpp \
	Cipher.cpp \
	Crypto.cpp \
	CTR.cpp \
	Curve25519.cpp \
	EAX.cpp \
	Ed25519.cpp \
	GCM.cpp \
	GF128.cpp \
	GHASH.cpp \
	Hash.cpp \
        HKDF.cpp \
	KeccakCore.cpp \
        NewHope.cpp \
	NoiseSource.cpp \
	OFB.cpp \
	OMAC.cpp \
        P521.cpp \
	Poly1305.cpp \
        RNG_host.cpp \
	SHA1.cpp \
	SHA224.cpp \
	SHA256.cpp \
	SHA384.cpp \
	SHA3.cpp \
	SHA512.cpp \
	SHAKE.cpp \
	Speck.cpp \
	SpeckSmall.cpp \
	SpeckTiny.cpp \
	XOF.cpp \
	XTS.cpp

SKETCHES = \
	TestAcorn/TestAcorn.ino \
	TestAES/TestAES.ino \
	TestAESTiny/TestAESTiny.ino \
	TestAESSmall/TestAESSmall.ino \
	TestAscon/TestAscon.ino \
	TestBigNumberUtil/TestBigNumberUtil.ino \
	TestBLAKE2b/TestBLAKE2b.ino \
	TestBLAKE2s/TestBLAKE2s.ino \
	TestCBC/TestCBC.ino \
	TestCFB/TestCFB.ino \
	TestChaCha/TestChaCha.ino \
	TestChaChaPoly/TestChaChaPoly.ino \
	TestCTR/TestCTR.ino \
	TestCurve25519/TestCurve25519.ino \
	TestCurve25519Math/TestCurve25519Math.ino \
	TestEAX/TestEAX.ino \
	TestEd25519/TestEd25519.ino \
	TestHKDF/TestHKDF.ino \
	TestGCM/TestGCM.ino \
	TestGHASH/TestGHASH.ino \
	TestNewHope/TestNewHope.ino \
	TestOFB/TestOFB.ino \
	TestP521/TestP521.ino \
	TestP521Math/TestP521Math.ino \
	TestPoly1305/TestPoly1305.ino \
	TestSHA1/TestSHA1.ino \
	TestSHA224/TestSHA224.ino \
	TestSHA256/TestSHA256.ino \
	TestSHA384/TestSHA384.ino \
	TestSHA3_256/TestSHA3_256.ino \
	TestSHA3_512/TestSHA3_512.ino \
	TestSHA512/TestSHA512.ino \
	TestSHAKE128/TestSHAKE128.ino \
	TestSHAKE256/TestSHAKE256.ino \
	TestSpeck/TestSpeck.ino \
	TestXTS/TestXTS.ino \

OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
DEPS    = $(patsubst %.cpp,.depend/%.d,$(SOURCES))

SKETCH_OUTPUTS = $(patsubst %.ino,%.sketch,$(SKETCHES))

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(RM) $(LIBRARY)
	$(AR) cr $(LIBRARY) $(OBJECTS)

clean:
	$(RM) $(OBJECTS) $(LIBRARY)
	$(RM) $(SKETCH_OUTPUTS)
	$(RM) -r .depend Test*

check: all $(SKETCH_OUTPUTS)
	@for sketch in $(SKETCH_OUTPUTS); do \
		echo Running $$sketch; \
		$$sketch | grep -i fail; \
	done; exit 0

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.sketch: %.ino $(LIBRARY)
	mkdir -p `dirname $@`
	$(CXX) -x c++ $(CXXFLAGS) \
		-include $(TOPDIR)/host/emulation/Arduino.h \
		-include $(TOPDIR)/host/emulation/Arduino.cpp \
		-o $@ $< -L. -lCrypto

.depend/%.d: %.cpp
	@set -e; rm -f $@; mkdir -p `dirname $@`; \
	$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\(.*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DEPS)
