#include <iostream>
#include <cassert>
#include <vector>
#include <array>

using namespace std;

class IPAddress
{
    public:
        array<int, 4> address;
        IPAddress(array<int, 4> ip) : address(ip) {}

        void PrintAddress()
        {
            cout << address[0] << "."
                 << address[1] << "."
                 << address[2] << "."
                 << address[0];
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

        bool GetMarkingProbability()
        { //Returns true if packet is set to be marked.
            float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            return (r <= _markProbability);
        }

        void ForwardPacket(Packet& packet)
        {
            //Probability of marking packet.
            if (!packet.marked && GetMarkingProbability())
            {
                packet.marked = true;
                cout << "Router @: ";
                _ipAddress.PrintAddress();
                cout << " marked the packet." << endl;
            }

            packet.hopCount++;

            if (_ipAddress == packet.destination)
            {
                cout << "Packet Reached Destination @: ";
                _ipAddress.PrintAddress();
                cout << " in " << packet.hopCount << "hops." << endl;
            }

            //Forwarding Logic
            if (!neighborRouters.empty())
            { //Forward to first neighbor.
                neighborRouters[0]->ForwardPacket(packet);
            }
        }

        IPAddress RequestAddress()
        {
            return _ipAddress;
        }
};