//Supports Structures, Primitives, calloc'd, dalloc'd objects, VOID pointers, Pointers to Pointers(Multi-Level Indirections), 

//modelling of struct DB for MLD lib

#ifndef MLD_H
#define MLD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* struct Database Definition Begin */
//struct db is modeled as a linked list of struct records here

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

//to print the field info, like the field name, data type, size, offset, nested structure name, address of the field object using the field offset, filed size & nested structure name

#define OFFSET_OFF(structure_name, field_name) \
    ((size_t) &(((structure_name *)0)->field_name))

#define FIELD_SIZE(structure_name, field_name) \
    sizeof(((structure_name *)0)->field_name)

struct StructureDb {
    StructureDbRecord *head;
    unsigned int count;
};

//struct registration helper macros

//FIELD_INFO macro is used to register the fields of the structure, array of FieldInfo is passed to REGISTER_struct macro
#define FIELD_INFO(structure_name, field_name, data_type, nested_structure_name) \
    {#field_name, data_type, FIELD_SIZE(structure_name, field_name), OFFSET_OFF(structure_name, field_name), #nested_structure_name}

//REGISTER_struct macro is used to register the struct in the struct database
#define REGISTER_STRUCTURE(StructureDb, struct_name, fields_array)\
    do{\
        StructureDbRecord *record = calloc(1, sizeof(StructureDbRecord));\
        strncpy(record->structure_name, #struct_name, MAX_STRUCTURE_NAME_LENGTH);\
        record->structure_size = sizeof(struct_name);\
        record->field_count = sizeof(fields_array) / sizeof(FieldInfo);\
        record->fields = fields_array;\
        if(add_structure_to_database(StructureDb, record)){\
            assert(0);\
        }\
    }while(0);

void print_structure_record(StructureDbRecord *structure_record);

void print_structure_database(StructureDb *StructureDb);

int add_structure_to_database(StructureDb *StructureDb, StructureDbRecord *structure_record); //returns 0 on success, -1 on failure

StructureDbRecord *struct_db_lookup(StructureDb *struct_db, char *structure_name);

/* struct Database Definition Ends */

/* Object Database Definition Begin */

//similar to struct db, object db is modeled as a linked list of object records, to keep track of all the objects in the system, including the dynamically allocated objects

typedef struct ObjectDbRecord ObjectDbRecord;

typedef struct ObjectDb ObjectDb;

typedef enum {
    MLD_FALSE,
    MLD_TRUE
} MldBoolean;

struct ObjectDbRecord {
    ObjectDbRecord *next;
    void *pointer; //pointer to the object
    unsigned int units; //number of units of the object
    StructureDbRecord *structure_record; //pointer to the struct record of the object
    MldBoolean is_visited; //used for graph traversal
    MldBoolean is_root; //is this object a root object?
};

struct ObjectDb {
    StructureDb *struct_db;
    ObjectDbRecord *head;
    unsigned int count;
};

// void *xcalloc(ObjectDb *object_db, char *structure_name, int units); //API to malloc the object

/*
 xcallocdoes the following :
 1. Allocate “units” units of contiguous memory for object of type “struct_name”
 2. Create the object record for new allocated object, and add the object record in object 
database
 3. Link the object record with struct record for struct “struct_name”
 4. Return the pointer to the allocated object
 Thus, xcallocallocates memory for the object, but also create internal data struct in MLD
 Library so that MLD can keep track of the newly allocated object
*/

// void xfree(ObjectDb *object_db, void *pointer); //API to free the object

void print_object_record(ObjectDbRecord *object_record);

void print_object_database(ObjectDb *object_db);

/* Object Database Definition Ends */

/*mld algorithm starts here*/

/*Application can create root objects in two ways 
 Global root object
 Dynamic root object
*/

void register_globall_object_as_root(ObjectDb *object_db, void *object_ptr, char *structure_name, unsigned int units); //Create a new object dbrecord entry in object db of MLD library, mark it as root

void set_dynamic_object_as_root(ObjectDb *object_db, void *object_ptr);// Search an existing object dbrecord entry in object dbof MLD library, mark it as root

/*
exactly how mld algo works:
1. Initialize MLD algorithm // given object db, is_visited = false for all objects
2. Run MLD algorithm // iterate over all root objects recursively, dfs, explore all objects reachable from root objects, mark them as visited
3. Print Leaked Objects // iterate over all objects in object db, print objects which are not visited
*/

void run_mld_algorithm(ObjectDb *object_db);

void init_primitive_data_types_support(StructureDb *struct_db);

void report_leaked_objects(ObjectDb *object_db);

// void mld_dump_object_rec_detail(ObjectDbRecord *object_record);

#endif

#ifdef TRACE

#define xcalloc(object_db, structure_name, units) \
    xcalloc_with_trace(object_db, structure_name, units, __FILE__, __LINE__)

#define add_object_to_object_db(object_db, pointer, units, struct_rec, boolean_is_root) \
    add_object_to_object_db_with_trace(object_db, pointer, units, struct_rec, boolean_is_root, __FILE__, __LINE__)

#define mld_dump_object_rec_detail(object_record) \
    mld_dump_object_rec_detail_with_trace(object_record, __FILE__, __LINE__)

#define xmalloc(object_db, structure_name, units) \
    xmalloc_with_trace(object_db, structure_name, units, __FILE__, __LINE__)

#define xfree(object_db, pointer) \
    xfree_with_trace(object_db, pointer, __FILE__, __LINE__)

// Trace-enabled prototypes
void *xcalloc_with_trace(ObjectDb *object_db, char *structure_name, int units, const char *file, int line);
void add_object_to_object_db_with_trace(ObjectDb *object_db, void *pointer, unsigned int units, StructureDbRecord *struct_rec, MldBoolean boolean_is_root, const char *file, int line);
void mld_dump_object_rec_detail_with_trace(ObjectDbRecord *object_record, const char *file, int line);
void *xmalloc_with_trace(ObjectDb *object_db, char *structure_name, int units, const char *file, int line);
void xfree_with_trace(ObjectDb *object_db, void *pointer, const char *file, int line);

#else // Non-trace version

void *xcalloc(ObjectDb *object_db, char *structure_name, int units);
void add_object_to_object_db(ObjectDb *object_db, void *pointer, unsigned int units, StructureDbRecord *struct_rec, MldBoolean boolean_is_root);
void mld_dump_object_rec_detail(ObjectDbRecord *object_record);
void *xmalloc(ObjectDb *object_db, char *structure_name, int units);
void xfree(ObjectDb *object_db, void *pointer);

#endif
