#include <cstdlib>
#include <ctime>

#include "Router.h"

int main()
{
    srand(static_cast<unsigned>(time(nullptr)));

    float packetMarkingProbability = 0.2f;

    Router r1({192, 168, 1, 1}, packetMarkingProbability);
    Router r2({192, 168, 1, 2}, packetMarkingProbability);
    Router r3({192, 168, 1, 3}, packetMarkingProbability);
    Router r4({192, 168, 1, 4}, packetMarkingProbability);
    Router victim({192, 168, 1, 5}, packetMarkingProbability);

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

    system("pause");
    return 0;
};