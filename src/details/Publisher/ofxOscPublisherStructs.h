//
//  ofxOscPublisherStructs.h
//  ofxPubSubOscDev
//
//  Created by ISHII 2bit on 2016/05/31.
//
//

#pragma once

#ifndef ofxOscPublisherStructs_h
#define ofxOscPublisherStructs_h

#include <string>

namespace ofx {
    namespace PubSubOsc {
        namespace Publish {
            struct IP {
                IP(const IP &ip)
                : ip(ip.ip) {}
                
                IP(const std::string &ip)
                : ip(ip) {}
                
                bool operator<(const IP &rhs) const {
                    return ip < rhs.ip;
                }
                
                std::string ip;
            private:
                IP();
            };
            
            struct Destination {
                Destination() {}
                Destination(const Destination &destination)
                : ip(destination.ip)
                , port(destination.port) {}
                
                Destination(const std::string &ip, int port)
                : ip(ip)
                , port(port) {}
                
                inline bool operator<(const Destination &rhs) const {
                    return (ip != rhs.ip) ? (ip < rhs.ip) : (port < rhs.port);
                }
                
                inline bool operator!=(const Destination &rhs) const {
                    return ip != rhs.ip || port != rhs.port;
                }
                
                std::string ip;
                int port;
            private:
            };
            
            struct DestinationWithAddress {
                DestinationWithAddress(const DestinationWithAddress &destination)
                : destination(destination.destination)
                , address(destination.address) {}
                
                DestinationWithAddress(const std::string &ip, int port, const std::string &address)
                : destination(ip, port)
                , address(address) {}
                
                inline bool operator<(const DestinationWithAddress &rhs) const {
                    return (destination != rhs.destination) ? (destination < rhs.destination) : (address < rhs.address);
                }
                
                operator Destination() const {
                    return destination;
                }
                
                inline operator const Destination&() const {
                    return destination;
                }
                
                Destination destination;
                std::string address;
            private:
                DestinationWithAddress();
            };
        };
    };
};

#endif /* ofxOscPublisherStructs_h */
