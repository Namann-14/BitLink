#include "torrent/torrent.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "crypto/sha1.h"

namespace torrent {

    std::string loadFile(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    TorrentInfo extractMetadata(const bencode::BencodeValue& root) {
        TorrentInfo info;

        if (!std::holds_alternative<bencode::BencodeDict>(root)) {
            throw std::runtime_error("Torrent file root must be a dictionary");
        }

        const auto& rootDict = std::get<bencode::BencodeDict>(root);

        // Extract announce URL
        auto announceIt = rootDict.find("announce");
        if (announceIt != rootDict.end() && std::holds_alternative<std::string>(announceIt->second)) {
            info.announce = std::get<std::string>(announceIt->second);
        }

        // Extract info dictionary
        auto infoIt = rootDict.find("info");
        if (infoIt == rootDict.end() || !std::holds_alternative<bencode::BencodeDict>(infoIt->second)) {
            throw std::runtime_error("Torrent file missing info dictionary");
        }

        const auto& infoDict = std::get<bencode::BencodeDict>(infoIt->second);

        // Extract name
        auto nameIt = infoDict.find("name");
        if (nameIt != infoDict.end() && std::holds_alternative<std::string>(nameIt->second)) {
            info.name = std::get<std::string>(nameIt->second);
        }

        // Extract piece length
        auto pieceLenIt = infoDict.find("piece length");
        if (pieceLenIt != infoDict.end() && std::holds_alternative<int64_t>(pieceLenIt->second)) {
            info.pieceLength = std::get<int64_t>(pieceLenIt->second);
        }

        // Extract pieces
        auto piecesIt = infoDict.find("pieces");
        if (piecesIt != infoDict.end() && std::holds_alternative<std::string>(piecesIt->second)) {
            info.pieces = std::get<std::string>(piecesIt->second);
            if (info.pieces.size() % 20 != 0) {
                throw std::runtime_error("Invalid pieces length");
            }
        }

        // Calculate info_hash
        std::string encodedInfo = bencode::encode(infoIt->second);
        info.infoHash = crypto::sha1(encodedInfo);
        info.infoHashHex = crypto::toHex(info.infoHash);

        return info;
    }

    std::vector<Peer> extractPeers(const bencode::BencodeValue& trackerResponse) {
        std::vector<Peer> peers;

        if (!std::holds_alternative<bencode::BencodeDict>(trackerResponse)) {
            return peers;
        }

        const auto& rootDict = std::get<bencode::BencodeDict>(trackerResponse);
        auto peersIt = rootDict.find("peers");
        
        if (peersIt != rootDict.end() && std::holds_alternative<bencode::BencodeList>(peersIt->second)) {
            const auto& peerList = std::get<bencode::BencodeList>(peersIt->second);
            for (const auto& peerVal : peerList) {
                if (std::holds_alternative<bencode::BencodeDict>(peerVal)) {
                    const auto& peerDict = std::get<bencode::BencodeDict>(peerVal);
                    
                    auto ipIt = peerDict.find("ip");
                    auto portIt = peerDict.find("port");
                    
                    if (ipIt != peerDict.end() && std::holds_alternative<std::string>(ipIt->second) &&
                        portIt != peerDict.end() && std::holds_alternative<int64_t>(portIt->second)) {
                        
                        Peer p;
                        p.ip = std::get<std::string>(ipIt->second);
                        p.port = static_cast<int>(std::get<int64_t>(portIt->second));
                        peers.push_back(p);
                    }
                }
            }
        }

        return peers;
    }

}
