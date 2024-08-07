/*
 * Description:配置管理
 */

#include "config_mgmt.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>

typedef struct
{
    char *str;
    char str_len;
} STR_T;

static sqlite3 *g_config_db = NULL;

int32_t config_mgmt_init(void)
{
    char path[PATH_MAX_LEN] = { 0 };
    int ret;
    char *sql;
    char *err_msg = NULL;

    if (g_config_db != NULL)
    {
        return RET_OK;
    }

    ret = snprintf(path, sizeof(path), "%s/%s.db", CONFIG_FILE_PREFIX, MODULE_NAME);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_open(path, &g_config_db);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "Can't open database: %s\n", sqlite3_errmsg(g_config_db));
        return RET_ERR;
    }
    else
    {
        syslog(LOG_NOTICE, "Opened database successfully");
    }

    sql = malloc(SQL_STATEMENTS_LEN);
    if (sql == NULL)
    {
        syslog(LOG_ERR, "malloc failed");
        return RET_ERR;
    }

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "CREATE TABLE IF NOT EXISTS %s ("
        "PROPERTY_NAME TEXT PRIMARY KEY NOT NULL,PROPERTY_VALUE BLOB NOT NULL);",
        MODULE_NAME);
    if (ret < 0)
    {
        free(sql);
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, NULL, NULL, &err_msg);
    free(sql);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int32_t config_mgmt_deinit(void)
{
    sqlite3_close(g_config_db);
    g_config_db = NULL;
    return RET_OK;
}

static int32_t exec_insert(char *sql)
{
    int ret;
    char *err_msg = NULL;

    ret = sqlite3_exec(g_config_db, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

static int32_t exec_lookup(char *sql, sqlite3_callback callback_func, void *argc)
{
    int ret;
    char *err_msg = NULL;

    ret = sqlite3_exec(g_config_db, sql, callback_func, argc, &err_msg);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int32_t config_mgmt_set_char_value(char *name, char value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%c');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_insert(sql);
}

static int fill_char(void *dest, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(char *)dest = *argv[0];
    }

    return 0;
}

int32_t config_mgmt_get_char_value(char *name, char *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_lookup(sql, fill_char, value);
}

int32_t config_mgmt_set_bool_value(char *name, bool value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%d');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_insert(sql);
}

static int fill_bool(void *dest, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        if (*argv[0] == true)
        {
            *(bool *)dest = true;
        }
        else
        {
            *(bool *)dest = false;
        }
    }

    return 0;
}

int32_t config_mgmt_get_bool_value(char *name, bool *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_lookup(sql, fill_bool, value);
}

int32_t config_mgmt_set_int32_value(char *name, int32_t value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%d');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_insert(sql);
}

static int fill_int32(void *dest, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(int32_t *)dest = atoi(argv[0]);
    }

    return 0;
}

int32_t config_mgmt_get_int32_value(char *name, int32_t *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_lookup(sql, fill_int32, value);
}

int32_t config_mgmt_set_uint32_value(char *name, uint32_t value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%u');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_insert(sql);
}

static int fill_uint32(void *dest, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(uint32_t *)dest = strtoul(argv[0], NULL, 0);
    }

    return 0;
}

int32_t config_mgmt_get_uint32_value(char *name, uint32_t *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_lookup(sql, fill_uint32, value);
}

int32_t config_mgmt_set_int64_value(char *name, int64_t value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%ld');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_insert(sql);
}

static int fill_int64(void *dest, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(int64_t *)dest = strtoll(argv[0], NULL, 0);
    }

    return 0;
}

int32_t config_mgmt_get_int64_value(char *name, int64_t *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_lookup(sql, fill_int64, value);
}

int32_t config_mgmt_set_uint64_value(char *name, uint64_t value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%lu');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_insert(sql);
}

int fill_uint64(void *dest, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(uint64_t *)dest = strtoull(argv[0], NULL, 0);
    }

    return 0;
}

int32_t config_mgmt_get_uint64_value(char *name, uint64_t *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_lookup(sql, fill_uint64, value);
}

int32_t config_mgmt_set_double_value(char *name, double value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%lf');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_insert(sql);
}

int fill_double(void *dest, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(double *)dest = strtold(argv[0], NULL);
    }

    return 0;
}

int32_t config_mgmt_get_double_value(char *name, double *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_lookup(sql, fill_double, value);
}

int32_t config_mgmt_set_float_value(char *name, float value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%f');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_insert(sql);
}

int fill_float(void *dest, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(float *)dest = strtof(argv[0], NULL);
    }

    return 0;
}

int32_t config_mgmt_get_float_value(char *name, float *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_lookup(sql, fill_float, value);
}

int32_t config_mgmt_set_string_value(char *name, char *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;

    if (strlen(value) > DB_STR_MAX_LEN)
    {
        syslog(LOG_ERR, "Str is too long");
        return RET_ERR;
    }

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%s');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    return exec_insert(sql);
}

int fill_str(void *dest, int argc, char **argv, char **azColName)
{
    STR_T *str = (STR_T *)dest;

    if (argc != 0)
    {
        strncpy(str->str, argv[0], str->str_len - 1);
        str->str[str->str_len - 1] = '\0';
    }

    return 0;
}

int32_t config_mgmt_get_string_value(char *name, char *value, uint32_t val_len)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;
    STR_T str;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    str.str = value;
    str.str_len = val_len;

    return exec_lookup(sql, fill_str, &str);
}