//implementing the functions declared in mld.h

#include "mld.h"

/*
as dbs are modeled as hashmaps, the functions to add a structure to the db, lookup a structure in the db, print a structure record, print the db, are implemented here
*/

//table size is a prime number
#define TABLE_SIZE 101

//hash function is a polynomial rolling hash function, given string as input, returns a hash value which is an integer
unsigned int polynonial_rolling_hash(const char *key){
    unsigned int hash = 0;
    unsigned int p_pow = 1;

    for(int i = 0; i<strlen(key) && MAX_STRUCTURE_NAME_LENGTH; i++){
        hash = (hash + (key[i] * p_pow)) % TABLE_SIZE;
        p_pow = (p_pow * 31) % TABLE_SIZE;
    }
    return hash;
}

char *DataTypes[] = {
    "UINT8_TYPE",
    "UINT32_TYPE",
    "INT32_TYPE",
    "CHAR_TYPE",
    "FLOAT_TYPE",
    "DOUBLE_TYPE",
    "OBJECT_pointer_TYPE",
    "OBJECT_STRUCT_TYPE",
    "VOID_pointer_TYPE"
};

void print_structure_record(StructureDbRecord *structure_record) {
    if (!structure_record) return;

    printf("\n|------------------------------------------------------|\n");
    printf("| Structure Name : %-35s |\n", structure_record->structure_name);
    printf("| Size           : %-35u |\n", structure_record->structure_size);
    printf("| Field Count    : %-35u |\n", structure_record->field_count);
    printf("|------------------------------------------------------|\n");

    printf("| %-3s | %-20s | %-15s | %-5s | %-6s | %-20s |\n",
           "#", "Field Name", "Data Type", "Size", "Offset", "Nested Struct");
    printf("|-----|----------------------|-----------------|-------|--------|----------------------|\n");

    for (int j = 0; j < structure_record->field_count; j++) {
        FieldInfo *field = &structure_record->fields[j];

        // Print field details
        printf("| %-3d | %-20s | %-15s | %-5u | %-6u | %-20s |\n",
               j,
               field->field_name,
               (field->data_type == UINT8_TYPE) ? "UINT8" :
               (field->data_type == UINT32_TYPE) ? "UINT32" :
               (field->data_type == INT32_TYPE) ? "INT32" :
               (field->data_type == CHAR_TYPE) ? "CHAR" :
               (field->data_type == FLOAT_TYPE) ? "FLOAT" :
               (field->data_type == DOUBLE_TYPE) ? "DOUBLE" :
               (field->data_type == OBJECT_pointer_TYPE) ? "OBJ_PTR" :
               (field->data_type == VOID_pointer_TYPE) ? "VOID_PTR" : "UNKNOWN",
               field->size,
               field->offset,
               field->nested_structure_name[0] ? field->nested_structure_name : "N/A");
    }

    printf("|------------------------------------------------------|\n\n");
}

void print_structure_database(StructureDb *struct_db){
    if(!struct_db) return;

    printf("Printing STRUCTURE DATABASE\n");

    //iterate through the hash table of structure db, to print all the structure records
    for(int i = 0; i<TABLE_SIZE-1; i++){
        StructureDbRecord *structure_record = struct_db->structutre_db_arr[i];
        for(; structure_record; structure_record = structure_record->next){
            print_structure_record(structure_record);
        }
    }

    printf("End of STRUCTURE DATABASE\n");
}

int add_structure_to_database(StructureDb *struct_db, StructureDbRecord *structure_record){
    //get the hash value of the structure name
    unsigned int hash = polynonial_rolling_hash(structure_record->structure_name);

    //add the structure record to the hash table
    if(struct_db->structutre_db_arr[hash]){
        structure_record->next = struct_db->structutre_db_arr[hash];
        struct_db->structutre_db_arr[hash] = structure_record;
    }else{
        struct_db->structutre_db_arr[hash] = structure_record;
    }
    struct_db->count++;
    return 0;
}

/*
explanation of above code, generally, to handle the collision, we can use separate chaining, which is implemented here
if the hash value is already occupied, then the new structure record is added to the head of the linked list
if the hash value is not occupied, then the new structure record is added to the hash table
*/

