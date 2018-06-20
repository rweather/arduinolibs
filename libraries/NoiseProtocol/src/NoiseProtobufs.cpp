/*
 * Copyright (C) 2018 Southern Storm Software, Pty Ltd.
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

#include "NoiseProtobufs.h"
#include <string.h>

/**
 * \class NoiseProtobufFormatter NoiseProtobufs.h <NoiseProtobufs.h>
 * \brief Formats data into a buffer using the protobufs format.
 *
 * This class provides support for creating messages in the protobufs
 * format.  Fields in a message consist of a tag followed by a value.
 *
 * This implementation provides a subset of the full protobufs encoding,
 * limited to the types bool, uint32, uint64, sint32, sint64, string,
 * bytes, and embedded messages.  In addition, tag numbers are limited
 * to unsigned 8-bit values.
 *
 * Reference: https://developers.google.com/protocol-buffers/docs/encoding
 *
 * \sa NoiseProtobufParser
 */

/**
 * \type NoiseProtobufTag_t
 * \brief Integer type that defines a protobuf tag number (unsigned 8-bit).
 */

/**
 * \brief Constructs a new protobuf formatter.
 *
 * \param data Points to the buffer to format fields into.
 * \param maxSize Maximum size of the buffer in bytes.
 */
NoiseProtobufFormatter::NoiseProtobufFormatter(void *data, size_t maxSize)
    : buf((uint8_t *)data)
    , maxsz(maxSize)
    , pos(0)
    , err(false)
{
}

/**
 * \brief Destroys this protobuf formatter.
 */
NoiseProtobufFormatter::~NoiseProtobufFormatter()
{
}

/**
 * \fn size_t NoiseProtobufFormatter::size() const
 * \brief Returns the final size of the data after all fields have been added.
 *
 * \return The number of bytes that were written to the buffer.
 *
 * If an error has occurred, then size() will be invalid.
 *
 * \sa error()
 */

/**
 * \fn bool NoiseProtobufFormatter::error() const
 * \brief Determine if an error occurred during formatting.
 *
 * \return Returns true if the buffer ran out of space while trying to
 * write the fields; false if formatting was successful.
 *
 * \sa size()
 */

/**
 * \fn void NoiseProtobufFormatter::addBool(NoiseProtobufTag_t tag, bool value)
 * \brief Adds a boolean field.
 *
 * \param tag Tag number for the field.
 * \param value Boolean value for the field.
 */

/**
 * \fn void NoiseProtobufFormatter::addUInt32(NoiseProtobufTag_t tag, uint32_t value)
 * \brief Adds an unsigned 32-bit integer field.
 *
 * \param tag Tag number for the field.
 * \param value Unsigned 32-bit value for the field.
 */

/**
 * \fn void NoiseProtobufFormatter::addUInt64(NoiseProtobufTag_t tag, uint64_t value)
 * \brief Adds an unsigned 64-bit integer field.
 *
 * \param tag Tag number for the field.
 * \param value Unsigned 64-bit value for the field.
 */

/**
 * \fn void NoiseProtobufFormatter::addInt32(NoiseProtobufTag_t tag, int32_t value)
 * \brief Adds a signed 32-bit integer field.
 *
 * \param tag Tag number for the field.
 * \param value Signed 32-bit value for the field.
 *
 * This function uses the ZigZag encoding for the signed integer value.
 */

/**
 * \fn void NoiseProtobufFormatter::addInt64(NoiseProtobufTag_t tag, int64_t value)
 * \brief Adds a signed 64-bit integer field.
 *
 * \param tag Tag number for the field.
 * \param value Signed 64-bit value for the field.
 *
 * This function uses the ZigZag encoding for the signed integer value.
 */

/**
 * \brief Adds an arbitrary byte array as a field.
 *
 * \param tag Tag number for the field.
 * \param data Points to the array of bytes for the field's value.
 * \param len Number of bytes in the array.
 */
void NoiseProtobufFormatter::addBytes
    (NoiseProtobufTag_t tag, const void *data, size_t len)
{
    if (!data && len > 0) {
        err = true;
        return;
    }
    addHeader(tag, 2);
    if (sizeof(len) <= 4)
        addVarUInt32(len);
    else
        addVarUInt64(len);
    if (!err) {
        if (len <= (maxsz - pos)) {
            memcpy(buf + pos, data, len);
            pos += len;
        } else {
            err = true;
        }
    }
}

