#include "KNN.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <iomanip>
using namespace std;


// ============================================================
// CONSTRUCTOR
// ============================================================

KNN::KNN(int k)
{
    this->k = k;
}


// ============================================================
// THÊM DỮ LIỆU HUẤN LUYỆN
// ============================================================

void KNN::addSample(
    FeatureVector feature,
    int label)
{
    trainingData.push_back(
        {
            feature,
            label
        }
    );
}


// ============================================================
// TÍNH KHOẢNG CÁCH EUCLIDEAN
// ============================================================

double KNN::distance(
    const FeatureVector& a,
    const FeatureVector& b)
{
    double d1 =
        a.cosine -
        b.cosine;

    double d2 =
        a.fingerprintOverlap -
        b.fingerprintOverlap;

    double d3 =
        a.tokenSimilarity -
        b.tokenSimilarity;

    return sqrt(
        d1 * d1 +
        d2 * d2 +
        d3 * d3
    );
}


// ============================================================
// DỰ ĐOÁN
// ============================================================

int KNN::predict(
    const FeatureVector& feature)
{
    if (trainingData.empty())
        return 0;

    struct Neighbor
    {
        double distance;
        int label;
    };

    vector<Neighbor> neighbors;

    for (const auto& sample : trainingData)
    {
        neighbors.push_back(
            {
                distance(
                    feature,
                    sample.feature
                ),
                sample.label
            }
        );
    }


    // Sắp xếp từ gần nhất đến xa nhất
    sort(
        neighbors.begin(),
        neighbors.end(),
        [](const Neighbor& a,
           const Neighbor& b)
        {
            return a.distance <
                   b.distance;
        }
    );


    int actualK =
        min(
            k,
            static_cast<int>(
                neighbors.size()
            )
        );


    if (actualK == 0)
        return 0;


    int plagiarismVotes = 0;
    int normalVotes = 0;


    // Lấy K hàng xóm gần nhất
    for (int i = 0; i < actualK; i++)
    {
        if (neighbors[i].label == 1)
            plagiarismVotes++;
        else
            normalVotes++;
    }


    // ========================================================
    // TÍNH CONFIDENCE
    // ========================================================

    int maxVotes =
        max(
            plagiarismVotes,
            normalVotes
        );

    double confidenceValue =
        static_cast<double>(maxVotes) /
        actualK;


    // ========================================================
    // NGƯỠNG 0.75
    // ========================================================
    //
    // Nếu confidence >= 0.75
    // thì kết quả theo nhóm có nhiều phiếu hơn.
    //
    // Nếu confidence < 0.75
    // thì không đủ độ tin cậy -> KHÔNG ĐẠO VĂN
    // ========================================================

    if (confidenceValue >= 0.75)
    {
        if (plagiarismVotes > normalVotes)
            return 1;

        return 0;
    }


    return 0;
}


// ============================================================
// ĐỘ TIN CẬY
// ============================================================

double KNN::confidence(
    const FeatureVector& feature)
{
    if (trainingData.empty())
        return 0.0;


    struct Neighbor
    {
        double distance;
        int label;
    };


    vector<Neighbor> neighbors;


    for (const auto& sample : trainingData)
    {
        neighbors.push_back(
            {
                distance(
                    feature,
                    sample.feature
                ),
                sample.label
            }
        );
    }


    sort(
        neighbors.begin(),
        neighbors.end(),
        [](const Neighbor& a,
           const Neighbor& b)
        {
            return a.distance <
                   b.distance;
        }
    );


    int actualK =
        min(
            k,
            static_cast<int>(
                neighbors.size()
            )
        );


    if (actualK == 0)
        return 0.0;


    int plagiarismVotes = 0;
    int normalVotes = 0;


    for (int i = 0; i < actualK; i++)
    {
        if (neighbors[i].label == 1)
            plagiarismVotes++;
        else
            normalVotes++;
    }


    int maxVotes =
        max(
            plagiarismVotes,
            normalVotes
        );


    return
        static_cast<double>(maxVotes) /
        actualK;
}


// ============================================================
// IN DATASET
// ============================================================

void KNN::printTrainingData()
{
    cout << "\n";

    cout << "============================================================\n";
    cout << "                 KNN TRAINING DATA\n";
    cout << "============================================================\n";

    cout << left
         << setw(5) << "STT"
         << setw(12) << "Cosine"
         << setw(12) << "Overlap"
         << setw(12) << "Token"
         << setw(20) << "Label"
         << endl;

    cout << "------------------------------------------------------------\n";


    for (size_t i = 0;
         i < trainingData.size();
         i++)
    {
        cout << left
             << setw(5)
             << i + 1

             << setw(12)
             << fixed
             << setprecision(4)
             << trainingData[i].feature.cosine

             << setw(12)
             << trainingData[i].feature.fingerprintOverlap

             << setw(12)
             << trainingData[i].feature.tokenSimilarity

             << setw(20);


        if (trainingData[i].label == 1)
            cout << "DAO_VAN";
        else
            cout << "KHONG_DAO_VAN";


        cout << endl;
    }
}