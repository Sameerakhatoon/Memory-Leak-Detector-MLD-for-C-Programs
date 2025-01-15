//modelling of struct DB for MLD lib, as hashmap

#ifndef MLD_H
#define MLD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
at the core, hashmap maintains array where each element os a bucket
each bucket can store one or more key-value pairs
size of array is capacity of hashmap
each bucket is a linked list of key-value pairs, to handle collisions, 

compute hash, get the index of the array, get the bucket, search the linked list in the bucket for the key, the value is pointer to record

modeling of struct DB for MLD lib, as hashmap
db has pointer to head of the array of buckets, count of the number of structures registered in the db

modeling struct db record, each record can be retrieved by name of the structure, compute hash, get the index of the array, get the bucket, search the linked list in the bucket for the key, the value is pointer to record
more on buckets: each bucket is a linked list of struct db records, to handle collisions

modelling if object db for MLD lib, as hashmap
db has pointer to head of the array of buckets, count of the number of objects registered in the db

modeling object db record, each record can be retrieved by pointer to the object, compute hash, get the index of the array, get the bucket, search the linked list in the bucket for the key, the value is pointer to record
more on buckets: each bucket is a linked list of object db records, to handle collisions
*/

/*struct db definition begins here*/

#define MAX_STRUCTURE_NAME_LENGTH 128
#define MAX_FIELD_NAME_LENGTH 128

typedef struct StructureDbRecord StructureDbRecord;

typedef struct FieldInfo FieldInfo;

typedef struct StructureDb StructureDb;

struct StructureDbRecord {
    StructureDbRecord *next;
    char structure_name[MAX_STRUCTURE_NAME_LENGTH];
    unsigned int structure_size;
    unsigned int field_count;
    FieldInfo *fields;
};

typedef enum {
    UINT8_TYPE,
    UINT32_TYPE,
    INT32_TYPE,
    CHAR_TYPE,
    FLOAT_TYPE,
    DOUBLE_TYPE,
    OBJECT_pointer_TYPE,
    OBJECT_STRUCT_TYPE,
    VOID_pointer_TYPE
} DataType;

struct FieldInfo {
    char field_name[MAX_FIELD_NAME_LENGTH];
    DataType data_type;
    unsigned int size;
    unsigned int offset;
    char nested_structure_name[MAX_STRUCTURE_NAME_LENGTH];
};

#define OFFSET_OFF(structure_name, field_name) \
    ((size_t) &(((structure_name *)0)->field_name))

#define FIELD_SIZE(structure_name, field_name) \
    sizeof(((structure_name *)0)->field_name)

struct StructureDb {
    StructureDbRecord *structutre_db_arr[100];
    int count;
};

#define FIELD_INFO(structure_name, field_name, data_type, nested_structure_name) \
    {#field_name, data_type, FIELD_SIZE(structure_name, field_name), OFFSET_OFF(structure_name, field_name), #nested_structure_name}

#define REGISTER_STRUCTURE(struct_db, struct_name, fields_array) \
    do { \
        StructureDbRecord *record = calloc(1, sizeof(StructureDbRecord)); \
        strncpy(record->structure_name, #struct_name, MAX_STRUCTURE_NAME_LENGTH); \
        record->structure_size = sizeof(struct_name); \
        printf("fields_array: %p\n", fields_array); \
        printf("sizeof(fields_array): %lu\n", sizeof(fields_array)); \
        printf("size of FieldInfo: %lu\n", sizeof(FieldInfo)); \
        record->field_count = sizeof(fields_array) / sizeof(FieldInfo); \
        printf("field_count: %d\n", record->field_count); \
        record->fields = fields_array; \
        if(add_structure_to_database(struct_db, record)) { \
            assert(0); \
        } \
    } while(0);

void print_structure_record(StructureDbRecord *structure_record);

void print_structure_database(StructureDb *struct_db);

int add_structure_to_database(StructureDb *struct_db, StructureDbRecord *structure_record);

StructureDbRecord *struct_db_lookup(StructureDb *struct_db, char *structure_name);

/*struct db definition ends*/

/*object db definition begins here*/

typedef struct ObjectDbRecord ObjectDbRecord;

typedef struct ObjectDb ObjectDb;

typedef enum {
    MLD_FALSE,
    MLD_TRUE
} MldBoolean;

struct ObjectDbRecord {
    ObjectDbRecord *next;
    void *pointer;
    unsigned int units;
    StructureDbRecord *structure_record;
    MldBoolean is_visited;
    MldBoolean is_root;
};

struct ObjectDb {
    ObjectDbRecord *object_db_arr[100];
    StructureDb *struct_db;
    int count;
};

void print_object_record(ObjectDbRecord *object_record);

void print_object_database(ObjectDb *object_db);

void register_global_object_as_root(ObjectDb *object_db, void *object_ptr, char *structure_name, unsigned int units);

void set_dynamic_object_as_root(char *structure_name, ObjectDb *object_db, void *object_ptr);

void run_mld_algorithm(ObjectDb *object_db);

void init_primitive_data_types_support(StructureDb *struct_db);

void report_leaked_objects(ObjectDb *object_db);

#endif

#ifdef TRACE

#define xcalloc(object_db, structure_name, units) \
    xcalloc_with_trace(object_db, structure_name, units, __FILE__, __LINE__)

#define add_object_to_object_db(structure_name, object_db, pointer, units, struct_rec, boolean_is_root) \
    add_object_to_object_db_with_trace(structure_name, object_db, pointer, units, struct_rec, boolean_is_root, __FILE__, __LINE__)

#define mld_dump_object_rec_detail(object_record) \
    mld_dump_object_rec_detail_with_trace(object_record, __FILE__, __LINE__)

#define xmalloc(object_db, structure_name, units) \
    xmalloc_with_trace(object_db, structure_name, units, __FILE__, __LINE__)

#define xfree(structure_name, object_db, pointer) \
    xfree_with_trace(structure_name, object_db, pointer, __FILE__, __LINE__)

// Trace-enabled prototypes
void *xcalloc_with_trace(ObjectDb *object_db, char *structure_name, int units, const char *file, int line);
void add_object_to_object_db_with_trace(char *structure_name, ObjectDb *object_db, void *pointer, unsigned int units, StructureDbRecord *struct_rec, MldBoolean boolean_is_root, const char *file, int line);
void mld_dump_object_rec_detail_with_trace(ObjectDbRecord *object_record, const char *file, int line);
void *xmalloc_with_trace(ObjectDb *object_db, char *structure_name, int units, const char *file, int line);
void xfree_with_trace(char *structure_name, ObjectDb *object_db, void *pointer, const char *file, int line);

#else // Non-trace version

void *xcalloc(ObjectDb *object_db, char *structure_name, int units);
void add_object_to_object_db(char *structure_name, ObjectDb *object_db, void *pointer, unsigned int units, StructureDbRecord *struct_rec, MldBoolean boolean_is_root);
void mld_dump_object_rec_detail(ObjectDbRecord *object_record);
void *xmalloc(ObjectDb *object_db, char *structure_name, int units);
void xfree(char *structure_name, ObjectDb *object_db, void *pointer);

#endif