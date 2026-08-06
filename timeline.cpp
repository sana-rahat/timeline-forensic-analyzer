#include <iostream>
#include <filesystem>
#include <fstream>
#include <openssl/md5.h>
#include <cstdio>
#include <chrono>
#include <ctime>

using namespace std;
namespace fs = std::filesystem;

// Calculate MD5 hash using safe streaming (no memory overflow)
string md5File(const string &path) {
    ifstream file(path, ios::binary);
    if (!file) return "ERROR: Cannot read file";

    MD5_CTX ctx;
    MD5_Init(&ctx);

    char buffer[4096];
    while (file.good()) {
        file.read(buffer, sizeof(buffer));
        MD5_Update(&ctx, buffer, file.gcount());
    }

    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_Final(hash, &ctx);

    char buf[3];
    string result;

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(buf, "%02x", hash[i]);
        result += buf;
    }

    return result;
}

// Convert last write time to a readable string
string modifiedTime(const fs::path &path) {
    auto ftime = fs::last_write_time(path);
    auto sctp = chrono::time_point_cast<chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + chrono::system_clock::now()
    );

    time_t cftime = chrono::system_clock::to_time_t(sctp);
    char buffer[25];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&cftime));
    return string(buffer);
}

int main() {
    string folder;

    cout << "Enter folder path: ";
    getline(cin, folder);

    if (!fs::exists(folder)) {
        cout << "Error: Folder does not exist.\n";
        return 1;
    }

    cout << "\n===== TIMELINE REPORT =====\n";

    for (auto &entry : fs::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            cout << "\nFile: " << entry.path().filename().string() << "\n";

            try {
                cout << "Modified: " << modifiedTime(entry.path()) << "\n";
            }
            catch (...) {
                cout << "Modified: ERROR retrieving timestamp\n";
            }

            cout << "MD5: " << md5File(entry.path().string()) << "\n";
            cout << "-----------------------------\n";
        }
    }

    return 0;
}
