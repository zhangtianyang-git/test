#include <stdio.h>
#include <stdlib.h>
#include "handlesockettransferfile.h"
#include <string>
#include <string.h>

int main(int argc, char *argv[])
{
    HandleSocketTransferFile hand;
    //(const string &sqlver, const string &localip, const string &serverip, int port)
    hand.SetTransferFileInfo("v0.0.1","192.168.1.100","192.168.1.99",22223);
    hand.TcpwriteTransferFile();
    while(1);
}
