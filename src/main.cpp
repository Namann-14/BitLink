#include <iostream>
#include <stdexcept>
#include "bencode/bencode.h"
#include "torrent/torrent.h"
#include "crypto/sha1.h"
#include "net/http.h"

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
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}