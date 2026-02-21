#include <iostream>
#include <cassert>
#include <vector>
#include <array>
#include <random>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <string>

using namespace std;

class IPAddress
{
    public:
        array<int, 4> address;
        IPAddress() : address{0, 0, 0, 0} {}
        IPAddress(array<int, 4> ip) : address(ip) {}

        void PrintAddress() const
        {
            cout << address[0] << "."
                 << address[1] << "."
                 << address[2] << "."
                 << address[3];
        }

        string GetAddressAsString() const
        {
            return to_string(address[0]) + "." +
                   to_string(address[1]) + "." +
                   to_string(address[2]) + "." +
                   to_string(address[3]);
        }

        bool operator==(const IPAddress& other) const
        {
            return address == other.address;
        }

        bool operator!=(const IPAddress& other) const
        {
            return address != other.address;
        }
};

struct Packet
{
    int hopCount = 0;
    bool marked = false;
    IPAddress source;
    IPAddress destination;
    IPAddress previousRouter;
    IPAddress markingRouter;
    IPAddress markingPreviousRouter;
    Packet(IPAddress s, IPAddress d) : source(s), destination(d) {}
};

struct MarkedPacketInfo
{
    IPAddress markedRouterAddress;
    IPAddress previousHopAddress;
    int hopCountFromSource;
    MarkedPacketInfo(IPAddress s, IPAddress d, int h) : markedRouterAddress(s), 
                                                    previousHopAddress(d),
                                                    hopCountFromSource(h) {}
};

class Router
{
    private:
        //Packet Queue and Threading
        queue<Packet> packetQueue;
        condition_variable cv; //Used to avoid spin waiting.
        thread worker;
        bool running = false;
        //Mutexes
        static mutex coutMutex;
        mutex routerMutex;
        //Router
        IPAddress _ipAddress; //Routers personal address
        vector<Router*> neighborRouters;
        vector<MarkedPacketInfo> markedPackets;
        float _markProbability; //0-1
        long packetDelay = 0.1; //ms
    public:
        Router(array<int, 4> ip, float markProbability) : _ipAddress(ip), _markProbability(markProbability)
        {
            assert(markProbability >= 0 && markProbability <= 1 && "Probability 'markProbability' must be a float between 0 and 1.");
        }

        ~Router()
        {
            Stop();
        }

        void Start()
        {
            running = true;
            worker = thread(&Router::ProcessPackets, this);
        }

        void Stop()
        {
            {
                lock_guard<mutex> lock(routerMutex);
                running = false;
            }
            cv.notify_all();
            if (worker.joinable()) { worker.join(); }
        }

        void Connect(Router* other)
        {
            neighborRouters.push_back(other);
        }

        bool GetMarkingProbability() const
        { //Returns true if packet is randomly set to be marked.
            thread_local static mt19937 generator(random_device{}());
            uniform_real_distribution<float> dist(0.f, 1.f);
            return dist(generator) < _markProbability;
        }

        void EnqueuePacket(Packet packet)
        {
            {
                lock_guard<mutex> lock(routerMutex);
                packetQueue.push(packet);
            }
            cv.notify_one();
        }

        void ProcessPackets()
        {
            while (true)
            {
                Packet packet(IPAddress({0,0,0,0}), IPAddress({0,0,0,0}));

                {
                    unique_lock<mutex> lock(routerMutex);
                    cv.wait(lock, [this]() { //Avoids spin waiting.
                        return !packetQueue.empty() || !running;
                    });

                    if (!running && packetQueue.empty()) { break; }
                        
                    packet = packetQueue.front();
                    packetQueue.pop();
                }

                HandlePacket(packet);
            }
        }

        void HandlePacket(Packet& packet)
        {
            Router* nextRouter = nullptr;

            //Drop packets.
            if (packet.hopCount > 15)
            {
                {
                    lock_guard<mutex> lock(coutMutex);
                    cout << "TTL has been exceeded, packet dropped @: ";
                    _ipAddress.PrintAddress();
                    cout << endl;
                }
                return;
            }

            //Probability of Marking Packet.
            if (GetMarkingProbability() && _ipAddress != packet.destination)
            {
                packet.marked = true;
                packet.markingRouter = _ipAddress;
                packet.markingPreviousRouter = packet.previousRouter;
                {
                    /*
                    lock_guard<mutex> lock(coutMutex);
                    cout << "Router @: ";
                    _ipAddress.PrintAddress();
                    cout << " marked the packet." << endl;
                    */
                }
            }

            if (_ipAddress == packet.destination)
            {
                if (packet.marked)
                {
                    lock_guard<mutex> lock(routerMutex);
                    markedPackets.emplace_back(packet.markingRouter, packet.markingPreviousRouter, packet.hopCount);
                }

                {
                    /*
                    lock_guard<mutex> lock(coutMutex);
                    cout << "Packet Reached Destination @: ";
                    _ipAddress.PrintAddress();
                    cout << " in " << packet.hopCount << " hop(s)." << endl;
                    */
                }
                return;
            }

            packet.hopCount++;

            //Forwarding Logic
            if (!neighborRouters.empty())
            {
                packet.previousRouter = _ipAddress;

                {
                    /*
                    lock_guard<mutex> lock(coutMutex);
                    cout << "Packet forwarding from ";
                    _ipAddress.PrintAddress();
                    cout << " to ";
                    neighborRouters[0]->RequestAddress().PrintAddress();
                    cout << endl;
                    */
                }

                //Select Random Router Next
                thread_local static mt19937 gen(random_device{}());
                uniform_int_distribution<size_t> dist(0, neighborRouters.size() - 1);
                nextRouter = neighborRouters[dist(gen)];
            }

            if (nextRouter)
            {
                this_thread::sleep_for(chrono::milliseconds(packetDelay)); //Packet delay
                nextRouter->EnqueuePacket(packet);
            }
        }

        const IPAddress& RequestAddress() const
        {
            return _ipAddress;
        }

        vector<MarkedPacketInfo> GetMarkedPackets() const
        {
            return markedPackets;
        }
};