StructureDbRecord *struct_db_lookup(StructureDb *struct_db, char *structure_name){
    //get the hash value of the structure name
    unsigned int hash = polynonial_rolling_hash(structure_name);

    //lookup the structure record in the hash table
    StructureDbRecord *head = struct_db->structutre_db_arr[hash];

    if(!head) return NULL;

    for(; head; head = head->next){
        if(strncmp(head->structure_name, structure_name, MAX_STRUCTURE_NAME_LENGTH) == 0)
            return head;
    }
    return NULL;
}

ObjectDbRecord *object_db_lookup(char *structure_name, ObjectDb *object_db, void *pointer){
    //get the hash value of the structure name
    unsigned int hash = polynonial_rolling_hash(structure_name);

    //lookup the object record in the hash table
    ObjectDbRecord *head = object_db->object_db_arr[hash];

    if(!head) return NULL;

    for(; head; head = head->next){
        if(head->pointer == pointer)
            return head;
    }
    return NULL;

}

#ifdef TRACE

void add_object_to_object_db_with_trace(char *structure_name, ObjectDb *object_db, void *pointer, unsigned int units, StructureDbRecord *struct_rec, MldBoolean boolean_is_root, const char *file, int line){
    ObjectDbRecord *obj_rec = object_db_lookup(structure_name, object_db, pointer);
    obj_rec = calloc(1, sizeof(ObjectDbRecord));
    obj_rec->next = NULL;
    obj_rec->pointer = pointer;
    obj_rec->units = units;
    obj_rec->structure_record = struct_rec;
    obj_rec->is_root = boolean_is_root;

    //get the hash value of the structure name
    unsigned int hash = polynonial_rolling_hash(struct_rec->structure_name);//here object_name shall be used

    //add the object record to the hash table
    if(object_db->object_db_arr[hash]){
        obj_rec->next = object_db->object_db_arr[hash];
        object_db->object_db_arr[hash] = obj_rec;
    }else{
        object_db->object_db_arr[hash] = obj_rec;
    }
    object_db->count++;
    printf("[OBJECT ADDED] %s : Line %d - Added object %p (%s) to database\n",
        file, line, pointer, struct_rec->structure_name);
}

void *xcalloc_with_trace(ObjectDb *object_db, char *structure_name, int units, const char *file, int line){

    StructureDbRecord *struct_rec = struct_db_lookup(object_db->struct_db, structure_name);
    void *pointer = calloc(units, struct_rec->structure_size);
    if(!pointer) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    // Add object to db
    add_object_to_object_db_with_trace(structure_name, object_db, pointer, units, struct_rec, MLD_FALSE, file, line);
    printf("[ALLOC] %s : Line %d - Allocated %d units for %s at %p\n",
           file, line, units, structure_name, pointer);
    return pointer;
}

void *xmalloc_with_trace(ObjectDb *object_db, char *structure_name, int units, const char *file, int line){

    StructureDbRecord *struct_rec = struct_db_lookup(object_db->struct_db, structure_name);
    void *pointer = malloc(units * struct_rec->structure_size);
    if(!pointer) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    // Add object to db
    add_object_to_object_db_with_trace(structure_name, object_db, pointer, units, struct_rec, MLD_FALSE, file, line);
    printf("[ALLOC] %s : Line %d - Allocated %d units for %s at %p\n",
        file, line, units, structure_name, pointer);
    return pointer;
}

void delete_object_record_from_object_db(ObjectDb *object_db, ObjectDbRecord *obj_rec, const char *file, int line){
    assert(obj_rec);

    //get the hash value of the structure name
    unsigned int hash = polynonial_rolling_hash(obj_rec->structure_record->structure_name); //object_name shall be used

    //delete the object record from the hash table
    ObjectDbRecord *head = object_db->object_db_arr[hash];
    ObjectDbRecord *prev = NULL;

    for(; head; prev = head, head = head->next){
        if(head == obj_rec){
            if(prev){
                prev->next = head->next;
            }else{
                object_db->object_db_arr[hash] = head->next;
            }
            free(head);
            object_db->count--;
            printf("[OBJECT REMOVED] %s : Line %d - Freed object %p\n", file, line, obj_rec->pointer);
            return;
        }
    }
}

void xfree_with_trace(char *structure_name, ObjectDb *object_db, void *pointer, const char *file, int line){
    if(!pointer) return;

    ObjectDbRecord *obj_rec = object_db_lookup(structure_name, object_db, pointer);
    assert(obj_rec);

    free(obj_rec->pointer);
    obj_rec->pointer = NULL;

    delete_object_record_from_object_db(object_db, obj_rec, file, line);

    printf("[FREE] %s : Line %d - Freed object %p\n", file, line, pointer);

}

