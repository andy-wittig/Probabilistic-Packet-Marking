#include <iostream>
#include <cassert>
#include <vector>
#include <array>
#include <mutex>
#include <random>

using namespace std;

class IPAddress
{
    public:
        array<int, 4> address;
        IPAddress(array<int, 4> ip) : address(ip) {}

        void PrintAddress() const
        {
            cout << address[0] << "."
                 << address[1] << "."
                 << address[2] << "."
                 << address[3];
        }

        bool operator==(const IPAddress& other) const
        {
            return address == other.address;
        }
};

struct Packet
{
    int hopCount = 0;
    bool marked = false;
    IPAddress source;
    IPAddress destination;
    Packet(IPAddress s, IPAddress d) : source(s), destination(d) {}
};

class Router
{
    private:
        static mutex coutMutex;
        mutex routerMutex;

        IPAddress _ipAddress;
        vector<Router*> neighborRouters;
        float _markProbability;
    public:
        Router(array<int, 4> ip, float markProbability) : _ipAddress(ip), _markProbability(markProbability)
        {
            assert(markProbability >= 0 && markProbability <= 1 && "Probability 'markProbability' must be a float between 0 and 1.");
        }

        void Connect(Router* other)
        {
            neighborRouters.push_back(other);
        }

        bool GetMarkingProbability() const
        { //Returns true if packet is set to be marked.
            thread_local static mt19937 generator(random_device{}());
            uniform_real_distribution<float> dist(0.f, 1.f);
            return dist(generator) <= _markProbability;
        }

        void ForwardPacket(Packet& packet)
        {
            Router* nextRouter = nullptr;

            { //Mutex Lock
                lock_guard<mutex> lock(routerMutex);

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
                if (!packet.marked && GetMarkingProbability())
                {
                    packet.marked = true;
                    {
                        lock_guard<mutex> lock(coutMutex);
                        cout << "Router @: ";
                        _ipAddress.PrintAddress();
                        cout << " marked the packet." << endl;
                    }
                }

                if (_ipAddress == packet.destination)
                {
                    {
                        lock_guard<mutex> lock(coutMutex);
                        cout << "Packet Reached Destination @: ";
                        _ipAddress.PrintAddress();
                        cout << " in " << packet.hopCount << " hop(s)." << endl;
                    }
                    return;
                }

                packet.hopCount++;

                //Forwarding Logic
                if (!neighborRouters.empty())
                { //Forward to first neighbor.
                    {
                        lock_guard<mutex> lock(coutMutex);
                        cout << "Packet forwarding from ";
                        _ipAddress.PrintAddress();
                        cout << " to ";
                        neighborRouters[0]->RequestAddress().PrintAddress();
                        cout << endl;
                    }

                    nextRouter = neighborRouters[0];
                }
            }

            if (nextRouter)
            {
                nextRouter->ForwardPacket(packet);
            }
        } //End of Mutex

        const IPAddress& RequestAddress() const
        {
            return _ipAddress;
        }
};