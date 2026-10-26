#ifndef KNN_H
#define KNN_H

#include <vector>
#include <string>

using namespace std;

struct FeatureVector
{
    double cosine;
    double fingerprintOverlap;
    double tokenSimilarity;
};

struct TrainingSample
{
    FeatureVector feature;

    // 1 = có dấu hiệu đạo văn
    // 0 = không có dấu hiệu đạo văn
    int label;
};

class KNN
{
private:
    int k;

    vector<TrainingSample> trainingData;

    double distance(
        const FeatureVector& a,
        const FeatureVector& b
    );

public:
    KNN(int k = 3);

    void addSample(
        FeatureVector feature,
        int label
    );

    int predict(
        const FeatureVector& feature
    );

    double confidence(
        const FeatureVector& feature
    );

    void printTrainingData();
};

#endif