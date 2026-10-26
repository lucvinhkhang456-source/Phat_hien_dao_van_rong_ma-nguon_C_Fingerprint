#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include "Fingerprint.h"
#include "KNN.h"

using namespace std;
namespace fs = filesystem;


// ============================================================
// ĐỌC FILE
// ============================================================

string readFile(const string& filename)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cout << "Khong the mo file: "
             << filename
             << endl;

        return "";
    }

    stringstream buffer;

    buffer << file.rdbuf();

    file.close();

    return buffer.str();
}


// ============================================================
// LẤY TÊN FILE
// ============================================================

string getFileName(const string& path)
{
    fs::path filePath(path);

    return filePath.filename().string();
}


// ============================================================
// GHI KẾT QUẢ JSON
// ============================================================

void writeJsonResult(
    ofstream& file,
    const string& programA,
    const string& programB,
    double cosine,
    double overlap,
    double token,
    int k,
    int prediction,
    double confidence,
    bool& firstResult)
{
    if (!firstResult)
    {
        file << ",\n";
    }

    firstResult = false;

    file << "        {\n";

    file << "            \"program_a\": \""
         << getFileName(programA)
         << "\",\n";

    file << "            \"program_b\": \""
         << getFileName(programB)
         << "\",\n";

    file << "            \"features\": {\n";

    file << fixed
         << setprecision(4);

    file << "                \"cosine_similarity\": "
         << cosine
         << ",\n";

    file << "                \"fingerprint_overlap\": "
         << overlap
         << ",\n";

    file << "                \"token_similarity\": "
         << token
         << "\n";

    file << "            },\n";

    file << "            \"knn\": {\n";

    file << "                \"k\": "
         << k
         << ",\n";

    file << "                \"label\": \"";

    if (prediction == 1)
    {
        file << "CO DAU HIEU DAO VAN";
    }
    else
    {
        file << "KHONG CO DAU HIEU DAO VAN";
    }

    file << "\",\n";

    file << "                \"confidence\": "
         << confidence * 100.0
         << "\n";

    file << "            }\n";

    file << "        }";
}


// ============================================================
// IN BẢNG FINGERPRINT
// ============================================================

void printFingerprintTable(
    const string& programName,
    const vector<Fingerprint>& fingerprints)
{
    cout << "\n";
    cout << programName << "\n";

    cout << left
         << setw(8) << "STT"
         << setw(25) << "Hash"
         << setw(12) << "Position"
         << endl;

    cout << "------------------------------------------------------------\n";

    for (size_t i = 0;
         i < fingerprints.size();
         i++)
    {
        cout << left
             << setw(8)
             << i + 1

             << setw(25)
             << fingerprints[i].hash

             << setw(12)
             << fingerprints[i].position

             << endl;
    }

    cout << "\nSo luong Fingerprint: "
         << fingerprints.size()
         << endl;
}


// ============================================================
// PHÂN TÍCH MỘT CẶP CHƯƠNG TRÌNH
// ============================================================

bool analyzePrograms(
    const string& fileA,
    const string& fileB,
    KNN& knn,
    ofstream& jsonFile,
    bool& firstResult)
{
    cout << "\n";
    cout << "======================================================================\n";

    cout << "SO SANH: "
         << getFileName(fileA)
         << " <--> "
         << getFileName(fileB)
         << endl;

    cout << "======================================================================\n";


    // ========================================================
    // ĐỌC CODE
    // ========================================================

    string codeA =
        readFile(fileA);

    string codeB =
        readFile(fileB);


    if (codeA.empty() || codeB.empty())
    {
        cout << "Khong the phan tich cap file nay.\n";

        return false;
    }


    // ========================================================
    // TOKENIZATION
    // ========================================================

    vector<string> tokensA =
        tokenize(codeA);

    vector<string> tokensB =
        tokenize(codeB);


    // ========================================================
    // TOKEN NORMALIZATION
    // ========================================================

    vector<string> normalizedA =
        normalizeTokens(tokensA);

    vector<string> normalizedB =
        normalizeTokens(tokensB);


    // ========================================================
    // FINGERPRINT
    // ========================================================

    vector<Fingerprint> fingerprintsA =
        generateFingerprints(
            codeA,
            5,
            4
        );

    vector<Fingerprint> fingerprintsB =
        generateFingerprints(
            codeB,
            5,
            4
        );


    // ========================================================
    // HIỂN THỊ FINGERPRINT
    // ========================================================

    cout << "\n";
    cout << "---------------- PROGRAM A ----------------\n";

    printFingerprintTable(
        getFileName(fileA),
        fingerprintsA
    );


    cout << "\n";
    cout << "---------------- PROGRAM B ----------------\n";

    printFingerprintTable(
        getFileName(fileB),
        fingerprintsB
    );


    // ========================================================
    // FEATURE EXTRACTION
    // ========================================================

    double cosine =
        cosineSimilarity(
            fingerprintsA,
            fingerprintsB
        );


    double overlap =
        fingerprintOverlap(
            fingerprintsA,
            fingerprintsB
        );


    double token =
        tokenSimilarity(
            normalizedA,
            normalizedB
        );


    // ========================================================
    // HIỂN THỊ FEATURE
    // ========================================================

    cout << "\n";
    cout << "========================================\n";
    cout << "         FEATURE EXTRACTION\n";
    cout << "========================================\n";

    cout << fixed
         << setprecision(4);

    cout << "Cosine Similarity      : "
         << cosine
         << endl;

    cout << "Fingerprint Overlap    : "
         << overlap
         << endl;

    cout << "Token Similarity       : "
         << token
         << endl;


    // ========================================================
    // FEATURE VECTOR
    // ========================================================

    FeatureVector feature;

    feature.cosine =
        cosine;

    feature.fingerprintOverlap =
        overlap;

    feature.tokenSimilarity =
        token;


    // ========================================================
    // KNN
    // ========================================================

    int prediction =
        knn.predict(
            feature
        );


    double confidence =
        knn.confidence(
            feature
        );


    // ========================================================
    // KẾT QUẢ
    // ========================================================

    cout << "\n";
    cout << "========================================\n";
    cout << "             AI - KNN\n";
    cout << "========================================\n";

    cout << "K = 3\n";

    cout << "Label: ";

    if (prediction == 1)
    {
        cout << "CO DAU HIEU DAO VAN\n";
    }
    else
    {
        cout << "KHONG CO DAU HIEU DAO VAN\n";
    }

    cout << "Confidence: "
         << fixed
         << setprecision(4)
         << confidence * 100.0
         << "%\n";


    // ========================================================
    // LƯU JSON
    // ========================================================

    writeJsonResult(
        jsonFile,
        fileA,
        fileB,
        cosine,
        overlap,
        token,
        3,
        prediction,
        confidence,
        firstResult
    );


    return true;
}


