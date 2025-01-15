//implementing the functions declared in mld.h

#include "mld.h"

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
    printf("No of Structures Registered = %d\n", struct_db->count);

    StructureDbRecord *structure_record = struct_db->head;
    for(int i = 0; structure_record; structure_record = structure_record->next, i++){
        printf("Structure No: %d (%p)\n", i, structure_record);
        print_structure_record(structure_record);
    }
}

int add_structure_to_database(StructureDb *struct_db, StructureDbRecord *structure_record){
    StructureDbRecord *head = struct_db->head;

    if(!head){
        struct_db->head = structure_record;
        structure_record->next = NULL;
        struct_db->count++;
        return 0;
    }

    structure_record->next = head;
    struct_db->head = structure_record;
    struct_db->count++;
    return 0;
}   

StructureDbRecord *struct_db_lookup(StructureDb *struct_db, char *structure_name){
    StructureDbRecord *head = struct_db->head;

    if(!head) return NULL;

    for(; head; head = head->next){
        if(strncmp(head->structure_name, structure_name, MAX_STRUCTURE_NAME_LENGTH) == 0)
            return head;
    }
    return NULL;
}

ObjectDbRecord *object_db_lookup(ObjectDb *object_db, void *pointer){
    ObjectDbRecord *head = object_db->head;

    if(!head) return NULL;

    for(; head; head = head->next){
        if(head->pointer == pointer)
            return head;
    }
    return NULL;
}

#ifdef TRACE

void add_object_to_object_db_with_trace(
    ObjectDb *object_db, void *pointer, unsigned int units, 
    StructureDbRecord *struct_rec, MldBoolean boolean_is_root,
    const char *file, int line) {

    // Create a new object record
    ObjectDbRecord *new_record = calloc(1, sizeof(ObjectDbRecord));
    new_record->pointer = pointer;
    new_record->units = units;
    new_record->structure_record = struct_rec;
    new_record->is_root = boolean_is_root;
    new_record->is_visited = MLD_FALSE;

    // Link into object database
    new_record->next = object_db->head;
    object_db->head = new_record;
    object_db->count++;

    printf("[OBJECT ADDED] %s : Line %d - Added object %p (%s) to database\n",
           file, line, pointer, struct_rec->structure_name);
}

void *xcalloc_with_trace(ObjectDb *object_db, char *structure_name, int units, const char *file, int line) {

    // Allocate memory
    void *pointer = calloc(units, struct_db_lookup(object_db->struct_db, structure_name)->structure_size);
    if (!pointer) {
        printf("Memory allocation failed at %s : Line %d\n", file, line);
        exit(1);
    }

    // Add to object database with trace info
    add_object_to_object_db_with_trace(object_db, pointer, units, 
        struct_db_lookup(object_db->struct_db, structure_name), MLD_FALSE, file, line);

    printf("[ALLOC] %s : Line %d - Allocated %d units for %s at %p\n",
           file, line, units, structure_name, pointer);

    return pointer;
}

void *xmalloc_with_trace(ObjectDb *object_db, char *structure_name, int units, const char *file, int line) {

    // Allocate memory
    void *pointer = malloc(units * struct_db_lookup(object_db->struct_db, structure_name)->structure_size);
    if (!pointer) {
        printf("Memory allocation failed at %s : Line %d\n", file, line);
        exit(1);
    }

    // Add to object database with trace info
    add_object_to_object_db_with_trace(object_db, pointer, units, 
        struct_db_lookup(object_db->struct_db, structure_name), MLD_FALSE, file, line);

    printf("[ALLOC] %s : Line %d - Allocated %d units for %s at %p\n",
           file, line, units, structure_name, pointer);

    return pointer;
}

void delete_object_record_from_object_db_with_trace(ObjectDb *object_db, ObjectDbRecord *obj_rec, const char *file, int line) {
    assert(obj_rec);

    ObjectDbRecord *head = object_db->head;
    if (head == obj_rec) {
        object_db->head = obj_rec->next;
        object_db->count--;
        free(obj_rec);
        printf("[OBJECT REMOVED] %s : Line %d - Freed object %p\n", file, line, obj_rec->pointer);
        return;
    }

    ObjectDbRecord *prev = head;
    while (head) {
        if (head == obj_rec) {
            prev->next = head->next;
            object_db->count--;
            free(head);
            printf("[OBJECT REMOVED] %s : Line %d - Freed object %p\n", file, line, obj_rec->pointer);
            return;
        }
        prev = head;
        head = head->next;
    }
}

