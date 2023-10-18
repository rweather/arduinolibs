#ifndef INLINES_H
#define INLINES_H

extern bool secure_compare(const void *data1, const void *data2, size_t len);
extern void clean(void *dest, size_t size);


template <typename T>
inline void clean(T &var)
{
    clean(&var, sizeof(T));
}
#endif