// ============================================================
// MAIN
// ============================================================

int main()
{
    cout << "======================================================================\n";

    cout << "              FINGERPRINT PLAGIARISM DETECTOR\n";

    cout << "======================================================================\n";


    // ========================================================
    // TÌM TẤT CẢ FILE CPP
    // ========================================================

    string dataFolder =
        "../data";


    vector<string> files;


    if (!fs::exists(dataFolder))
    {
        cerr << "Khong tim thay folder: "
             << dataFolder
             << endl;

        return 1;
    }


    for (const auto& entry :
         fs::directory_iterator(dataFolder))
    {
        if (!entry.is_regular_file())
            continue;


        fs::path path =
            entry.path();


        string extension =
            path.extension().string();


        // Chỉ lấy file .cpp
        if (extension == ".cpp")
        {
            files.push_back(
                path.string()
            );
        }
    }


    // ========================================================
    // KIỂM TRA SỐ LƯỢNG FILE
    // ========================================================

    if (files.size() < 2)
    {
        cerr << "Can it nhat 2 file .cpp de thuc hien so sanh.\n";

        return 1;
    }


    // ========================================================
    // SẮP XẾP TÊN FILE
    // ========================================================

    sort(
        files.begin(),
        files.end()
    );


    cout << "\n";
    cout << "So luong bai thi tim thay: "
         << files.size()
         << endl;


    cout << "\nDanh sach file:\n";


    for (size_t i = 0;
         i < files.size();
         i++)
    {
        cout << i + 1
             << ". "
             << getFileName(files[i])
             << endl;
    }


    // ========================================================
    // TẠO JSON
    // ========================================================

    ofstream jsonFile(
        "../data/results.json"
    );


    if (!jsonFile.is_open())
    {
        cerr << "\nKhong the tao file results.json\n";

        return 1;
    }


    jsonFile << "{\n";

    jsonFile << "    \"total_programs\": "
             << files.size()
             << ",\n";

    jsonFile << "    \"results\": [\n";


    bool firstResult = true;


    // ========================================================
    // KHỞI TẠO KNN
    // ========================================================

    KNN knn(3);


    // ========================================================
    // TRAINING DATA
    // ========================================================

    // --------------------------------------------------------
    // NHÓM CÓ DẤU HIỆU ĐẠO VĂN
    // --------------------------------------------------------

    knn.addSample(
        {
            1.00,
            1.00,
            1.00
        },
        1
    );


    knn.addSample(
        {
            0.85,
            0.75,
            0.92
        },
        1
    );


    knn.addSample(
        {
            0.75,
            0.65,
            0.88
        },
        1
    );


    knn.addSample(
        {
            0.65,
            0.55,
            0.80
        },
        1
    );


    // --------------------------------------------------------
    // NHÓM KHÔNG CÓ DẤU HIỆU ĐẠO VĂN
    // --------------------------------------------------------

    knn.addSample(
        {
            0.20,
            0.20,
            0.90
        },
        0
    );


    knn.addSample(
        {
            0.15,
            0.10,
            0.85
        },
        0
    );


    knn.addSample(
        {
            0.25,
            0.20,
            0.80
        },
        0
    );


    knn.addSample(
        {
            0.30,
            0.25,
            0.88
        },
        0
    );


    // ========================================================
    // IN DATASET
    // ========================================================

    knn.printTrainingData();


    // ========================================================
    // SO SÁNH TẤT CẢ CÁC CẶP
    // ========================================================

    int totalComparisons = 0;


    for (size_t i = 0;
         i < files.size();
         i++)
    {
        for (size_t j = i + 1;
             j < files.size();
             j++)
        {
            bool success =
                analyzePrograms(
                    files[i],
                    files[j],
                    knn,
                    jsonFile,
                    firstResult
                );


            if (success)
            {
                totalComparisons++;
            }
        }
    }


    // ========================================================
    // ĐÓNG JSON
    // ========================================================

    jsonFile << "\n";

    jsonFile << "    ]\n";

    jsonFile << "}\n";

    jsonFile.close();


    // ========================================================
    // HOÀN THÀNH
    // ========================================================

    cout << "\n";
    cout << "======================================================================\n";

    cout << "                         HOAN THANH\n";

    cout << "======================================================================\n";


    cout << "\nTong so bai thi: "
         << files.size()
         << endl;


    cout << "Tong so cap da so sanh: "
         << totalComparisons
         << endl;


    cout << "\nKet qua da duoc luu vao:\n";

    cout << "../data/results.json\n";


    return 0;
}