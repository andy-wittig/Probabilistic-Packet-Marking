#include <cstdlib>
#include <ctime>
#include <iostream>

#include "Router.h"

int main()
{
    srand(static_cast<unsigned>(time(nullptr)));

    float markingProbability;
    cout << "Please enter a marking probability (p): ";
    while (!(cin >> markingProbability))
    {
        cout << "The input provided was not a valid number" << endl;
    }

    float attackerRate;
    cout << "Please enter the attackers rate (x): ";
    while (!(cin >> attackerRate))
    {
        cout << "The input provided was not a valid number" << endl;
    }

    //-------------------
    // Generate Topology
    // 10-20 Routers
    // 3, 4, or 5 branches
    // Attackers should send more packets than normal users (syn-flood or ping-flood).
    // Attacker rate of sending packets is x times more than normal users.
    //-------------------
    Router attacker({192, 168, 1, 1}, markingProbability);
    Router r2({192, 168, 1, 2}, markingProbability);
    Router r3({192, 168, 1, 3}, markingProbability);
    Router r4({192, 168, 1, 4}, markingProbability);
    Router victim({192, 168, 1, 5}, markingProbability);

    //---Network Topology---
    //      attacker
    //         /
    //        R2 
    //       /  \
    //      R3   R4
    //        \   \
    //         Victim
    //---------------------

    attacker.Connect(&r2);
    r2.Connect(&r3);
    r2.Connect(&r4);
    r3.Connect(&victim);
    r4.Connect(&victim);
    //-------------------

    //-------------------
    // Packet Forwarding
    //-------------------
    Packet packet(attacker.RequestAddress(), victim.RequestAddress());
    attacker.ForwardPacket(packet);

    system("pause");
    return 0;
};