#include <cstdlib>
#include <iostream>
#include <ctime>
#include <limits>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "Router.h"

using namespace std;

mutex Router::coutMutex; //Global mutex for thread-safe debug printing.

void RunNodeSample(Router& endpoint)
{
    vector<MarkedPacketInfo> markedPackets = endpoint.GetMarkedPackets();
    unordered_map<string, int> routerCountMap;

    for (auto& markedPacket : markedPackets)
    {
        /*
        cout << "Marked Router: ";
        markedPacket.markedRouterAddress.PrintAddress();
        cout << ", Hop Count: " << markedPacket.hopCountFromSource << endl;
        */

        //Convert address to comparable string.
        string address = markedPacket.markedRouterAddress.GetAddressAsString();               

        routerCountMap[address]++;
    }

    vector<pair<string, int>> sortedRouters(routerCountMap.begin(), routerCountMap.end());

    //Remove routers with less than 10% frequency.
    int maxCount = 0;
    for (auto& router : sortedRouters)
    {
        maxCount = max(maxCount, router.second);
    }
    int thresholdCount = max(1, static_cast<int>(floor(maxCount * 0.1f)));

    //Filter Weak Edges
    vector<pair<string, int>> filteredRouters;
    for (auto& router : sortedRouters)
    {
        if (router.second >= thresholdCount)
            filteredRouters.push_back(router);
    }

    /*
    for (auto& router : filteredRouters)
    {
        cout << router.first << ": " << router.second << endl;
    }
    */

    sort(filteredRouters.begin(), filteredRouters.end(),
        [] (const pair<string, int>& a, const pair<string, int>& b)
        {
            return a.second > b.second;
        }); //Sort using lambda.
    
    //Ground Truth
    vector<string> truePath = {
        "192.168.1.10",
        "192.168.1.10",
        "192.168.1.9",
        "192.168.1.8",
        "192.168.1.4",
        "192.168.1.6",
        "192.168.1.2",
        "192.168.1.0",
        "192.168.0.0",
    };

    unordered_set<string> trueSet = {
        "192.168.1.10",
        "192.168.1.9",
        "192.168.1.8",
        "192.168.1.4",
        "192.168.1.6",
        "192.168.1.2",
        "192.168.1.0",
        "192.168.0.0",
    };

    //Construct Path
    vector<string> predictedPath;

    float accuracyPercent;
    //cout << "Estimated Path: victim -> ";
    for (auto& address : filteredRouters)
    {
        //cout << address.first << " -> ";
        predictedPath.push_back(address.first);
    }
    //cout << "attacker" << endl;
    unordered_set<string> predictedSet(predictedPath.begin(), predictedPath.end());
    
    int correctOrdered = 0;
    int minLength = min(predictedPath.size(), truePath.size());
    for (int i = 0; i < minLength; ++i)
    {
        if (predictedPath[i] == truePath[i])
        {
            correctOrdered++;
        }   
    }
    accuracyPercent = 100.0f * correctOrdered / truePath.size();
    cout << "Node-sampling orderd accuracy: " << accuracyPercent << "%" << endl;
    
    int truePositives = 0;
    for (const auto& node : predictedSet)
    {
        if (trueSet.find(node) != trueSet.end())
            truePositives++;
    }
    int falsePositives = 0;
    for (const auto& node : predictedSet)
    {
        if (trueSet.find(node) == trueSet.end())
            falsePositives++;
    }

    accuracyPercent = 100.0f * max(0, truePositives - falsePositives) / trueSet.size();
    cout << "Node-sampling unordered accuracy: " << accuracyPercent << "%" << endl;
}

void RunEdgeSample(Router& endpoint)
{
    vector<MarkedPacketInfo> markedPackets = endpoint.GetMarkedPackets();
    /*
    for (auto markedPacket : markedPackets)
    {
        cout << "Marked Router: ";
        markedPacket.markedRouterAddress.PrintAddress();
        cout << ", Previous Router: ";
        markedPacket.previousHopAddress.PrintAddress();
        cout << ", Hop Count: " << markedPacket.hopCountFromSource << endl;
    }
    */

    unordered_map<string, unordered_map<string, int>> edgeCount;
    for (auto& markedPacket : markedPackets)
    { //Calculate the edge frequencies.
        string start = markedPacket.previousHopAddress.GetAddressAsString();
        string end = markedPacket.markedRouterAddress.GetAddressAsString();

        edgeCount[start][end]++;
    }

    //Flatten Edge Count Map
    vector<pair<pair<string, string>, int>> flattenEdges;
    for (auto& from : edgeCount)
    {
        for (auto& to : from.second)
        {
            flattenEdges.push_back({{from.first, to.first}, to.second});
        }
    }

    //Calculate Minimum Threshold
    int maxCount = 0;
    for (auto& edge : flattenEdges)
    {
        maxCount = max(maxCount, edge.second);
    }
    int thresholdCount = max(1, static_cast<int>(floor(maxCount * 0.1f)));

    //Filter Weak Edges
    vector<pair<pair<string,string>, int>> filteredEdges;
    for (auto& edge : flattenEdges)
    {
        if (edge.second >= thresholdCount)
            filteredEdges.push_back(edge);
    }

    //Sort Edges
    sort(filteredEdges.begin(), filteredEdges.end(),
        [] (const pair<pair<string, string>, int>& a, const pair<pair<string, string>, int>& b)
    {
        return a.second > b.second;
    });

    //Ground Truth
    vector<string> truePath = {
        "192.168.1.10",
        "192.168.1.10",
        "192.168.1.9",
        "192.168.1.8",
        "192.168.1.4",
        "192.168.1.6",
        "192.168.1.2",
        "192.168.1.0",
        "192.168.0.0",
    };

    unordered_set<string> trueSet = {
        "192.168.1.10",
        "192.168.1.9",
        "192.168.1.8",
        "192.168.1.4",
        "192.168.1.6",
        "192.168.1.2",
        "192.168.1.0",
        "192.168.0.0",
    };

    //Construct Path
    vector<string> predictedPath;
    float accuracyPercent;

    for (auto& edge : filteredEdges)
    {
        /*
        cout << "From: " << edge.first.first <<
                ", To: " << edge.first.second <<
                ", Frequency: " << edge.second << endl;
        */
        predictedPath.push_back(edge.first.second);
    }
    unordered_set<string> predictedSet(predictedPath.begin(), predictedPath.end());
    
    int correctOrdered = 0;
    int minLength = min(predictedPath.size(), truePath.size());
    for (int i = 0; i < minLength; ++i)
    {
        if (predictedPath[i] == truePath[i])
        {
            correctOrdered++;
        }   
    }
    accuracyPercent = 100.0f * correctOrdered / truePath.size();
    cout << "Edge-sampling orderd accuracy: " << accuracyPercent << "%" << endl;
    
    int truePositives = 0;
    for (const auto& node : predictedSet)
    {
        if (trueSet.find(node) != trueSet.end())
            truePositives++;
    }
    int falsePositives = 0;
    for (const auto& node : predictedSet)
    {
        if (trueSet.find(node) == trueSet.end())
            falsePositives++;
    }

    accuracyPercent = 100.0f * max(0, truePositives - falsePositives) / trueSet.size();
    cout << "Edge-sampling unordered accuracy: " << accuracyPercent << "%" << endl;
}

