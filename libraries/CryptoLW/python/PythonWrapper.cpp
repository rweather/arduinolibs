// #include <stdexcept>
#include <boost/python.hpp>
#include "Acorn128.h"
#include "Ascon128.h"
namespace bp = boost::python;

// Wrapper for Python using Boost.Python (boost.org)

// ----------------------------
// Shamelessly stolen from pyRF24 (https://github.com/nRF24/RF24)
void throw_ba_exception(void)
{
    PyErr_SetString(PyExc_TypeError, "buf parameter must be bytes or bytearray");
    bp::throw_error_already_set();
};

char* get_bytes_or_bytearray_str(bp::object buf)
{
    PyObject* py_ba;
    py_ba = buf.ptr();
    if (PyByteArray_Check(py_ba)) {
        return PyByteArray_AsString(py_ba);
    } else if (PyBytes_Check(py_ba)) {
        return PyBytes_AsString(py_ba);
    } else {
        throw_ba_exception();
    }
    return NULL;
};

int get_bytes_or_bytearray_ln(bp::object buf)
{
    PyObject* py_ba;
    py_ba = buf.ptr();
    if (PyByteArray_Check(py_ba)) {
        return PyByteArray_Size(py_ba);
    } else if (PyBytes_Check(py_ba)) {
        return PyBytes_Size(py_ba);
    } else {
        throw_ba_exception();
    }
    return 0;
};
// ----------------------------

bool setKey_wrap(AuthenticatedCipher& ref, bp::object buf)
{
    const char* buffer = get_bytes_or_bytearray_str(buf);
    return ref.setKey((uint8_t*)buffer, get_bytes_or_bytearray_ln(buf));
};

bool setIV_wrap(AuthenticatedCipher& ref, bp::object buf)
{
    const char* buffer = get_bytes_or_bytearray_str(buf);
    return ref.setIV((uint8_t*)buffer, get_bytes_or_bytearray_ln(buf));
};

void addAuthData_wrap(AuthenticatedCipher& ref, bp::object buf)
{
    const char* buffer = get_bytes_or_bytearray_str(buf);
    ref.addAuthData((uint8_t*)buffer, get_bytes_or_bytearray_ln(buf));
};

bp::object encrypt_wrap(AuthenticatedCipher& ref, bp::object buf)
{
    const char* inputBuffer = get_bytes_or_bytearray_str(buf);

    // Recuperer la taille du buffer, identique pour output.
    int len = get_bytes_or_bytearray_ln(buf);

    // Creer un buffer d'output sur le heap et chiffrer
    char* outputBuffer = new char[len + 1];
    ref.encrypt((uint8_t*) outputBuffer, (uint8_t*) inputBuffer, len);

    // Convertir le buffer d'output en objet Python
    bp::object py_ba(bp::handle<>( PyByteArray_FromStringAndSize(outputBuffer, len) ));

    delete[] outputBuffer;  // Cleanup heap
    return py_ba;
};

bp::object decrypt_wrap(AuthenticatedCipher& ref, bp::object buf)
{
    const char* inputBuffer = get_bytes_or_bytearray_str(buf);

    // Recuperer la taille du buffer, identique pour output.
    int len = get_bytes_or_bytearray_ln(buf);

    // Creer un buffer d'output sur le heap et dechiffrer
    char* outputBuffer = new char[len + 1];
    ref.decrypt((uint8_t*) outputBuffer, (uint8_t*) inputBuffer, len);

    // Convertir le buffer d'output en objet Python
    bp::object py_ba(bp::handle<>( PyByteArray_FromStringAndSize(outputBuffer, len) ));

    delete[] outputBuffer;  // Cleanup heap
    return py_ba;
};

bp::object computeTag_wrap(AuthenticatedCipher& ref)
{
    // Compute tag, sauver un buffer temporaire
    char outputBuffer[16];
    ref.computeTag((uint8_t*)&outputBuffer, sizeof(outputBuffer));

    // Convertir le buffer d'output en objet Python
    bp::object py_ba(bp::handle<>( PyByteArray_FromStringAndSize(outputBuffer, sizeof(outputBuffer)) ));

    return py_ba;
};

void checkTag_wrap(AuthenticatedCipher& ref, bp::object buf)
{
    const char* buffer = get_bytes_or_bytearray_str(buf);

    if( ! ref.checkTag((uint8_t*)buffer, get_bytes_or_bytearray_ln(buf)) )
    {
        // Comportement Python habituel, lancer une exception plutot que retourner un false
        PyErr_SetString(PyExc_ValueError, "AuthenticatedCipher: invalid tag");
        bp::throw_error_already_set();
    }
};

BOOST_PYTHON_MODULE(CryptoLW)
{
    bp::class_<Cipher, boost::noncopyable>("Cipher", bp::no_init);

    bp::class_<AuthenticatedCipher, bp::bases<Cipher>, boost::noncopyable>("AuthenticatedCipher", bp::no_init);

    bp::class_<Acorn128, bp::bases<AuthenticatedCipher>>("Acorn128")
        .def("keySize", &Acorn128::keySize)
        .def("ivSize", &Acorn128::ivSize)
        .def("tagSize", &Acorn128::tagSize)
        .def("setKey", &setKey_wrap)
        .def("setIV", &setIV_wrap)
        .def("encrypt", &encrypt_wrap)
        .def("decrypt", &decrypt_wrap)
        .def("addAuthData", &addAuthData_wrap)
        .def("computeTag", &computeTag_wrap)
        .def("checkTag", &checkTag_wrap)
        .def("clear", &Acorn128::clear)
    ;

    bp::class_<Ascon128, bp::bases<AuthenticatedCipher>>("Ascon128")
        .def("keySize", &Ascon128::keySize)
        .def("ivSize", &Ascon128::ivSize)
        .def("tagSize", &Ascon128::tagSize)
        .def("setKey", &setKey_wrap)
        .def("setIV", &setIV_wrap)
        .def("encrypt", &encrypt_wrap)
        .def("decrypt", &decrypt_wrap)
        .def("addAuthData", &addAuthData_wrap)
        .def("computeTag", &computeTag_wrap)
        .def("checkTag", &checkTag_wrap)
        .def("clear", &Ascon128::clear)
    ;
};
