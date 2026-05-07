#include <iostream>
#include <stdexcept>
#include <fstream>
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
        
        bool success = false;
        for (auto& p : peers) {
            std::cout << "\n--- Attempting Peer: " << p.ip << ":" << p.port << " ---\n";
            try {
                net::TcpSocket sock;
                sock.connect(p.ip, p.port);
                std::cout << "Connected!\n";

                std::vector<uint8_t> handshake;
                handshake.push_back(19);
                std::string protocol = "BitTorrent protocol";
                handshake.insert(handshake.end(), protocol.begin(), protocol.end());
                for (int k = 0; k < 8; k++) handshake.push_back(0);
                handshake.insert(handshake.end(), info.infoHash.begin(), info.infoHash.end());
                handshake.insert(handshake.end(), peerId.begin(), peerId.end());

                sock.send(handshake);
                std::vector<uint8_t> response = sock.receive(68);

                if (response[0] == 19 && std::string(response.begin() + 1, response.begin() + 20) == "BitTorrent protocol") {
                    std::string peerHash(response.begin() + 28, response.begin() + 48);
                    if (peerHash == info.infoHash) {
                        std::cout << "[SUCCESS] Handshake successful!\n";

                        std::cout << "Sending 'Interested' message...\n";
                        std::vector<uint8_t> interestedMsg = {0, 0, 0, 1, 2};
                        sock.send(interestedMsg);

                        bool unchoked = false;
                        bool hasBitfield = false;
                        int msgCount = 0;
                        while (!unchoked && msgCount < 5) { // Try up to 5 messages
                            net::PeerMessage msg = sock.receiveMessage();
                            msgCount++;
                            if (msg.id == 5) {
                                std::cout << "-> Peer sent Bitfield! They have pieces.\n";
                                hasBitfield = true;
                            } else if (msg.id == 1) {
                                std::cout << "-> Peer sent UNCHOKE!\n";
                                unchoked = true;
                            }
                        }

                        if (!unchoked || !hasBitfield) {
                            std::cout << "Peer didn't unchoke or has no pieces. Trying next peer...\n";
                            continue;
                        }

                        std::cout << "\n--- Requesting Block ---\n";
                        std::vector<uint8_t> reqMsg = {
                            0, 0, 0, 13, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 0
                        };
                        sock.send(reqMsg);

                        bool blockReceived = false;
                        while (!blockReceived) {
                            net::PeerMessage msg = sock.receiveMessage();
                            if (msg.id == 7) {
                                std::cout << "-> Peer sent PIECE! We got the data!\n";
                                if (msg.payload.size() >= 8) {
                                    int blockSize = msg.payload.size() - 8;
                                    std::ofstream out("ubuntu_piece0_block0.dat", std::ios::binary);
                                    out.write(reinterpret_cast<const char*>(msg.payload.data() + 8), blockSize);
                                    out.close();
                                    std::cout << "\n[SUCCESS] Block successfully written to 'ubuntu_piece0_block0.dat'!\n";
                                }
                                blockReceived = true;
                            }
                        }

                        success = true;
                        break; // Stop after downloading one block!
                    }
                }
            } catch (const std::exception& e) {
                std::cout << "Failed: " << e.what() << "\n";
            }
        }

        if (!success) {
            std::cout << "\nFailed to download block from any peer.\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}