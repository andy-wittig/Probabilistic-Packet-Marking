#include <cstdlib>
#include <iostream>
#include <ctime>
#include <limits>

#include "Router.h"

using namespace std;

mutex Router::coutMutex; //Global mutex for thread-safe debug printing.

int main()
{
    srand(static_cast<unsigned>(time(nullptr)));

    float markingProbability;
    cout << "Please enter a marking probability (p): ";
    while (!(cin >> markingProbability))
    {
        cout << "The input provided was not a valid number.";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    int attackerRate;
    cout << "Please enter the attackers rate (x): ";
    while (!(cin >> attackerRate))
    {
        cout << "The input provided was not a valid number.";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    //-------------------
    // Generate Topology
    // 10-20 Routers
    // 3, 4, or 5 branches
    //-------------------
    Router attacker({192, 168, 1, 1}, markingProbability);
    Router r1({192, 169, 1, 2}, markingProbability);
    Router r2({192, 168, 1, 3}, markingProbability);
    Router r3({192, 168, 1, 4}, markingProbability);
    Router r4({192, 168, 1, 5}, markingProbability);
    Router victim({192, 168, 1, 6}, markingProbability);

    //---Network Topology---
    //      R1  attacker
    //        \/
    //        R2 
    //       /  \
    //      R3   R4
    //        \   \
    //         Victim
    //---------------------

    attacker.Connect(&r2);
    r1.Connect(&r2);
    r2.Connect(&r3);
    r2.Connect(&r4);
    r3.Connect(&victim);
    r4.Connect(&victim);

    attacker.Start();
    r1.Start();
    r2.Start();
    r3.Start();
    r4.Start();
    //-------------------

    //-------------------
    // Packet Forwarding
    //-------------------

    int basePacketCount = 50;
    int attackerPacketCount = basePacketCount * attackerRate;

    Packet normalPacket(r1.RequestAddress(), victim.RequestAddress());
    Packet maliciousPacket(attacker.RequestAddress(), victim.RequestAddress());

    for (int i = 0; i < basePacketCount; i++)
    { //Normal Use Packets
        r1.EnqueuePacket(normalPacket);
    }
    for (int i = 0; i < attackerPacketCount; i++)
    { //Attacker Packets
        attacker.EnqueuePacket(maliciousPacket);
    }

    //Cleanup Routers
    attacker.Stop();
    r1.Stop();
    r2.Stop();
    r3.Stop();
    r4.Stop();

    //Wait Terminal
    cin.get();
    cin.get();

    return 0;
};