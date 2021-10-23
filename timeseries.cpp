#include "timeseries.h"
#include "fstream"

map<string, vector<float>> readFromFile(const char *CSVfileName) {
    map<string, vector<float>> features_array;
    ifstream file;
    file.open(CSVfileName);
    string str;
    // find keys
    file >> str;
    while (!str.empty()) {
        // parameter of feature
        string token = str.substr(0, str.find(","));
        str.replace(str.find(token), token.length() + 1, "");
        features_array[token] = vector<float>(0);
    }
    // find values
    while (file >> str) {
        for (auto pair : features_array) {
            string token = str.substr(0, str.find(","));
            str.replace(str.find(token), token.length() + 1, "");
            features_array[pair.first].push_back(stof(token));
        }
    }
    //close file
    file.close();
    return features_array;
}

TimeSeries::TimeSeries(const char *CSVfileName) {
   features = readFromFile(CSVfileName);
}

TimeSeries::TimeSeries(map<string, vector<float>> features_map) {
    features = features_map;
}

vector<string> TimeSeries::GetFeaturesNames() const {
    vector<string> names(0);
    for (auto pair : features)
        names.push_back(pair.first);
    return names;
}

vector<float> TimeSeries::GetFeatureSamples(string s) const {
    return features.at(s);
}

int TimeSeries::NumberOfFeatures() const {
    return features.size();
}

int TimeSeries::NumberOfSampales(string s) const {
    return features.at(s).size();
}

TimeSeries::TimeSeries(const TimeSeries &ts) {
    features = ts.features;
}




