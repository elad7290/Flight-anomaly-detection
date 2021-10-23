#include "SimpleAnomalyDetector.h"

#define ACCURATE 1.1

SimpleAnomalyDetector::SimpleAnomalyDetector() {
    cf = vector<correlatedFeatures>(0);
    m_threshold = 0.9;
}

SimpleAnomalyDetector::~SimpleAnomalyDetector() {

}

/** HELPERS **/

// return max deviation of reg line
float maxOffset(vector<Point *> points, int size, Line reg) {
    float max = 0;
    for (int i = 0; i < size; i++) {
        float offset = dev(*points[i], reg);
        if (max < offset)
            max = offset;
    }
    return max;
}

// creating points of 2 features
vector<Point *> SimpleAnomalyDetector::featuresToPoints(int size, vector<float> feature1, vector<float> feature2) {
    vector<Point *> points;
    for (int i = 0; i < size; i++) {
        float x = feature1.at(i);
        float y = feature2.at(i);
        Point *p = new Point(x, y);
        points.push_back(p);
    }
    return points;
}

void SimpleAnomalyDetector::freePoints(vector<Point *> points) {
    for (int i = 0; i < points.size(); ++i) {
        delete (points[i]);
    }
}

// finds pairs of corllative features
vector<pair<pair<string, string>, float>> SimpleAnomalyDetector::FindCorllations(const TimeSeries &ts) {
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
        if (is_corrlate(max)) {
            corllations.push_back(make_pair(make_pair(features[i], max_name), max));
        }
    }
    return corllations;
}

bool SimpleAnomalyDetector::is_corrlate(float val) {
    if (val >= m_threshold) {
        return true;
    }
    return false;
}

void SimpleAnomalyDetector::setDetectors(correlatedFeatures *correlatedFeatures, vector<Point *> points, int size) {
    Line lin_reg = linear_reg(&points[0], size);
    correlatedFeatures->lin_reg = lin_reg;
    correlatedFeatures->threshold = ACCURATE * maxOffset(points, size, lin_reg);
}

bool SimpleAnomalyDetector::checkOffset(Point point, int index) {
    float offset = dev(point, cf[index].lin_reg);
    if (offset > cf[index].threshold){
        return true;
    }
    return false;
}

/*******************/

void SimpleAnomalyDetector::learnNormal(const TimeSeries &ts) {
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
        setDetectors(&correlatedFeatures, points, size);

        // add correlatedFeatures
        cf.push_back(correlatedFeatures);

        // free points
        freePoints(points);
    }
}

vector<AnomalyReport> SimpleAnomalyDetector::detect(const TimeSeries &ts) {
    vector<AnomalyReport> anomaly;
    for (int i = 0; i < cf.size(); i++) {
        // create points
        vector<Point *> points = featuresToPoints(ts.NumberOfSampales(cf[i].feature1),
                                                  ts.GetFeatureSamples(cf[i].feature1),
                                                  ts.GetFeatureSamples(cf[i].feature2));
        for (int j = 0; j < points.size(); j++) {
            // check offset
            if (checkOffset(*points[j],i)) {
                // report
                string description = cf[i].feature1 + "-" + cf[i].feature2;
                long time_step = j + 1;

                anomaly.push_back(AnomalyReport(description, time_step));
            }
        }
        freePoints(points);
    }
    return anomaly;
}



