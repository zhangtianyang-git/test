#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#if WIN32
#include "sqlite3.h"
#else
#include "../include/sqlite3.h"

int main(int argc, char* argv[])
{
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   std::string res="";
   rc = sqlite3_open("/config/tool.db", &db);

   if(rc){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }
   char sql[256] = "select dev_id,dev_name,dev_ip,dev_port,"
                   "dev_gateway,dev_mac,dev_alias,dev_groupIp,"
                   "dev_groupPort,dev_resolution,dev_position,"
                   "dev_ver,dev_hash,dev_type,dev_nodeId from dsc_device;" ;
   char** pResult;
   int nRow;
   int nCol;
   rc = sqlite3_get_table(db, sql, &pResult, &nRow, &nCol, NULL);
   if (rc == SQLITE_OK)
   {
       if (nRow >= 1){
            res = pResult[nCol-1];
            std::cout << res.c_str()<<std::endl;
       }
       std::cout << "sss"<<std::endl;
   }
   std::cout << "row"<<nRow<<" col"<<nCol<<std::endl;
   sqlite3_free_table(pResult);

   sqlite3_close(db);
}
#endif
