#include "mld.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

/*application structures*/

typedef struct Employee {
    char emp_name[30];
    unsigned int emp_id;
    unsigned int age;
    struct Employee *mgr;
    float salary;
} Employee;

typedef struct Student {
    char stud_name[32];
    unsigned int rollno;
    unsigned int age;
    float aggregate;
    struct Student *best_colleague;
} Student;

int main(int argc, char **argv){
    StructureDb *struct_db = calloc(1, sizeof(StructureDb));
    //initialize the struct db with primitive data types
    init_primitive_data_types_support(struct_db);

    //define fields of Employee structure
    static FieldInfo emp_fields[] = {
        FIELD_INFO(Employee, emp_name, CHAR_TYPE, 0),
        FIELD_INFO(Employee, emp_id, UINT32_TYPE, 0),
        FIELD_INFO(Employee, age, UINT32_TYPE, 0),
        FIELD_INFO(Employee, mgr, OBJECT_pointer_TYPE, Employee),
        FIELD_INFO(Employee, salary, FLOAT_TYPE, 0)
    };
    
    //register Employee structure in struct db
    REGISTER_STRUCTURE(struct_db, Employee, emp_fields);

    //define fields of Student structure
    static FieldInfo stud_fields[] = {
        FIELD_INFO(Student, stud_name, CHAR_TYPE, 0),
        FIELD_INFO(Student, rollno, UINT32_TYPE, 0),
        FIELD_INFO(Student, age, UINT32_TYPE, 0),
        FIELD_INFO(Student, aggregate, FLOAT_TYPE, 0),
        FIELD_INFO(Student, best_colleague, OBJECT_pointer_TYPE, Student)
    };

    //register Student structure in struct db
    REGISTER_STRUCTURE(struct_db, Student, stud_fields);

    // //testing struct db
    print_structure_database(struct_db);

    //allocate memory for object db
    ObjectDb *object_db = calloc(1, sizeof(ObjectDb));
    //link struct db to object db
    object_db->struct_db = struct_db;

    //also pass object name to functions, so that hash value can be calculated, finding can also be done using object names

    //allocate memory for Student object
    Student *s1 = xmalloc(object_db, "Student", 1);
    memset(s1, 0, sizeof(Student));
    //mark Student object as root object
    set_dynamic_object_as_root("Student", object_db, s1);

    //allocate memory for Student object
    Student *s2 = xmalloc(object_db, "Student", 1);
    memset(s2, 0, sizeof(Student));
    //initialize Student object
    strncpy(s2->stud_name, "John", strlen("John"));

    //allocate memory for Employee object
    Employee *e1 = xmalloc(object_db, "Employee", 1);
    memset(e1, 0, sizeof(Employee));
    //set Employee object as root object
    set_dynamic_object_as_root("Employee", object_db, e1);
    
    //allocate memory for int object
    int *p = xmalloc(object_db, "int", 1);
    memset(p, 0, sizeof(int));

    //allocate memory for int object
    float *q = xmalloc(object_db, "float", 1);
    memset(q, 0, sizeof(float));

    //initialize Student object
    s1->best_colleague = s2;

    //mark int object as root object
    set_dynamic_object_as_root("int", object_db, p);

    //allocate memory for Employee object
    Employee *e2 = xmalloc(object_db, "Employee", 1);
    memset(e2, 0, sizeof(Employee));
    //mark Employee object as root object
    set_dynamic_object_as_root("Employee", object_db, e2);
    //initialize Employee object
    e2->mgr = e1;

    print_object_database(object_db);


    xfree("int", object_db, p);
    xfree("Employee", object_db, e1);
    xfree("Student", object_db, s1);
    xfree("Student", object_db, s2);
    xfree("Employee", object_db, e2);
    
    //runn the MLD algorithm
    run_mld_algorithm(object_db);
    //report the leaked objects
    report_leaked_objects(object_db);

    return 0;
}