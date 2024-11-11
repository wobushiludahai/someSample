/*
 *Description:持久化配置管理
 */

#ifndef _CONFIG_MGMT_H_
#define _CONFIG_MGMT_H_

#include <stdint.h>
#include <stdbool.h>
#include <sqlite3.h>

#define CONFIG_FILE_PREFIX "/home/hai/github/someSample/Middleware/tests/output"

#ifndef PATH_MAX_LEN
#define PATH_MAX_LEN (256)
#endif

#define SQL_STATEMENTS_LEN (1024)
#define DB_STR_MAX_LEN (700)

#ifndef MODULE_NAME
#define MODULE_NAME "config_mgmt"
#endif

#define DEFAULT_PRIMARY_KEY 1

#ifndef RET_OK
#define RET_OK 0
#endif

#ifndef RET_ERR
#define RET_ERR -1
#endif

int32_t config_mgmt_init(void);
int32_t config_mgmt_deinit(void);

int32_t config_mgmt_set_char_value(char *name, char value);
int32_t config_mgmt_get_char_value(char *name, char *value);

int32_t config_mgmt_set_bool_value(char *name, bool value);
int32_t config_mgmt_get_bool_value(char *name, bool *value);

int32_t config_mgmt_set_int32_value(char *name, int32_t value);
int32_t config_mgmt_get_int32_value(char *name, int32_t *value);

int32_t config_mgmt_set_uint32_value(char *name, uint32_t value);
int32_t config_mgmt_get_uint32_value(char *name, uint32_t *value);

int32_t config_mgmt_set_int64_value(char *name, int64_t value);
int32_t config_mgmt_get_int64_value(char *name, int64_t *value);

int32_t config_mgmt_set_uint64_value(char *name, uint64_t value);
int32_t config_mgmt_get_uint64_value(char *name, uint64_t *value);

int32_t config_mgmt_set_double_value(char *name, double value);
int32_t config_mgmt_get_double_value(char *name, double *value);

int32_t config_mgmt_set_float_value(char *name, float value);
int32_t config_mgmt_get_float_value(char *name, float *value);

int32_t config_mgmt_set_string_value(char *name, char *value);
int32_t config_mgmt_get_string_value(char *name, char *value, uint32_t val_len);

int32_t config_mgmt_get_binary_value(char *name, char *value, uint32_t val_len);
int32_t config_mgmt_set_binary_value(char *name, char *value, uint32_t val_len);

#endif /* _CONFIG_MGMT_H_ */