/**
 * \brief Adds a NUL-terminated string as a field.
 *
 * \param tag Tag number for the field.
 * \param str String value for the field, NUL-terminated.
 *
 * If \a str is NULL, then an empty string will be added.
 */
void NoiseProtobufFormatter::addString(NoiseProtobufTag_t tag, const char *str)
{
    addBytes(tag, str, str ? strlen(str) : 0);
}

/**
 * \fn void NoiseProtobufFormatter::addString(NoiseProtobufTag_t tag, const char *str, size_t len)
 * \brief Adds a length-delimited string as a field.
 *
 * \param tag Tag number for the field.
 * \param str String value for the field.
 * \param len Number of bytes in the string value.
 */

#define ADD_BYTE(b) \
    do { \
        if (!err) { \
            if (pos < maxsz) { \
                buf[pos++] = (uint8_t)(b); \
            } else { \
                err = true; \
            } \
        } \
    } while (0)

/**
 * \brief Starts an embedded message.
 *
 * \param tag Tag number for the embedded message.
 *
 * \return A reference to the start of the embedded message, to be passed
 * to endEmbedded() when the application has finished adding nested fields.
 *
 * \note The size of the embedded data is limited to no more than 65535 bytes,
 * to simplify the implementation of endEmbedded().  Since Noise messages are
 * also limited to no more than 65535 bytes, this shouldn't be a problem
 * in practice.
 *
 * \sa endEmbedded()
 */
size_t NoiseProtobufFormatter::startEmbedded(NoiseProtobufTag_t tag)
{
    addHeader(tag, 2);
    size_t start = pos;
    ADD_BYTE(0); // Placeholder to be filled in by endEmbedded().
    return start;
}

/**
 * \brief Ends an embedded message and returns to the previous level.
 *
 * \param start The value that was returned from startEmbedded() at the
 * beginning of the embedded message.
 *
 * \sa startEmbedded()
 */
void NoiseProtobufFormatter::endEmbedded(size_t start)
{
    // If an error already occurred, then bail out.
    if (err)
        return;

    // If the starting position is invalid, then bail out.
    if (start >= pos) {
        err = true;
        return;
    }

    // Determine the actual length of the embedded message.
    size_t len = pos - start - 1;

    // Encode the length as 1, 2, or 3 bytes.  For the 2 and 3 byte
    // versions we need to shift the message data up to make room.
    if (len < (1U << 7)) {
        buf[start] = (uint8_t)len;
    } else if (len < (1U << 14)) {
        if (pos < maxsz) {
            memmove(buf + start + 2, buf + start + 1, len);
            ++pos;
            buf[start] = (uint8_t)(len | 0x80);
            buf[start + 1] = (uint8_t)(len >> 7);
        } else {
            err = true;
        }
    } else if (len < (1U << 16)) {
        if ((pos + 1) < maxsz) {
            memmove(buf + start + 3, buf + start + 1, len);
            pos += 2;
            buf[start] = (uint8_t)(len | 0x80);
            buf[start + 1] = (uint8_t)((len >> 7) | 0x80);
            buf[start + 2] = (uint8_t)(len >> 14);
        } else {
            err = true;
        }
    } else {
        err = true;
    }
}

/**
 * \fn void NoiseProtobufFormatter::addHeader(NoiseProtobufTag_t tag, uint8_t wireType)
 * \brief Adds a protobuf tag header.
 *
 * \param tag Tag number for the field.
 * \param wireType Protobuf wire type for the field; 0 for integer, 2 for
 * length-delimited.
 */

/**
 * \brief Adds an unsigned 14-bit value as a varint.
 *
 * \param value Unsigned 14-bit value to add.
 */
void NoiseProtobufFormatter::addVarUInt14(uint16_t value)
{
    if (value < (1U << 7)) {
        ADD_BYTE(value);
    } else {
        ADD_BYTE(value | 0x80);
        ADD_BYTE(value >> 7);
    }
}

/**
 * \brief Adds an unsigned 32-bit value as a varint.
 *
 * \param value Unsigned 32-bit value to add.
 */
void NoiseProtobufFormatter::addVarUInt32(uint32_t value)
{
    if (value < (1U << 7)) {
        ADD_BYTE(value);
    } else if (value < (1U << 14)) {
        ADD_BYTE(value | 0x80);
        ADD_BYTE(value >> 7);
    } else if (value < (1U << 21)) {
        ADD_BYTE(value | 0x80);
        ADD_BYTE((value >> 7) | 0x80);
        ADD_BYTE(value >> 14);
    } else if (value < (1U << 28)) {
        ADD_BYTE(value | 0x80);
        ADD_BYTE((value >> 7) | 0x80);
        ADD_BYTE((value >> 14) | 0x80);
        ADD_BYTE(value >> 21);
    } else {
        ADD_BYTE(value | 0x80);
        ADD_BYTE((value >> 7) | 0x80);
        ADD_BYTE((value >> 14) | 0x80);
        ADD_BYTE((value >> 21) | 0x80);
        ADD_BYTE(value >> 28);
    }
}

