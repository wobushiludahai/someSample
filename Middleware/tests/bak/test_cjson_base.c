#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "cJSON.h"

size_t ret;

int main(void)
{
    // 打开JSON文件
    FILE *file = fopen("property_t.json", "r");
    if (!file) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // 读取文件内容
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = (char *)malloc(length + 1);
    ret = fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0'; // 确保字符串以null结尾

    // 解析JSON字符串
    cJSON *json = cJSON_Parse(data);
    if (!json) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: [%s]\n", error_ptr);
        }
        free(data);
        return EXIT_FAILURE;
    }

    // 打印解析结果
    char *printed = cJSON_Print(json);
    printf("%s\n", printed);
    free(printed); // 释放cJSON_Print返回的字符串

    // 访问JSON中的值
    cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");
    if (cJSON_IsString(name) && name->valuestring != NULL) {
        printf("Name: %s\n", name->valuestring);
    }

    cJSON *age = cJSON_GetObjectItemCaseSensitive(json, "age");
    if (cJSON_IsNumber(age)) {
        printf("Age: %d\n", age->valueint);
    }

    // 清理
    cJSON_Delete(json);
    free(data);
}