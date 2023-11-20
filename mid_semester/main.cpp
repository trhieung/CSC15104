#include "volume.h"

int main(){
    CustomVolume a("MyFS.DRS", 1 << 23, "password123");

    a.setup("MyFS.DRS");

    a.check_volpwd("MyFS.DRS", "password123");
    a.check_volpwd("MyFS.DRS", "p@ssword");

    a.change_volpwd("MyFS.DRS", "password123", "p@ssword");

    a.create_txt_file("test1.txt", 8*512+1, true);
    a.create_txt_file("test2.txt", 8*512*2+1, true);

    a.importFile("MyFS.DRS", "p@ssword", "vol_test1.txt", "test1.txt");
    a.importFile("MyFS.DRS", "p@ssword", "vol_test2.txt", "test2.txt");

    a.listFile("MyFS.DRS");

    a.exportFile("MyFS.DRS", "p@ssword", "vol_test_out1.txt", "vol_test1.txt");
    a.exportFile("MyFS.DRS", "p@ssword", "vol_test_out2.txt", "vol_test2.txt");

    a.deleteFile("MyFS.DRS", "p@ssword", "vol_test1.txt");
    a.listFile("MyFS.DRS");
    cerr << "hello";

    return 0;
}