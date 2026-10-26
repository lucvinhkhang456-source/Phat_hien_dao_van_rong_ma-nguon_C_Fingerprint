#include "Fingerprint.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <cmath>
using namespace std;

// ============================================================
// TÁCH TOKEN
// ============================================================

vector<string> tokenize(const string& code)
{
    vector<string> tokens;

    string current;

    for (size_t i = 0; i < code.length(); i++)
    {
        char c = code[i];

        // Comment //
        if (c == '/' && i + 1 < code.length() && code[i + 1] == '/')
        {
            while (i < code.length() && code[i] != '\n')
                i++;

            continue;
        }

        // Comment /*
        if (c == '/' && i + 1 < code.length() && code[i + 1] == '*')
        {
            i += 2;

            while (i + 1 < code.length())
            {
                if (code[i] == '*' && code[i + 1] == '/')
                {
                    i++;
                    break;
                }

                i++;
            }

            continue;
        }

        // Khoảng trắng
        if (isspace(static_cast<unsigned char>(c)))
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }

            continue;
        }

        // Chữ hoặc số
        if (isalnum(static_cast<unsigned char>(c)) || c == '_')
        {
            current += c;
        }
        else
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }

            // Toán tử 2 ký tự
            if (i + 1 < code.length())
            {
                string two;
                two += c;
                two += code[i + 1];

                if (two == "==" ||
                    two == "!=" ||
                    two == "<=" ||
                    two == ">=" ||
                    two == "++" ||
                    two == "--" ||
                    two == "&&" ||
                    two == "||" ||
                    two == "+=" ||
                    two == "-=" ||
                    two == "*=" ||
                    two == "/=")
                {
                    tokens.push_back(two);
                    i++;
                    continue;
                }
            }

            string one;
            one += c;

            tokens.push_back(one);
        }
    }

    if (!current.empty())
        tokens.push_back(current);

    return tokens;
}

// ============================================================
// CHUẨN HÓA TOKEN
// ============================================================

vector<string> normalizeTokens(const vector<string>& tokens)
{
    vector<string> result;

    // Các từ khóa C/C++
    unordered_set<string> keywords =
    {
        "int",
        "float",
        "double",
        "char",
        "bool",
        "void",
        "long",
        "short",
        "signed",
        "unsigned",
        "const",
        "if",
        "else",
        "for",
        "while",
        "do",
        "switch",
        "case",
        "break",
        "continue",
        "return",
        "class",
        "struct",
        "public",
        "private",
        "protected",
        "using",
        "namespace",
        "include",
        "true",
        "false"
    };

    for (const string& token : tokens)
    {
        // Số
        bool isNumber = true;

        for (char c : token)
        {
            if (!isdigit(static_cast<unsigned char>(c)))
            {
                isNumber = false;
                break;
            }
        }

        if (isNumber && !token.empty())
        {
            result.push_back("NUM");
            continue;
        }

        // String
        if (token.length() >= 2 &&
            token.front() == '"' &&
            token.back() == '"')
        {
            result.push_back("STRING");
            continue;
        }

        // Từ khóa giữ nguyên
        if (keywords.count(token))
        {
            result.push_back(token);
            continue;
        }

        // Tên biến / tên hàm
        if (!token.empty() &&
            (isalpha(static_cast<unsigned char>(token[0])) ||
             token[0] == '_'))
        {
            result.push_back("VAR");
            continue;
        }

        // Ký hiệu giữ nguyên
        result.push_back(token);
    }

    return result;
}

// ============================================================
// TẠO K-GRAM
// ============================================================

vector<string> createKGrams(
    const vector<string>& tokens,
    int k)
{
    vector<string> kgrams;

    if (tokens.size() < static_cast<size_t>(k))
        return kgrams;

    for (size_t i = 0; i <= tokens.size() - k; i++)
    {
        string gram;

        for (int j = 0; j < k; j++)
        {
            if (j > 0)
                gram += " ";

            gram += tokens[i + j];
        }

        kgrams.push_back(gram);
    }

    return kgrams;
}

// ============================================================
// HASH K-GRAM
// ============================================================

vector<uint64_t> hashKGrams(
    const vector<string>& kgrams)
{
    vector<uint64_t> hashes;

    for (const string& gram : kgrams)
    {
        uint64_t hash = 1469598103934665603ULL;

        for (unsigned char c : gram)
        {
            hash ^= c;
            hash *= 1099511628211ULL;
        }

        hashes.push_back(hash);
    }

    return hashes;
}

// ============================================================
// WINNOWING
// ============================================================

