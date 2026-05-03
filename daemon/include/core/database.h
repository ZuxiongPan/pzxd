#ifndef __DATABASE_H__
#define __DATABASE_H__

#define KEYNAME_MAXLEN 128
#define STRVALUE_MAXLEN 256

enum datatype {
    DBTYPE_INVALID = 0,
    DBTYPE_BOOL,
    DBTYPE_INT,
    DBTYPE_STRING,
    DBTYPE_MAX,
};

int database_init(void);
void database_destroy(void);
int database_get(const char *key, enum datatype type, void *value, int size);
int database_set(const char *key, enum datatype type, const void *value);

#endif