/**
 * \brief Adds an unsigned 64-bit value as a varint.
 *
 * \param value Unsigned 64-bit value to add.
 */
void NoiseProtobufFormatter::addVarUInt64(uint64_t value)
{
    if (value < 0x100000000ULL) {
        addVarUInt32((uint32_t)value);
    } else {
        while (value >= (1ULL << 7)) {
            ADD_BYTE(value | 0x80);
            value >>= 7;
        }
        ADD_BYTE(value);
    }
}

/**
 * \class NoiseProtobufParser NoiseProtobufs.h <NoiseProtobufs.h>
 * \brief Parses data from a buffer using the protobufs format.
 *
 * This class provides support for parsing messages in the protobufs
 * format.  Fields in a message consist of a tag followed by a value.
 *
 * This implementation provides a subset of the full protobufs encoding,
 * limited to the types bool, uint32, uint64, sint32, sint64, string,
 * bytes, and embedded messages.  In addition, tag numbers are limited
 * to unsigned 8-bit values.
 *
 * Reference: https://developers.google.com/protocol-buffers/docs/encoding
 *
 * \sa NoiseProtobufFormatter
 */

/**
 * \brief Constructs a new protobuf parser for reading the contents
 * of a buffer.
 *
 * \param data Points to the buffer to parse.
 * \param size Size of the buffer in bytes.
 */
NoiseProtobufParser::NoiseProtobufParser(const void *data, size_t size)
    : buf((const uint8_t *)data)
    , pos(0)
    , end(size)
    , ptr(0)
    , value(0)
    , wire(0xFF)
    , err(false)
{
}

/**
 * \brief Destroys this protobuf parser.
 */
NoiseProtobufParser::~NoiseProtobufParser()
{
}

/**
 * \fn bool NoiseProtobufParser::error() const
 * \brief Determine if an error occurred during parsing.
 *
 * \return Returns true if malformed data was found while parsing,
 * false if parsing was successful.
 */

/**
 * \brief Reads the next field from a protobuf-formatted message.
 *
 * \param tag Returns the tag number for the next field.
 *
 * \return Returns true if a new field was found, or false at the end
 * of the message.  Also returns false if an error occurs.
 */
bool NoiseProtobufParser::readNext(NoiseProtobufTag_t &tag)
{
    tag = 0;
    if (err || pos >= end)
        return false;
    uint64_t header = readVarint();
    if (err)
        return false;
    if (header >= (256U << 3)) {
        err = true;
        return false;
    }
    tag = (NoiseProtobufTag_t)(header >> 3);
    wire = (uint8_t)(header & 0x07);
    if (wire == 0) {
        value = readVarint();
        return !err;
    } else if (wire == 2) {
        value = readVarint();
        if (err)
            return false;
        if (value > (end - pos)) {
            err = true;
            return false;
        }
        ptr = pos;
        pos += (size_t)value;
        return true;
    } else {
        err = true;
        return false;
    }
}

/**
 * \brief Reads a boolean value from the current field.
 *
 * \return The boolean value in the current field.
 *
 * This function will raise an error() if the current field is not boolean.
 */
bool NoiseProtobufParser::readBool()
{
    if (err)
        return false;
    if (wire != 0) {
        err = true;
        return false;
    }
    return value != 0;
}

/**
 * \brief Reads an unsigned 32-bit integer value from the current field.
 *
 * \return The integer value in the current field.
 *
 * This function will raise an error() if the current field is not an
 * integer or the value is out of range.
 *
 * \sa readUInt64(), readInt32()
 */
uint32_t NoiseProtobufParser::readUInt32()
{
    if (err)
        return 0;
    if (wire != 0 || value >= 0x100000000ULL) {
        err = true;
        return false;
    }
    return (uint32_t)value;
}

/**
 * \brief Reads an unsigned 64-bit integer value from the current field.
 *
 * \return The integer value in the current field.
 *
 * This function will raise an error() if the current field is not an integer.
 *
 * \sa readUInt32(), readInt64()
 */
