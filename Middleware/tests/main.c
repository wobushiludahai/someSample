#include <stdio.h>
#include <stdlib.h>
#include "glib.h"

#include "test_cases.h"
#include "dbus/dbus.h"

void test_call_method(void);
void test_get_property(void);
void test_haidbus(void);

int main(void)
{
    // test_call_method();
    // test_get_property();
    test_haidbus();

    // 因为我们使用了异步调用，所以这里需要进入主循环等待回调
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop); // 这会阻塞，直到你退出主循环

    g_main_loop_unref(loop);

    return 0;
}