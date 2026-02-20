#include <cstdlib>
#include <iostream>
#include <ctime>
#include <limits>
#include <string>

#include "Router.h"

using namespace std;

mutex Router::coutMutex; //Global mutex for thread-safe debug printing.

void RunNodeSample(Router& endpoint)
{
    for (auto markedPacket : endpoint.GetMarkedPackets())
    {
        cout << "Marked Router: ";
        markedPacket.markedRouterAddress.PrintAddress();
        cout << ", Hop Count: " << markedPacket.hopCountFromSource << endl;
    }
}

void RunEdgeSample(Router& endpoint)
{
for (auto markedPacket : endpoint.GetMarkedPackets())
    {
        cout << "Marked Router: ";
        markedPacket.markedRouterAddress.PrintAddress();
        cout << ", Previous Router: ";
        markedPacket.previousHopAddress.PrintAddress();
        cout << ", Hop Count: " << markedPacket.hopCountFromSource << endl;
    }
}

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
    //   R1(.2) attacker(.1)
    //        \/
    //        R2(.3)
    //       /  \
    //    R3(.4) R4(.5)
    //        \   \
    //         Victim(.6)
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
    victim.Start();
    //-------------------

    //-------------------
    // Packet Forwarding
    //-------------------
    int basePacketCount = 10;
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
    victim.Stop();

    //-------------------
    // Traceback
    //-------------------
    int tracebackChoice;
    cout << "Please enter (1) to perform node-sampling, or (2) for edge-sampling: ";
    while (!(cin >> tracebackChoice))
    {
        cout << "The input provided was not a valid number.";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    switch (tracebackChoice)
    {
        case 1:
            RunNodeSample(victim);
            break;
        case 2:
            RunEdgeSample(victim);
            break;
        default:
            cout << "The input provided was not 1 or 2." << endl;
            break;
    }

    //Wait Terminal
    cin.get();
    cin.get();

    return 0;
};