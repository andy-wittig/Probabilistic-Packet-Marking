#include <cstdlib>
#include <iostream>
#include <ctime>
#include <limits>
#include <string>
#include <algorithm>
#include <unordered_map>

#include "Router.h"

using namespace std;

mutex Router::coutMutex; //Global mutex for thread-safe debug printing.

void RunNodeSample(Router& endpoint)
{
    vector<MarkedPacketInfo> markedPackets = endpoint.GetMarkedPackets();
    unordered_map<string, int> routerCountMap;

    for (auto& markedPacket : markedPackets)
    {
        cout << "Marked Router: ";
        markedPacket.markedRouterAddress.PrintAddress();
        cout << ", Hop Count: " << markedPacket.hopCountFromSource << endl;

        //Convert address to comparable string.
        string address = to_string(markedPacket.markedRouterAddress.address[0]) + "." +
                     to_string(markedPacket.markedRouterAddress.address[1]) + "." +
                     to_string(markedPacket.markedRouterAddress.address[2]) + "." +
                     to_string(markedPacket.markedRouterAddress.address[3]);

        routerCountMap[address]++;
    }
    vector<pair<string, int>> sortedRouters(routerCountMap.begin(), routerCountMap.end());
    for (auto& router : sortedRouters)
    {
        cout << router.first << ": " << router.second << endl;
    }

    sort(sortedRouters.begin(), sortedRouters.end(),
        [] (const pair<string, int>& a, const pair<string, int>& b)
        {
            return a.second > b.second;
        }); //Sort using lambda.
    
    cout << "Estimated Path: ";
    for (auto& address : sortedRouters)
    {
        cout << address.first << " -> ";
    }
    cout << "victim" << endl;
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

    float attackerRate;
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
    Router r4({192, 168, 1, 6}, markingProbability);
    Router r5({192, 168, 1, 7}, markingProbability);
    Router r6({192, 168, 1, 8}, markingProbability);
    Router r7({192, 168, 1, 9}, markingProbability);
    Router r8({192, 168, 1, 10}, markingProbability);
    Router r9({192, 168, 1, 11}, markingProbability);
    Router r10({192, 168, 1, 12}, markingProbability);
    Router victim({192, 168, 1, 13}, markingProbability);

    attacker.Connect(&r2);
    r2.Connect(&r8);
    r1.Connect(&r5);
    r4.Connect(&r5);
    r4.Connect(&r7);
    r10.Connect(&r7);
    r5.Connect(&r8);
    r5.Connect(&r9);
    r7.Connect(&r9);
    r9.Connect(&r6);
    r6.Connect(&r3);
    r3.Connect(&victim);

    attacker.Start();
    r1.Start();
    r2.Start();
    r3.Start();
    r4.Start();
    r5.Start();
    r6.Start();
    r7.Start();
    r8.Start();
    r9.Start();
    r10.Start();
    victim.Start();
    //-------------------

    //-------------------
    // Packet Forwarding
    //-------------------
    int basePacketCount = 10;
    int attackerPacketCount = floor(basePacketCount * attackerRate);

    Packet normalPacket(r1.RequestAddress(), victim.RequestAddress());
    Packet maliciousPacket(attacker.RequestAddress(), victim.RequestAddress());

    //Could use chronos here to simulate wait between each packet being sent.
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
    r5.Stop();
    r6.Stop();
    r7.Stop();
    r8.Stop();
    r9.Stop();
    r10.Stop();
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