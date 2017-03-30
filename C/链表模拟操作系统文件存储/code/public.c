#include <stdio.h>
#include "fs-sim.h"

int main()
{
    Fs_sim filesystem;

    mkfs(&filesystem);

    ls(&filesystem, ".");

    return 0;
}
