#ifndef FINGERPRINT_H
#define FINGERPRINT_H

#include <string>
#include <vector>
#include <cstdint>

using namespace std;

struct Fingerprint
{
    uint64_t hash;
    int position;
};

vector<string> tokenize(const string& code);

vector<string> normalizeTokens(const vector<string>& tokens);

vector<string> createKGrams(
    const vector<string>& tokens,
    int k
);

vector<uint64_t> hashKGrams(
    const vector<string>& kgrams
);

vector<Fingerprint> winnowing(
    const vector<uint64_t>& hashes,
    int windowSize
);

vector<Fingerprint> generateFingerprints(
    const string& code,
    int k = 5,
    int windowSize = 4
);

double cosineSimilarity(
    const vector<Fingerprint>& a,
    const vector<Fingerprint>& b
);

double fingerprintOverlap(
    const vector<Fingerprint>& a,
    const vector<Fingerprint>& b
);

double tokenSimilarity(
    const vector<string>& a,
    const vector<string>& b
);

void printFingerprintTable(
    const string& programName,
    const vector<Fingerprint>& fingerprints
);

#endif