void xfree_with_trace(ObjectDb *object_db, void *pointer, const char *file, int line) {
    if (!pointer) return;

    ObjectDbRecord *obj_rec = object_db_lookup(object_db, pointer);
    assert(obj_rec);

    free(obj_rec->pointer);
    obj_rec->pointer = NULL;

    delete_object_record_from_object_db_with_trace(object_db, obj_rec, file, line);
    printf("[FREE] %s : Line %d - Freed object %p\n", file, line, pointer);
}

#else

void add_object_to_object_db(ObjectDb *object_db, void *pointer, unsigned int units, StructureDbRecord *struct_rec, MldBoolean boolean_is_root){
    ObjectDbRecord *obj_rec = object_db_lookup(object_db, pointer);
    assert(!obj_rec);

    obj_rec = calloc(1, sizeof(ObjectDbRecord));
    obj_rec->next = NULL;
    obj_rec->pointer = pointer;
    obj_rec->units = units;
    obj_rec->structure_record = struct_rec;
    obj_rec->is_root = boolean_is_root;

    ObjectDbRecord *head = object_db->head;

    if(!head){
        object_db->head = obj_rec;
        obj_rec->next = NULL;
        object_db->count++;
        return;
    }

    obj_rec->next = head;
    object_db->head = obj_rec;
    object_db->count++;
}  

void *xcalloc(ObjectDb *object_db, char *structure_name, int units){
    StructureDbRecord *structure_record = struct_db_lookup(object_db->struct_db, structure_name);
    assert(structure_record);

    void *pointer = calloc(units, structure_record->structure_size);
    if(!pointer) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    add_object_to_object_db(object_db, pointer, units, structure_record, MLD_FALSE);
    return pointer;
}

void *xmalloc(ObjectDb *object_db, char *structure_name, int units){
    StructureDbRecord *struct_rec = struct_db_lookup(object_db->struct_db, structure_name);
    void *pointer = malloc(units * struct_rec->structure_size);  // changed to malloc
    if(!pointer) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    // Add object to db
    add_object_to_object_db(object_db, pointer, units, struct_rec, MLD_FALSE);

    return pointer;
}

void delete_object_record_from_object_db(ObjectDb *object_db, ObjectDbRecord *obj_rec){

    assert(obj_rec);

    ObjectDbRecord *head = object_db->head;
    if(head == obj_rec){
        object_db->head = obj_rec->next;
        object_db->count--;
        free(obj_rec);
        return;
    }

    ObjectDbRecord *prev = head;
    printf("Prev: %p\n", prev);
    while(head){
        if(head == obj_rec){
            prev->next = head->next;
            object_db->count--;
            free(head);
            return;
        }
        prev = head;
        head = head->next;
        printf("Head: %p\n", head);
    }
}

void xfree(ObjectDb *object_db, void *pointer){
    if(!pointer) return;

    ObjectDbRecord *obj_rec = object_db_lookup(object_db, pointer);
    assert(obj_rec);

    free(obj_rec->pointer);
    obj_rec->pointer = NULL;

    delete_object_record_from_object_db(object_db, obj_rec);
}

#endif


void print_object_record(ObjectDbRecord *object_record) {
    if (!object_record) return;

    printf("\n|------------------------------------------------------------------------------------------------------|\n");
    printf("| Object Structure : %-30s | Addr: %-12p | Units: %-3u | Root: %-3s |\n",
           object_record->structure_record->structure_name,
           object_record->pointer,
           object_record->units,
           object_record->is_root ? "YES" : "NO");
    printf("|------------------------------------------------------------------------------------------------------|\n");

    // Dump object details
    mld_dump_object_rec_detail(object_record);

    printf("|------------------------------------------------------------------------------------------------------|\n\n");
}


void print_object_database(ObjectDb *object_db){
    if(!object_db) return;

    printf("Printing OBJECT DATABASE\n");
    printf("No of Objects in Database = %d\n", object_db->count);

    ObjectDbRecord *object_record = object_db->head;
    for(int i = 0; object_record; object_record = object_record->next, i++){
        printf("Object No: %d (%p)\n", i, object_record);
        print_object_record(object_record);
    }
}

void register_globall_object_as_root(ObjectDb *object_db, void *object_pointer, char *structure_name, unsigned int units){
    StructureDbRecord *structure_record = struct_db_lookup(object_db->struct_db, structure_name);
    assert(structure_record);

    add_object_to_object_db(object_db, object_pointer, units, structure_record, MLD_TRUE);
} //Create a new object dbrecord entry in object db of MLD library, mark it as root

void set_dynamic_object_as_root(ObjectDb *object_db, void *object_pointer){
    ObjectDbRecord *obj_rec = object_db_lookup(object_db, object_pointer);
    assert(obj_rec);

    obj_rec->is_root = MLD_TRUE;
    obj_rec->is_visited = MLD_TRUE;
}// Search an existing object dbrecord entry in object dbof MLD library, mark it as root


