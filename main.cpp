#include "Router.h"

int main()
{
    Router r1({192, 168, 1, 1}, 0.2);
    Router r2({192, 168, 1, 1}, 0.2);
    Router r3({192, 168, 1, 1}, 0.2);
    Router r4({192, 168, 1, 1}, 0.2);
    Router victim({192, 168, 1, 1}, 0.2);

    //---Router Topology---
    //         R1
    //         /
    //        R2 
    //       /  \
    //      R3   R4
    //        \   \
    //         Victim
    //---------------------

    r1.Connect(&r2);
    r2.Connect(&r3);
    r2.Connect(&r4);
    r3.Connect(&victim);
    r4.Connect(&victim);

    Packet packet(r1.RequestAddress(), victim.RequestAddress());
    r1.ForwardPacket(packet);

    return 0;
};