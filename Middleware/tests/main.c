#include <stdio.h>
#include <stdlib.h>
#include "glib.h"

#include "test_cases.h"

gboolean test_timeout(gpointer data)
{   
    g_print("Timeout\n");

    return true;
}

int main(void)
{
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    g_timeout_add(1000, test_timeout, NULL);

    g_main_loop_run(loop);

    // test_config_mgmt();
}