void mld_dump_object_rec_detail_with_trace(ObjectDbRecord *object_Record, const char *file, int line){
    if(!object_Record || object_Record->structure_record==0) return;
    printf("object_Record: %p\n", object_Record);
    printf("object_Record->structure_record: %p\n", object_Record->structure_record);
    printf("object_Record->structure_record->structure_name: %s\n", object_Record->structure_record->structure_name);
    printf("object_Record->structure_record->structure_size: %d\n", object_Record->structure_record->structure_size);
    printf("object_Record->structure_record->field_count: %d\n", object_Record->structure_record->field_count);
    printf("object_Record->pointer: %p\n", object_Record->pointer);
    printf("object_Record->units: %d\n", object_Record->units);
    printf("object_Record->is_visited: %d\n", object_Record->is_visited);
    printf("object_Record->is_root: %d\n", object_Record->is_root);

    int field_count = object_Record->structure_record->field_count;
    printf("Field count: %d\n", field_count);
    FieldInfo *field = NULL;

    int units = object_Record->units, obj_index = 0;

    for (; obj_index < units; obj_index++) {
        // printf("Object index: %d\n", obj_index);
        char *current_object_ptr = (char *)(object_Record->pointer) +
                                   (obj_index * object_Record->structure_record->structure_size);

        printf("  Instance %d Address Range: [%p - %p]\n", obj_index,
               current_object_ptr,
               current_object_ptr + object_Record->structure_record->structure_size - 1);

        for (int field_index = 0; field_index < field_count; field_index++) {
            field = &object_Record->structure_record->fields[field_index];
            void *field_addr = (void *)(current_object_ptr + field->offset);

            switch (field->data_type) {
                case UINT8_TYPE:
                case INT32_TYPE:
                case UINT32_TYPE:
                    printf("    %-20s : %d\n", field->field_name, *(int *)field_addr);
                    break;
                case CHAR_TYPE:
                    printf("    %-20s : %c\n", field->field_name, *(char *)field_addr);
                    break;
                case FLOAT_TYPE:
                    printf("    %-20s : %.2f\n", field->field_name, *(float *)field_addr);
                    break;
                case DOUBLE_TYPE:
                    printf("    %-20s : %.2lf\n", field->field_name, *(double *)field_addr);
                    break;
                case OBJECT_pointer_TYPE:
                case VOID_pointer_TYPE:
                    printf("    %-20s : %p\n", field->field_name, *(void **)field_addr);
                    break;
                case OBJECT_STRUCT_TYPE:
                    printf("    %-20s : [Nested Structure: %s] @ %p\n",
                           field->field_name, field->nested_structure_name, *(void **)field_addr);
                    break;
                default:
                    printf("    %-20s : UNKNOWN DATA TYPE\n", field->field_name);
            }
        }
        printf("\n");
    }
}

#else

/*
similar to the structure db, the object db is also implemented as a hashmap, 
if hash value is already occupied, then the new object record is added to the head of the linked list
if hash value is not occupied, then the new object record is added to the hash table
*/

void add_object_to_object_db(char *structure_name, ObjectDb *object_db, void *pointer, unsigned int units, StructureDbRecord *struct_rec, MldBoolean boolean_is_root){
    ObjectDbRecord *obj_rec = object_db_lookup(structure_name, object_db, pointer);
    obj_rec = calloc(1, sizeof(ObjectDbRecord));
    obj_rec->next = NULL;
    obj_rec->pointer = pointer;
    obj_rec->units = units;
    obj_rec->structure_record = struct_rec;
    obj_rec->is_root = boolean_is_root;

    //get the hash value of the structure name
    unsigned int hash = polynonial_rolling_hash(struct_rec->structure_name);//here object_name shall be used

    //add the object record to the hash table
    if(object_db->object_db_arr[hash]){
        obj_rec->next = object_db->object_db_arr[hash];
        object_db->object_db_arr[hash] = obj_rec;
    }else{
        object_db->object_db_arr[hash] = obj_rec;
    }
    object_db->count++;
}



void *xcalloc(ObjectDb *object_db, char *structure_name, int units){

    StructureDbRecord *struct_rec = struct_db_lookup(object_db->struct_db, structure_name);
    void *pointer = calloc(units, struct_rec->structure_size);
    if(!pointer) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    // Add object to db
    add_object_to_object_db(structure_name, object_db, pointer, units, struct_rec, MLD_FALSE);

    return pointer;
}