void RunSimulation(float prob, float rate)
{
    cout << "Running simulation for p = " << prob << ", x = " << rate << endl; 
    float markingProbability = prob;
    /*
    cout << "Please enter a marking probability (p): ";
    while (!(cin >> markingProbability))
    {
        cout << "The input provided was not a valid number.";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    */
    
    float attackerRate = rate;
    /*
    cout << "Please enter the attackers rate (x): ";
    while (!(cin >> attackerRate))
    {
        cout << "The input provided was not a valid number.";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    */

    //-------------------
    // Generate Topology
    // 10-20 Routers
    // 3, 4, or 5 branches
    //-------------------
    Router attacker({192, 168, 1, 0}, markingProbability);
    Router attacker2({192, 168, 0, 0}, markingProbability);
    Router r1({192, 169, 1, 1}, markingProbability);
    Router r2({192, 168, 1, 2}, markingProbability);
    Router r3({192, 168, 1, 3}, markingProbability);
    Router r4({192, 168, 1, 4}, markingProbability);
    Router r5({192, 168, 1, 5}, markingProbability);
    Router r6({192, 168, 1, 6}, markingProbability);
    Router r7({192, 168, 1, 7}, markingProbability);
    Router r8({192, 168, 1, 8}, markingProbability);
    Router r9({192, 168, 1, 9}, markingProbability);
    Router r10({192, 168, 1, 10}, markingProbability);
    Router victim({192, 168, 1, 255}, markingProbability);

    //Topology for One Attacker and One Normal User
    /*
    attacker.Connect(&r2);
    r2.Connect(&r6);
    r6.Connect(&r9);
    r3.Connect(&r7);
    r4.Connect(&r3);
    r9.Connect(&r10);
    r7.Connect(&r9);
    r1.Connect(&r4);
    r1.Connect(&r5);
    r4.Connect(&r8);
    r5.Connect(&r8);
    r7.Connect(&r8);
    r8.Connect(&r10);
    r10.Connect(&victim);
    */

    //Topology for Two Attackers and One Normal User
    attacker.Connect(&r2);
    attacker2.Connect(&r4);
    r2.Connect(&r6);
    r6.Connect(&r9);
    r9.Connect(&r10);
    r7.Connect(&r9);
    r1.Connect(&r3);
    r1.Connect(&r5);
    r4.Connect(&r8);
    r5.Connect(&r8);
    r7.Connect(&r8);
    r8.Connect(&r10);
    r3.Connect(&r10);
    r10.Connect(&victim);

    attacker.Start();
    attacker2.Start();
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
        attacker2.EnqueuePacket(maliciousPacket);
    }

    //Cleanup Routers
    attacker.Stop();
    attacker2.Stop();
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

    RunNodeSample(victim);
    RunEdgeSample(victim);
}

int main()
{
    srand(static_cast<unsigned>(time(nullptr)));

    RunSimulation(0.2, 10);
    RunSimulation(0.2, 100);
    RunSimulation(0.2, 1000);

    RunSimulation(0.4, 10);
    RunSimulation(0.4, 100);
    RunSimulation(0.4, 1000);

    RunSimulation(0.5, 10);
    RunSimulation(0.5, 100);
    RunSimulation(0.5, 1000);

    RunSimulation(0.6, 10);
    RunSimulation(0.6, 100);
    RunSimulation(0.6, 1000);

    RunSimulation(0.8, 10);
    RunSimulation(0.8, 100);
    RunSimulation(0.8, 1000);

    //-------------------
    // Traceback
    //-------------------
    /*
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
    */

    //Wait Terminal
    cin.get();

    return 0;
};