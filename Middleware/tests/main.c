#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

int main(void)
{
    printf("hello World\n");

   sqlite3 *db;
   int rc;

   rc = sqlite3_open("test.db", &db);

   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }
   sqlite3_close(db);
}