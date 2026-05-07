#pragma once
#include <string>
#include <cstdint>
#include "bencode/bencode.h"

namespace torrent {

    struct TorrentInfo {
        std::string announce;
        std::string name;
        int64_t pieceLength;
        std::string pieces;
        std::string infoHash;
        std::string infoHashHex;
    };

    struct Peer {
        std::string ip;
        int port;
    };

    std::string loadFile(const std::string& path);
    TorrentInfo extractMetadata(const bencode::BencodeValue& root);
    std::vector<Peer> extractPeers(const bencode::BencodeValue& trackerResponse);

}
