#ifndef UTF8_SJIS_H
#define UTF8_SJIS_H

typedef struct {int utf8;int sjis;} t_utf8_sjis_xlate;
extern const int sjis_xlate_entries;
extern const t_utf8_sjis_xlate sjis_xlate[];
#endif        //  #ifndef UTF8_SJIS_H