ObjectDbRecord* get_next_root_object(ObjectDb *object_db, ObjectDbRecord *starting_from_here){
    ObjectDbRecord *first = starting_from_here ? starting_from_here->next : object_db->head;
    while(first){
        if(first->is_root)
            return first;
        first = first->next;
    }
    return NULL;
}

void init_mld_algorithm(ObjectDb *object_db){
    ObjectDbRecord *obj_rec = object_db->head;

    while(obj_rec){
        obj_rec->is_visited = MLD_FALSE;
        obj_rec = obj_rec->next;
    }
}


void mld_explore_objects_recursively(ObjectDb *object_db, ObjectDbRecord *parent_obj_rec){
    void *child_obj_address = NULL;

    /*
    for all fields F in parent object
        if F is a pointer
            get the address of the child object from the parent object
            if the child object is not NULL
                look up the child object in the object database
                if the child object is not visited
                    mark the child object as visited
                    explore the child object recursively
                else
                    continue to the next field
    */

    for(int i = 0; i < parent_obj_rec->units; i++){
        char *parent_obj_ptr = (char *)(parent_obj_rec->pointer) + (i * parent_obj_rec->structure_record->structure_size);
        /*
        explanation of the above code:
        1. The parent object pointer is calculated by adding the offset of the parent object to the pointer of the parent object.
        2. The offset of the parent object is calculated by multiplying the size of the parent object by the index of the unit.
        */

        for(int field_count = 0; field_count < parent_obj_rec->structure_record->field_count; field_count++){
            FieldInfo *field_info = &parent_obj_rec->structure_record->fields[field_count];

            if(field_info->data_type == OBJECT_pointer_TYPE || field_info->data_type == VOID_pointer_TYPE){
                char *child_obj_offset = parent_obj_ptr + field_info->offset;
                memcpy(&child_obj_address, child_obj_offset, sizeof(void *));

                if(!child_obj_address) continue;

                ObjectDbRecord *child_obj_rec = object_db_lookup(object_db, child_obj_address);
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
}

void run_mld_algorithm(ObjectDb *object_db){
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

#ifdef TRACE

void mld_dump_object_rec_detail_with_trace(
    ObjectDbRecord *object_record, const char *file, int line) {
    assert(object_record);

    printf("Leaked Object Details:\n");
    printf("  - Pointer Address : %p\n", object_record->pointer);
    printf("  - Units Allocated : %u\n", object_record->units);
    printf("  - Structure Name  : %s\n", 
            object_record->structure_record->structure_name);
    printf("  - Created at      : %s : Line %d\n", file, line);
    printf("  - Field Count     : %d\n", 
            object_record->structure_record->field_count);
    printf("  - Size            : %d bytes\n", 
            object_record->structure_record->structure_size);
    printf("  - Root Status     : %s\n", 
            object_record->is_root ? "YES" : "NO");

    printf("Field Details:\n");
    FieldInfo *fields = object_record->structure_record->fields;
    for (int i = 0; i < object_record->structure_record->field_count; i++) {
        printf("  -> Field Name      : %s\n", fields[i].field_name);
        printf("     Data Type       : %d\n", fields[i].data_type);
        printf("     Field Size      : %d\n", fields[i].size);
        printf("     Offset          : %d bytes\n", fields[i].offset);
        printf("     Nested Struct   : %s\n", fields[i].nested_structure_name);
    }
    printf("--------------------------------------------\n");
}

#else

void mld_dump_object_rec_detail(ObjectDbRecord *object_record) {
    int field_count = object_record->structure_record->field_count;
    FieldInfo *field = NULL;

    int units = object_record->units, obj_index = 0;

    for (; obj_index < units; obj_index++) {
        char *current_object_ptr = (char *)(object_record->pointer) +
                                   (obj_index * object_record->structure_record->structure_size);

        printf("  Instance %d Address Range: [%p - %p]\n", obj_index,
               current_object_ptr,
               current_object_ptr + object_record->structure_record->structure_size - 1);

        for (int field_index = 0; field_index < field_count; field_index++) {
            field = &object_record->structure_record->fields[field_index];
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

void report_leaked_objects(ObjectDb *object_db){
    printf("\n");
    for(ObjectDbRecord *obj_rec = object_db->head; obj_rec; obj_rec = obj_rec->next){
        if(!obj_rec->is_visited){
            printf("Memory Leak : ");
            // print_object_record(obj_rec);
            mld_dump_object_rec_detail(obj_rec);
            printf("\n");
        }
    }
}

void init_primitive_data_types_support(StructureDb *struct_db){
    REGISTER_STRUCTURE(struct_db, int, NULL);
    REGISTER_STRUCTURE(struct_db, float, NULL);
    REGISTER_STRUCTURE(struct_db, double, NULL);
}
