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
            float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            return (r <= _markProbability);
        }

        void ForwardPacket(Packet& packet)
        {
            //Drop packets.
            if (packet.hopCount > 15)
            {
                cout << "TTL has been exceeded, packet dropped @: ";
                _ipAddress.PrintAddress();
                cout << endl;
                return;
            }

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
                cout << " in " << packet.hopCount << " hop(s)." << endl;
                return;
            }

            //Forwarding Logic
            if (!neighborRouters.empty())
            { //Forward to first neighbor.
                cout << "Packet forwarding from ";
                _ipAddress.PrintAddress();
                cout << " to ";
                neighborRouters[0]->RequestAddress().PrintAddress();
                cout << endl;

                neighborRouters[0]->ForwardPacket(packet);
            }
        }

        const IPAddress& RequestAddress() const
        {
            return _ipAddress;
        }
};