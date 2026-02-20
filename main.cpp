#include <cstdlib>
#include <iostream>
#include <thread>
#include <mutex>
#include <ctime>
#include <limits>
#include <vector>

#include "Router.h"

using namespace std;

void SendPacket(Router& source, Router& destination)
{
    Packet packet(source.RequestAddress(), destination.RequestAddress());
    source.ForwardPacket(packet);
}

mutex Router::coutMutex;

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
    // Attackers should send more packets than normal users (syn-flood or ping-flood).
    // Attacker rate of sending packets is x times more than normal users.
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
    //-------------------

    //-------------------
    // Packet Forwarding
    // - Routers should have single thread allocated for use
    // - Routers use queues to process packets
    //-------------------

    int baseTasks = 1;
    int attackerTasks = baseTasks * attackerRate;
    vector<thread> forwardPacketThreads;
    forwardPacketThreads.reserve(baseTasks + attackerTasks); //Eliminates memory reallocation for every task added.
    
    for (int i = 0; i < baseTasks; i++)
    { //Normal Use Packets
        forwardPacketThreads.emplace_back(SendPacket, ref(r1), ref(victim));
    }
    for (int i = 0; i < attackerTasks; i++)
    { //Attacker Packets
        forwardPacketThreads.emplace_back(SendPacket, ref(attacker), ref(victim));
    }

    for (auto& t : forwardPacketThreads)
    {
        t.join();
    }

    cin.get();
    cin.get();
    return 0;
};