void *xmalloc(ObjectDb *object_db, char *structure_name, int units){

    StructureDbRecord *struct_rec = struct_db_lookup(object_db->struct_db, structure_name);
    void *pointer = malloc(units * struct_rec->structure_size);
    if(!pointer) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    // Add object to db
    add_object_to_object_db(structure_name, object_db, pointer, units, struct_rec, MLD_FALSE);
    return pointer;
}

void delete_object_record_from_object_db(ObjectDb *object_db, ObjectDbRecord *obj_rec){
    assert(obj_rec);

    //get the hash value of the structure name
    unsigned int hash = polynonial_rolling_hash(obj_rec->structure_record->structure_name); //object_name shall be used

    //delete the object record from the hash table
    ObjectDbRecord *head = object_db->object_db_arr[hash];
    ObjectDbRecord *prev = NULL;

    for(; head; prev = head, head = head->next){
        if(head == obj_rec){
            if(prev){
                prev->next = head->next;
            }else{
                object_db->object_db_arr[hash] = head->next;
            }
            free(head);
            object_db->count--;
            return;
        }
    }
}

void xfree(char *structure_name, ObjectDb *object_db, void *pointer){
    if(!pointer) return;

    ObjectDbRecord *obj_rec = object_db_lookup(structure_name, object_db, pointer);
    assert(obj_rec);

    free(obj_rec->pointer);
    obj_rec->pointer = NULL;

    delete_object_record_from_object_db(object_db, obj_rec);
}

void mld_dump_object_rec_detail(ObjectDbRecord *object_Record){
    if(!object_Record || object_Record->structure_record==0) return;
    printf("object_Record: %p\n", object_Record);
    printf("object_Record->structure_record: %p\n", object_Record->structure_record);
    printf("object_Record->structure_record->structure_name: %s\n", object_Record->structure_record->structure_name);
    printf("object_Record->structure_record->structure_size: %d\n", object_Record->structure_record->structure_size);
    printf("object_Record->structure_record->field_count: %d\n", object_Record->structure_record->field_count);
    printf("object_Record->pointer: %p\n", object_Record->pointer);
    printf("object_Record->units: %d\n", object_Record->units);
    printf("object_Record->is_visited: %d\n", object_Record->is_visited);
    printf("object_Record->is_root: %d\n", object_Record->is_root);

    int field_count = object_Record->structure_record->field_count;
    printf("Field count: %d\n", field_count);
    FieldInfo *field = NULL;

    int units = object_Record->units, obj_index = 0;

    for (; obj_index < units; obj_index++) {
        // printf("Object index: %d\n", obj_index);
        char *current_object_ptr = (char *)(object_Record->pointer) +
                                   (obj_index * object_Record->structure_record->structure_size);

        printf("  Instance %d Address Range: [%p - %p]\n", obj_index,
               current_object_ptr,
               current_object_ptr + object_Record->structure_record->structure_size - 1);

        for (int field_index = 0; field_index < field_count; field_index++) {
            field = &object_Record->structure_record->fields[field_index];
            void *field_addr = (void *)(current_object_ptr + field->offset);

            switch (field->data_type) {
                case UINT8_TYPE:
                case INT32_TYPE:
                case UINT32_TYPE:
                    printf("    %-20s : %d\n", field->field_name, *(int *)field_addr);
                    break;
                case CHAR_TYPE:
                    printf("    %-20s : %c\n", field->field_name, *(char *)field_addr);
                    break;
                case FLOAT_TYPE:
                    printf("    %-20s : %.2f\n", field->field_name, *(float *)field_addr);
                    break;
                case DOUBLE_TYPE:
                    printf("    %-20s : %.2lf\n", field->field_name, *(double *)field_addr);
                    break;
                case OBJECT_pointer_TYPE:
                case VOID_pointer_TYPE:
                    printf("    %-20s : %p\n", field->field_name, *(void **)field_addr);
                    break;
                case OBJECT_STRUCT_TYPE:
                    printf("    %-20s : [Nested Structure: %s] @ %p\n",
                           field->field_name, field->nested_structure_name, *(void **)field_addr);
                    break;
                default:
                    printf("    %-20s : UNKNOWN DATA TYPE\n", field->field_name);
            }
        }
        printf("\n");
    }
}

#endif

