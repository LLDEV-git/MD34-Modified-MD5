#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <filesystem>

using namespace std;

static const char HEX[] = "0123456789abcdef";

uint64_t rotl64(uint64_t x, unsigned r) {
    return (x << r) | (x >> (64 - r));
}

string md34_hash(const string& path) {
    ifstream file(path, ios::binary);

    uint64_t h[17] = {
        0x6A09E667F3BCC909ULL,
        0xBB67AE8584CAA73BULL,
        0x3C6EF372FE94F82BULL,
        0xA54FF53A5F1D36F1ULL,
        0x510E527FADE682D1ULL,
        0x9B05688C2B3E6C1FULL,
        0x1F83D9ABFB41BD6BULL,
        0x5BE0CD19137E2179ULL,
        0xCBBB9D5DC1059ED8ULL,
        0x629A292A367CD507ULL,
        0x9159015A3070DD17ULL,
        0x152FECD8F70E5939ULL,
        0x67332667FFC00B31ULL,
        0x8EB44A8768581511ULL,
        0xDB0C2E0D64F98FA7ULL,
        0x47B5481DBEFA4FA4ULL,
        0xD6E8FEB86659FD93ULL
    };

    uint64_t length = 0;
    unsigned char buffer[4096];

    while (file) {
        file.read(
            reinterpret_cast<char*>(buffer),
            sizeof(buffer)
        );

        streamsize count = file.gcount();

        for (streamsize i = 0; i < count; ++i) {
            uint64_t x = buffer[i] +
                         length *
                         0x9E3779B97F4A7C15ULL;

            for (int j = 0; j < 17; ++j) {
                uint64_t next = h[(j + 1) % 17];

                h[j] ^= x + next;
                h[j] = rotl64(
                    h[j],
                    static_cast<unsigned>((j * 7 + 13) % 63 + 1)
                );

                h[j] *=
                    0x100000001B3ULL +
                    static_cast<uint64_t>(j) *
                    0x9E3779B9ULL;

                x ^= h[j] +
                     static_cast<uint64_t>(j) *
                     0xC2B2AE3D27D4EB4FULL;
            }

            ++length;
        }
    }

    for (int round = 0; round < 34; ++round) {
        for (int j = 0; j < 17; ++j) {
            uint64_t a = h[j];
            uint64_t b = h[(j + 1) % 17];
            uint64_t c = h[(j + 5) % 17];

            h[j] ^= rotl64(b, 11);
            h[j] += rotl64(c, 23);
            h[j] *=
                0x9E3779B185EBCA87ULL +
                static_cast<uint64_t>(round + j);

            h[j] ^= h[j] >> 29;
            h[j] ^= a >> 17;
        }
    }

    for (int j = 0; j < 17; ++j) {
        h[j] ^= length;
        h[j] ^= rotl64(
            h[(j + 3) % 17],
            static_cast<unsigned>((j + 1) * 3)
        );

        h[j] *= 0xFF51AFD7ED558CCDULL;
        h[j] ^= h[j] >> 32;
    }

    string result;
    result.reserve(272);

    for (int j = 0; j < 17; ++j) {
        for (int shift = 60; shift >= 0; shift -= 4) {
            result += HEX[(h[j] >> shift) & 0xFULL];
        }
    }

    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: md34sum /path\n";
        return 1;
    }

    bool only_hash = false;
    string path;

    if (string(argv[1]) == "-h") {
        only_hash = true;

        if (argc < 3) {
            cout << "Usage: md34sum /path\n";
            return 1;
        }

        path = argv[2];
    }
    else {
        path = argv[1];
    }

    if (!filesystem::exists(path)) {
        cerr << "'" << path
             << "' : No such file or directory\n";
        return 1;
    }

    if (filesystem::is_directory(path)) {
        cerr << "'" << path
             << "' : Is a directory\n";
        return 1;
    }

    ifstream test(path, ios::binary);

    if (!test) {
        cerr << "'" << path
             << "' : No such file or directory\n";
        return 1;
    }

    test.close();

    string hash = md34_hash(path);

    if (only_hash) {
        cout << hash << '\n';
    }
    else {
        cout << hash << ' ' << path << '\n';
    }

    return 0;
}