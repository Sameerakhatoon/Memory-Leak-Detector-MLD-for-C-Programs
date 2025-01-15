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
    init_primitive_data_types_support(struct_db);

    static FieldInfo emp_fields[] = {
        FIELD_INFO(Employee, emp_name, CHAR_TYPE, 0),
        FIELD_INFO(Employee, emp_id, UINT32_TYPE, 0),
        FIELD_INFO(Employee, age, UINT32_TYPE, 0),
        FIELD_INFO(Employee, mgr, OBJECT_pointer_TYPE, Employee),
        FIELD_INFO(Employee, salary, FLOAT_TYPE, 0)
    };

    REGISTER_STRUCTURE(struct_db, Employee, emp_fields);

    static FieldInfo stud_fields[] = {
        FIELD_INFO(Student, stud_name, CHAR_TYPE, 0),
        FIELD_INFO(Student, rollno, UINT32_TYPE, 0),
        FIELD_INFO(Student, age, UINT32_TYPE, 0),
        FIELD_INFO(Student, aggregate, FLOAT_TYPE, 0),
        FIELD_INFO(Student, best_colleague, OBJECT_pointer_TYPE, Student)
    };

    REGISTER_STRUCTURE(struct_db, Student, stud_fields);

    ObjectDb *object_db = calloc(1, sizeof(ObjectDb));
    object_db->struct_db = struct_db;

    Student *s1 = xmalloc(object_db, "Student", 1);
    memset(s1, 0, sizeof(Student));
    set_dynamic_object_as_root(object_db, s1);

    Student *s2 = xmalloc(object_db, "Student", 1);
    memset(s2, 0, sizeof(Student));
    strncpy(s2->stud_name, "John", strlen("John"));

    Employee *e1 = xmalloc(object_db, "Employee", 1);
    memset(e1, 0, sizeof(Employee));

    int *p = xmalloc(object_db, "int", 1);
    memset(p, 0, sizeof(int));

    s1->best_colleague = s2;

    set_dynamic_object_as_root(object_db, p);

    Employee *e2 = xmalloc(object_db, "Employee", 1);
    memset(e2, 0, sizeof(Employee));
    set_dynamic_object_as_root(object_db, e2);

    e2->mgr = e1;

    // print_structure_database(struct_db);

    xfree(object_db, p);
    xfree(object_db, e1);
    xfree(object_db, s1);
    xfree(object_db, s2);
    xfree(object_db, e2);


    print_object_database(object_db);
    run_mld_algorithm(object_db);
    report_leaked_objects(object_db);




    return 0;

}