void print_object_record(ObjectDbRecord *object_record) {

    if (!object_record || object_record->structure_record==0) return;

    // Dump object details
    mld_dump_object_rec_detail(object_record);

    printf("|------------------------------------------------------------------------------------------------------|\n\n");
}

void print_object_database(ObjectDb *object_db){
    if(!object_db) return;

    printf("Printing OBJECT DATABASE\n");

    //iterate through the hash table of object db, to print all the object records
    for(int i = 0; i<TABLE_SIZE; i++){
        ObjectDbRecord *object_record = object_db->object_db_arr[i];
        for(; object_record && object_record->structure_record!=0; object_record = object_record->next){
            print_object_record(object_record);
        }
    }
}

void register_global_object_as_root(ObjectDb *object_db, void *object_ptr, char *structure_name, unsigned int units){
    StructureDbRecord *struct_rec = struct_db_lookup(object_db->struct_db, structure_name);
    assert(struct_rec);
    add_object_to_object_db(structure_name, object_db, object_ptr, units, struct_rec, MLD_TRUE);
}

void set_dynamic_object_as_root(char* structure_name, ObjectDb *object_db, void *object_ptr){
    ObjectDbRecord *obj_rec = object_db_lookup(structure_name, object_db, object_ptr);
    assert(obj_rec);
    obj_rec->is_root = MLD_TRUE;
}

ObjectDbRecord *get_next_root_object(ObjectDb *object_db, ObjectDbRecord *prev_root_obj){
    //iterate through the hash table of object db, to get the next root object
    for(int i = 0; i<TABLE_SIZE; i++){
        ObjectDbRecord *object_record = object_db->object_db_arr[i];
        for(; object_record && object_record->structure_record!=0; object_record = object_record->next){
            if(object_record->is_root && object_record != prev_root_obj && !object_record->is_visited){
                return object_record;
            }
        }
    }
    return NULL;
}

void init_mld_algorithm(ObjectDb *object_db){
    //initialize the mld algorithm, set is_visited = false for all objects
    for(int i = 0; i<TABLE_SIZE; i++){
        ObjectDbRecord *object_record = object_db->object_db_arr[i];
        for(; object_record; object_record = object_record->next){
            object_record->is_visited = MLD_FALSE;
        }
    }
}

void mld_explore_objects_recursively(ObjectDb *object_db, ObjectDbRecord *parent_obj_rec){
    //explore all objects reachable from the parent object recursively
    for(int i = 0; i<parent_obj_rec->structure_record->field_count; i++){
        FieldInfo *field = &parent_obj_rec->structure_record->fields[i];
        if(field->data_type == OBJECT_pointer_TYPE || field->data_type == VOID_pointer_TYPE){
            void *child_obj_address = (char *)parent_obj_rec->pointer + field->offset;
            if(!child_obj_address) continue;

            ObjectDbRecord *child_obj_rec = object_db_lookup(parent_obj_rec->structure_record->structure_name, object_db, child_obj_address);
            assert(child_obj_rec);

            if(!child_obj_rec->is_visited){
                child_obj_rec->is_visited = MLD_TRUE;
                mld_explore_objects_recursively(object_db, child_obj_rec);
            }
            else{
                continue;
            }
        }
    }
}

void run_mld_algorithm(ObjectDb *object_db){
    if(!object_db) return;
    init_mld_algorithm(object_db);

    ObjectDbRecord *root_obj = get_next_root_object(object_db, NULL);

    while(root_obj){
        if(root_obj->is_visited){
            root_obj = get_next_root_object(object_db, root_obj);
            continue;
        }

        root_obj->is_visited = MLD_TRUE;
        mld_explore_objects_recursively(object_db, root_obj);

        root_obj = get_next_root_object(object_db, root_obj);
    }
}

void report_leaked_objects(ObjectDb *object_db){
    printf("Leaked Objects Report:\n");

    //iterate through the hash table of object db, to print all the leaked object records
    for(int i = 0; i<TABLE_SIZE; i++){
        ObjectDbRecord *object_record = object_db->object_db_arr[i];
        for(; object_record && object_record->structure_record!=0; object_record = object_record->next){
            if(!object_record->is_visited && object_record->structure_record!=0){
                mld_dump_object_rec_detail(object_record);
            }
        }
    }
}

void init_primitive_data_types_support(StructureDb *struct_db){
    REGISTER_STRUCTURE(struct_db, int, NULL);
    REGISTER_STRUCTURE(struct_db, float, NULL);
    REGISTER_STRUCTURE(struct_db, double, NULL);
}

