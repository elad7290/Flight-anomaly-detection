
#include "HybridAnomalyDetector.h"
#include <cmath>

#define ACCURATE 1.1
#define MIN_THRESHOLD 0.5

HybridAnomalyDetector::HybridAnomalyDetector() {
}

HybridAnomalyDetector::~HybridAnomalyDetector() {
}

/**     HELPERS     */

bool HybridAnomalyDetector::is_corrlate(float val) {
    if (val < m_threshold && val > MIN_THRESHOLD) {
        return true;
    }
    return false;
}

void HybridAnomalyDetector::setDetectors(correlatedFeatures *correlatedFeatures, vector<Point *> points, int size) {
    Circle min_circle = findMinCircle(&points[0], size);
    correlatedFeatures->center_reg = min_circle.center;
    correlatedFeatures->threshold = ACCURATE * min_circle.radius;
}

bool HybridAnomalyDetector::checkOffset(Point point, int index) {
    float offset = distance(point, cf[index].center_reg);
    if (offset > cf[index].threshold){
        return true;
    }
    return false;
}

/*******************/

vector<pair<pair<string, string>, float>> HybridAnomalyDetector::FindCorllations(const TimeSeries &ts) {
    vector<pair<pair<string, string>, float>> corllations;
    vector<string> features = ts.GetFeaturesNames();
    float max;
    string max_name;
    for (int i = 0; i < ts.NumberOfFeatures(); i++) {
        max = 0;
        for (int j = i + 1; j < ts.NumberOfFeatures(); j++) {
            float val = pearson(&ts.GetFeatureSamples(features[i])[0], &ts.GetFeatureSamples(features[j])[0],
                                ts.NumberOfSampales(features[i]));
            if (max < val) {
                max = val;
                max_name = features[j];
            }
        }
        if (SimpleAnomalyDetector::is_corrlate(max)) {
            corllations.push_back(make_pair(make_pair(features[i], max_name), max));
        }
        if (HybridAnomalyDetector::is_corrlate(max)) {
            corllations.push_back(make_pair(make_pair(features[i], max_name), max));
        }
    }
    return corllations;
}

void HybridAnomalyDetector::learnNormal(const TimeSeries &ts) {
    vector<pair<pair<string, string>, float>> corllations_names = FindCorllations(ts);
    // string feature1,feature2
    for (pair<pair<string, string>, float> corl_pair : corllations_names) {

        // correlatedFeatures
        correlatedFeatures correlatedFeatures;
        correlatedFeatures.feature1 = corl_pair.first.first;
        correlatedFeatures.feature2 = corl_pair.first.second;

        // number of samples
        int size = ts.NumberOfSampales(corl_pair.first.first);
        // get features as points
        vector<Point *> points = featuresToPoints(size, ts.GetFeatureSamples(corl_pair.first.first),
                                                  ts.GetFeatureSamples(corl_pair.first.second));

        // set corrlation
        correlatedFeatures.corrlation = pearson(&ts.GetFeatureSamples(corl_pair.first.first)[0],
                                                &ts.GetFeatureSamples(corl_pair.first.second)[0], size);

        // set lin_reg and threshold
        if (SimpleAnomalyDetector::is_corrlate(corl_pair.second)) {
            SimpleAnomalyDetector::setDetectors(&correlatedFeatures, points, size);
        }
        if (HybridAnomalyDetector::is_corrlate(corl_pair.second)) {
            HybridAnomalyDetector::setDetectors(&correlatedFeatures, points, size);
        }

        // add correlatedFeatures
        cf.push_back(correlatedFeatures);

        // free points
        freePoints(points);
    }
}

vector<AnomalyReport> HybridAnomalyDetector::detect(const TimeSeries &ts) {
    vector<AnomalyReport> anomaly;
    for (int i = 0; i < cf.size(); i++) {
        // create points
        vector<Point *> points = featuresToPoints(ts.NumberOfSampales(cf[i].feature1),
                                                  ts.GetFeatureSamples(cf[i].feature1),
                                                  ts.GetFeatureSamples(cf[i].feature2));
        for (int j = 0; j < points.size(); j++) {
            // check offset
            if(abs(cf[i].corrlation) >= m_threshold) {
                if (SimpleAnomalyDetector::checkOffset(*points[j],i)){
                    // report
                    string description = cf[i].feature1 + "-" + cf[i].feature2;
                    long time_step = j + 1;
                    anomaly.push_back(AnomalyReport(description, time_step));
                }
            } else {
                if (HybridAnomalyDetector::checkOffset(*points[j],i)){
                    // report
                    string description = cf[i].feature1 + "-" + cf[i].feature2;
                    long time_step = j + 1;
                    anomaly.push_back(AnomalyReport(description, time_step));
                }
            }
        }
        freePoints(points);
    }
    return anomaly;
}

