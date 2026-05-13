

#ifndef CORE_SECTION_H
#define CORE_SECTION_H


#define SECTION_DATA(s)        __attribute__((section("."#s),used,no_reorder))
#define SECTION_DEFINE(s, T)   SECTION_IMPORT(s, T)
#define SECTION_IMPORT(s, T)   extern const T _##s##_start[]; extern const T _##s##_end[];
#define SECTION_FOREACH(s, x)  for (auto x = _##s##_start; x != _##s##_end; x++)


#endif


