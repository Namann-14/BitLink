#include <iostream>
#include <stdexcept>
#include "bencode/bencode.h"
#include "torrent/torrent.h"

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
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}