vector<Fingerprint> winnowing(
    const vector<uint64_t>& hashes,
    int windowSize)
{
    vector<Fingerprint> fingerprints;

    if (hashes.empty())
        return fingerprints;

    if (hashes.size() <= static_cast<size_t>(windowSize))
    {
        uint64_t minHash = hashes[0];
        int minPos = 0;

        for (size_t i = 1; i < hashes.size(); i++)
        {
            if (hashes[i] <= minHash)
            {
                minHash = hashes[i];
                minPos = static_cast<int>(i);
            }
        }

        fingerprints.push_back(
            {
                minHash,
                minPos
            }
        );

        return fingerprints;
    }

    int lastPosition = -1;

    for (size_t i = 0;
         i + windowSize <= hashes.size();
         i++)
    {
        uint64_t minHash = hashes[i];
        int minPos = static_cast<int>(i);

        for (size_t j = i + 1;
             j < i + windowSize;
             j++)
        {
            if (hashes[j] <= minHash)
            {
                minHash = hashes[j];
                minPos = static_cast<int>(j);
            }
        }

        // Không thêm trùng vị trí
        if (minPos != lastPosition)
        {
            fingerprints.push_back(
                {
                    minHash,
                    minPos
                }
            );

            lastPosition = minPos;
        }
    }

    return fingerprints;
}

// ============================================================
// TẠO FINGERPRINT HOÀN CHỈNH
// ============================================================

vector<Fingerprint> generateFingerprints(
    const string& code,
    int k,
    int windowSize)
{
    vector<string> tokens = tokenize(code);

    vector<string> normalized =
        normalizeTokens(tokens);

    vector<string> kgrams =
        createKGrams(normalized, k);

    vector<uint64_t> hashes =
        hashKGrams(kgrams);

    return winnowing(
        hashes,
        windowSize
    );
}

// ============================================================
// COSINE SIMILARITY
// ============================================================

double cosineSimilarity(
    const vector<Fingerprint>& a,
    const vector<Fingerprint>& b)
{
    if (a.empty() || b.empty())
        return 0.0;

    unordered_map<uint64_t, int> freqA;
    unordered_map<uint64_t, int> freqB;

    for (const auto& fp : a)
        freqA[fp.hash]++;

    for (const auto& fp : b)
        freqB[fp.hash]++;

    unordered_set<uint64_t> allHashes;

    for (const auto& item : freqA)
        allHashes.insert(item.first);

    for (const auto& item : freqB)
        allHashes.insert(item.first);

    double dot = 0.0;
    double normA = 0.0;
    double normB = 0.0;

    for (uint64_t h : allHashes)
    {
        double x = freqA[h];
        double y = freqB[h];

        dot += x * y;
        normA += x * x;
        normB += y * y;
    }

    if (normA == 0 || normB == 0)
        return 0.0;

    return dot /
           (sqrt(normA) * sqrt(normB));
}

// ============================================================
// FINGERPRINT OVERLAP
// ============================================================

double fingerprintOverlap(
    const vector<Fingerprint>& a,
    const vector<Fingerprint>& b)
{
    if (a.empty() || b.empty())
        return 0.0;

    unordered_set<uint64_t> setA;
    unordered_set<uint64_t> setB;

    for (const auto& fp : a)
        setA.insert(fp.hash);

    for (const auto& fp : b)
        setB.insert(fp.hash);

    int common = 0;

    for (uint64_t h : setA)
    {
        if (setB.count(h))
            common++;
    }

    return static_cast<double>(common) /
           static_cast<double>(
               min(setA.size(), setB.size())
           );
}

// ============================================================
// TOKEN SIMILARITY
// ============================================================

double tokenSimilarity(
    const vector<string>& a,
    const vector<string>& b)
{
    if (a.empty() || b.empty())
        return 0.0;

    unordered_map<string, int> freqA;
    unordered_map<string, int> freqB;

    for (const string& token : a)
        freqA[token]++;

    for (const string& token : b)
        freqB[token]++;

    unordered_set<string> allTokens;

    for (const auto& item : freqA)
        allTokens.insert(item.first);

    for (const auto& item : freqB)
        allTokens.insert(item.first);

    double dot = 0.0;
    double normA = 0.0;
    double normB = 0.0;

    for (const string& token : allTokens)
    {
        double x = freqA[token];
        double y = freqB[token];

        dot += x * y;
        normA += x * x;
        normB += y * y;
    }

    if (normA == 0 || normB == 0)
        return 0.0;

    return dot /
           (sqrt(normA) * sqrt(normB));
}

