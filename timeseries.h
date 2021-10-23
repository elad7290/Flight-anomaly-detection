#ifndef TIMESERIES_H_
#define TIMESERIES_H_

#include "map"
#include "vector"

using namespace std;

class TimeSeries {
private:
    map<string, vector<float> > features;
public:
    TimeSeries(){};

    TimeSeries(const char *CSVfileName);

    TimeSeries(map<string, vector<float> > features_map);

    TimeSeries(const TimeSeries &ts);

    vector<string> GetFeaturesNames() const;

    vector<float> GetFeatureSamples(string s) const;

    int NumberOfFeatures() const;

    int NumberOfSampales(string s) const;

};

#endif /* TIMESERIES_H_ */