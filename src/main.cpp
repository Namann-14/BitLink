#include <iostream>
#include <stdexcept>
#include "bencode/bencode.h"
#include "torrent/torrent.h"
#include "crypto/sha1.h"
#include "net/http.h"
#include "net/tcp.h"

int main() {
    try {
        std::cout << "Loading torrent file...\n";
        std::string data = torrent::loadFile("data/ubuntu-26.04-desktop-amd64.iso.torrent");
        
        int i = 0;
        bencode::BencodeValue root = bencode::parse(data, i);

        std::cout << "--- Torrent File Structure ---\n";
        bencode::printBencode(root);
        
        std::cout << "\n--- Extracted Metadata ---\n";
        torrent::TorrentInfo info = torrent::extractMetadata(root);
        
        std::cout << "Announce: " << info.announce << "\n";
        std::cout << "Name: " << info.name << "\n";
        std::cout << "Piece Length: " << info.pieceLength << "\n";
        std::cout << "Pieces (Hash Count): " << (info.pieces.size() / 20) << "\n";
        std::cout << "Info Hash (Hex): " << info.infoHashHex << "\n";

        std::cout << "\n--- Requesting Tracker ---\n";
        std::string peerId = "-BL0001-123456789012";
        std::string url = info.announce + 
            "?info_hash=" + crypto::urlEncode(info.infoHash) +
            "&peer_id=" + crypto::urlEncode(peerId) +
            "&port=6881&uploaded=0&downloaded=0&left=0&compact=1";
            
        std::cout << "GET " << url << "\n";
        std::string trackerResponse = net::httpGet(url);
        
        std::cout << "Received " << trackerResponse.size() << " bytes from tracker.\n";
        
        int j = 0;
        bencode::BencodeValue trackerRoot = bencode::parse(trackerResponse, j);
        std::cout << "--- Tracker Response ---\n";
        bencode::printBencode(trackerRoot);
        
        std::vector<torrent::Peer> peers = torrent::extractPeers(trackerRoot);
        torrent::Peer* targetPeer = nullptr;
        for (auto& p : peers) {
            if (p.ip.find('.') != std::string::npos) {
                targetPeer = &p;
                break;
            }
        }

        if (!targetPeer) {
            std::cout << "No IPv4 peers found in tracker response.\n";
            return 0;
        }

        std::cout << "\n--- Connecting to Peer: " << targetPeer->ip << ":" << targetPeer->port << " ---\n";
        
        net::TcpSocket sock;
        sock.connect(targetPeer->ip, targetPeer->port);
        std::cout << "Connected!\n";

        std::vector<uint8_t> handshake;
        handshake.push_back(19);
        std::string protocol = "BitTorrent protocol";
        handshake.insert(handshake.end(), protocol.begin(), protocol.end());
        for (int k = 0; k < 8; k++) handshake.push_back(0); // 8 reserved bytes
        handshake.insert(handshake.end(), info.infoHash.begin(), info.infoHash.end());
        handshake.insert(handshake.end(), peerId.begin(), peerId.end());

        std::cout << "Sending Handshake...\n";
        sock.send(handshake);

        std::cout << "Waiting for peer response (68 bytes)...\n";
        std::vector<uint8_t> response = sock.receive(68);

        std::cout << "Received Peer Handshake!\n";
        if (response[0] == 19 && std::string(response.begin() + 1, response.begin() + 20) == "BitTorrent protocol") {
            std::cout << "[SUCCESS] Protocol Match: BitTorrent protocol\n";
            std::string peerHash(response.begin() + 28, response.begin() + 48);
            if (peerHash == info.infoHash) {
                std::cout << "[SUCCESS] Info Hash Match! We are officially in the swarm!\n";
            } else {
                std::cout << "[ERROR] Info Hash Mismatch!\n";
            }
        } else {
            std::cout << "[ERROR] Invalid protocol string from peer.\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}