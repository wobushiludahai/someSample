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

int32_t config_mgmt_set_char_value(char *name, char value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;
    char *err_msg = NULL;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%c');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

static int callback_char(void *NotUsed, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(char *)NotUsed = *argv[0];
    }

    return 0;
}

int32_t config_mgmt_get_char_value(char *name, char *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    char *err_msg = NULL;
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);

    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, callback_char, value, &err_msg);
    if (err_msg != NULL)
    {
        printf("SQL error: %s\n", err_msg);
    }

    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int32_t config_mgmt_set_bool_value(char *name, bool value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;
    char *err_msg = NULL;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%d');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

static int callback_bool(void *NotUsed, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        if (*argv[0] == true)
        {
            *(bool *)NotUsed = true;
        }
        else
        {
            *(bool *)NotUsed = false;
        }
    }

    return 0;
}

int32_t config_mgmt_get_bool_value(char *name, bool *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    char *err_msg = NULL;
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, callback_bool, value, &err_msg);
    if (err_msg != NULL)
    {
        printf("SQL error: %s\n", err_msg);
    }

    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int32_t config_mgmt_set_int32_value(char *name, int32_t value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;
    char *err_msg = NULL;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%d');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

static int callback_int32(void *NotUsed, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(int32_t *)NotUsed = atoi(argv[0]);
    }

    return 0;
}

int32_t config_mgmt_get_int32_value(char *name, int32_t *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    char *err_msg = NULL;
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, callback_int32, value, &err_msg);
    if (err_msg != NULL)
    {
        printf("SQL error: %s\n", err_msg);
    }

    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int32_t config_mgmt_set_uint32_value(char *name, uint32_t value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;
    char *err_msg = NULL;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%u');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

static int callback_uint32(void *NotUsed, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(uint32_t *)NotUsed = strtoul(argv[0], NULL, 0);
    }

    return 0;
}

int32_t config_mgmt_get_uint32_value(char *name, uint32_t *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    char *err_msg = NULL;
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, callback_uint32, value, &err_msg);
    if (err_msg != NULL)
    {
        printf("SQL error: %s\n", err_msg);
    }

    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int32_t config_mgmt_set_int64_value(char *name, int64_t value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;
    char *err_msg = NULL;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%ld');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

static int callback_int64(void *NotUsed, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(int64_t *)NotUsed = strtoll(argv[0], NULL, 0);
    }

    return 0;
}

int32_t config_mgmt_get_int64_value(char *name, int64_t *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    char *err_msg = NULL;
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, callback_int64, value, &err_msg);
    if (err_msg != NULL)
    {
        printf("SQL error: %s\n", err_msg);
    }

    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int32_t config_mgmt_set_uint64_value(char *name, uint64_t value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;
    char *err_msg = NULL;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%lu');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int callback_uint64(void *NotUsed, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(uint64_t *)NotUsed = strtoull(argv[0], NULL, 0);
    }

    return 0;
}

int32_t config_mgmt_get_uint64_value(char *name, uint64_t *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    char *err_msg = NULL;
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, callback_uint64, value, &err_msg);
    if (err_msg != NULL)
    {
        printf("SQL error: %s\n", err_msg);
    }

    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int32_t config_mgmt_set_double_value(char *name, double value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;
    char *err_msg = NULL;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%lf');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int callback_double(void *NotUsed, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(double *)NotUsed = strtold(argv[0], NULL);
    }

    return 0;
}

int32_t config_mgmt_get_double_value(char *name, double *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    char *err_msg = NULL;
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, callback_double, value, &err_msg);
    if (err_msg != NULL)
    {
        printf("SQL error: %s\n", err_msg);
    }

    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int32_t config_mgmt_set_float_value(char *name, float value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;
    char *err_msg = NULL;

    ret = snprintf(sql, SQL_STATEMENTS_LEN,
        "INSERT OR REPLACE INTO %s (PROPERTY_NAME,PROPERTY_VALUE) VALUES ('%s','%f');", MODULE_NAME, name, value);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int callback_float(void *NotUsed, int argc, char **argv, char **azColName)
{
    if (argc != 0)
    {
        *(float *)NotUsed = strtof(argv[0], NULL);
    }

    return 0;
}

int32_t config_mgmt_get_float_value(char *name, float *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    char *err_msg = NULL;
    int ret;

    ret = snprintf(
        sql, SQL_STATEMENTS_LEN, "SELECT PROPERTY_VALUE FROM %s WHERE PROPERTY_NAME='%s';", MODULE_NAME, name);
    if (ret < 0)
    {
        syslog(LOG_ERR, "snprintf failed");
        return RET_ERR;
    }

    ret = sqlite3_exec(g_config_db, sql, callback_float, value, &err_msg);
    if (err_msg != NULL)
    {
        printf("SQL error: %s\n", err_msg);
    }

    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int32_t config_mgmt_set_string_value(char *name, char *value)
{
    char sql[SQL_STATEMENTS_LEN] = { 0 };
    int ret;
    char *err_msg = NULL;

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

    ret = sqlite3_exec(g_config_db, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}

int callback_str(void *NotUsed, int argc, char **argv, char **azColName)
{
    STR_T *str = (STR_T *)NotUsed;

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
    char *err_msg = NULL;
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

    ret = sqlite3_exec(g_config_db, sql, callback_str, &str, &err_msg);
    if (err_msg != NULL)
    {
        printf("SQL error: %s\n", err_msg);
    }

    if (ret != SQLITE_OK)
    {
        syslog(LOG_ERR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return RET_ERR;
    }

    return RET_OK;
}