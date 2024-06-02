#include "test_cases.h"
#include "config_mgmt.h"

void test_config_mgmt(void)
{
    config_mgmt_init();

    char test_char;
    config_mgmt_set_char_value("property_char", 'a');
    config_mgmt_get_char_value("property_char", &test_char);
    printf("char value: %c\n", test_char);

    bool test_bool;
    config_mgmt_set_bool_value("property_bool", false);
    config_mgmt_get_bool_value("property_bool", &test_bool);
    printf("bool value: %d\n", test_bool);

    int32_t test_int;
    config_mgmt_set_int32_value("property_int", -1);
    config_mgmt_get_int32_value("property_int", &test_int);
    printf("int value: %d\n", test_int);

    uint32_t test_uint32;
    config_mgmt_set_uint32_value("property_uint32", 0xffffffff);
    config_mgmt_get_uint32_value("property_uint32", &test_uint32);
    printf("uint32 value: %u\n", test_uint32);

    int64_t test_int64;
    config_mgmt_set_int64_value("property_int64", -1);
    config_mgmt_get_int64_value("property_int64", &test_int64);
    printf("int64 value: %ld\n", test_int64);

    uint64_t test_uint64;
    config_mgmt_set_uint64_value("property_uint64", 0xffffffffffffffff);
    config_mgmt_get_uint64_value("property_uint64", &test_uint64);
    printf("uint64 value: %lu\n", test_uint64);

    float test_float;
    config_mgmt_set_float_value("property_float", 3.14159263);
    config_mgmt_get_float_value("property_float", &test_float);
    printf("float value: %f\n", test_float);

    double test_double;
    config_mgmt_set_double_value("property_double", -235423.14159265358979323846);
    config_mgmt_get_double_value("property_double", &test_double);
    printf("double value: %lf\n", test_double);

    char test_string[100];
    config_mgmt_set_string_value("property_string", "Hello, World!                              0xffffffff test");
    config_mgmt_get_string_value("property_string", test_string, sizeof(test_string));
    printf("string value: %s\n", test_string);
}