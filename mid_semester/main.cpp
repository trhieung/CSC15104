#include "volume.h"

int main(){
    CustomVolume a("myvol", 1 << 23, "p@ssword");

    a.setup("MyFS.DRS");
    // bool x = a.openVolume("MyFS.DRS", "p@ssword");

    // char m[512];
    // for (int i = 0; i < 128; i++){
    //     memcpy(&m[i*4], &i, 4);
    // }

    // a.writeSector("MyFS.DRS", 16, &m[0]);

    a.importFile("MyFS.DRS", "p@ssword", "vol_test.txt", "test.txt");
    cerr << "hello";


    return 0;
}