uint64_t NoiseProtobufParser::readUInt64()
{
    if (err)
        return 0;
    if (wire != 0) {
        err = true;
        return false;
    }
    return value;
}

/**
 * \brief Reads a signed 32-bit integer value from the current field.
 *
 * \return The integer value in the current field.
 *
 * This function uses the ZigZag encoding for the signed integer value.
 * An error() is raised if the current field is not an integer or the value
 * is out of range.
 *
 * \sa readInt64(), readUInt32()
 */
int32_t NoiseProtobufParser::readInt32()
{
    if (err)
        return 0;
    if (wire != 0 || value >= 0x100000000ULL) {
        err = true;
        return false;
    }
    return ((int32_t)(value >> 1)) ^ (-((int32_t)(value & 0x01)));
}

/**
 * \brief Reads a signed 64-bit integer value from the current field.
 *
 * \return The integer value in the current field.
 *
 * This function uses the ZigZag encoding for the signed integer value.
 * An error() is raised if the current field is not an integer.
 *
 * \sa readInt32(), readUInt64()
 */
int64_t NoiseProtobufParser::readInt64()
{
    if (err)
        return 0;
    if (wire != 0) {
        err = true;
        return false;
    }
    return ((int64_t)(value >> 1)) ^ (-((int64_t)(value & 0x01)));
}

/**
 * \brief Reads a string value from the current field.
 *
 * \param str Returns a pointer to the start of the string.  This points
 * directly into the original buffer and will not be NUL-terminated.
 *
 * \return The length of the string in bytes.
 *
 * This function will raise an error() if the current field is not
 * length-delimited.
 *
 * \sa readBytes()
 */
size_t NoiseProtobufParser::readString(const char * &str)
{
    if (err) {
        str = 0;
        return 0;
    }
    if (wire != 2) {
        err = true;
        str = 0;
        return 0;
    }
    str = (const char *)(buf + ptr);
    return (size_t)value;
}

/**
 * \brief Reads a byte array from the current field.
 *
 * \param str Returns a pointer to the start of the byte array.
 *
 * \return The length of the byte array in bytes.
 *
 * This function will raise an error() if the current field is not
 * length-delimited.
 *
 * \sa readString()
 */
size_t NoiseProtobufParser::readBytes(const void * &data)
{
    if (err) {
        data = 0;
        return 0;
    }
    if (wire != 2) {
        err = true;
        data = 0;
        return 0;
    }
    data = (const void *)(buf + ptr);
    return (size_t)value;
}

/**
 * \brief Starts parsing an embedded message.
 *
 * \return A value to pass to endEmbedded() to pop out a level at the
 * end of the embedded message.
 *
 * This function will raise an error() if the current field is not
 * length-delimited.
 *
 * The following is an example of descending into an embedded message
 * to parse all of the fields, and then returning to the previous
 * message level at the end:
 *
 * \code
 * size_t posn = parser.startEmbedded();
 * while (parser.readNext(tag)) {
 *     ...
 * }
 * parser.endEmbedded(posn);
 * \endcode
 *
 * \sa endEmbedded()
 */
size_t NoiseProtobufParser::startEmbedded()
{
    if (err)
        return 0;
    if (wire != 2) {
        err = true;
        return 0;
    }
    size_t prev = end;
    end = pos;
    pos = ptr;
    wire = 0xFF;
    return prev;
}

/**
 * \brief Ends parsing of an embedded message.
 *
 * \param posn The position in the outer message to return to.  This must be
 * the return value from startEmbedded() for the same nesting level.
 *
 * \sa startEmbedded()
 */
void NoiseProtobufParser::endEmbedded(size_t posn)
{
    if (err)
        return;
    if (posn < end) {
        err = true;
        return;
    }
    pos = end;
    end = posn;
    wire = 0xFF;
}

/**
 * \brief Reads a variable-length integer from the message.
 *
 * \return The integer value.
 */
uint64_t NoiseProtobufParser::readVarint()
{
    uint64_t value = 0;
    unsigned bit = 0;
    while (pos < end && (buf[pos] & 0x80) != 0) {
        value |= ((uint64_t)(buf[pos] & 0x7F)) << bit;
        bit += 7;
        if (bit >= 63) {
            err = true;
            return 0;
        }
        ++pos;
    }
    if (pos >= end) {
        err = true;
        return 0;
    }
    if (bit >= 63 && (buf[pos] & 0xFE) != 0) {
        err = true;
        return 0;
    }
    value |= ((uint64_t)(buf[pos] & 0x7F)) << bit;
    ++pos;
    return value;
}
