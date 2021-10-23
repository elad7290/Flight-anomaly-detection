

#ifndef SIMPLEANOMALYDETECTOR_H_
#define SIMPLEANOMALYDETECTOR_H_

#include "AnomalyDetector.h"
#include "minCircle.h"

struct correlatedFeatures{
    string feature1,feature2;  // names of the correlated features
	float corrlation;
	Line lin_reg;
	Point center_reg;
	float threshold;
	// you can add feilds
};


class SimpleAnomalyDetector:public TimeSeriesAnomalyDetector{
protected:
	vector<correlatedFeatures> cf;
    float m_threshold;
public:
	SimpleAnomalyDetector();
	virtual ~SimpleAnomalyDetector();

	virtual void learnNormal(const TimeSeries& ts);
	virtual vector<AnomalyReport> detect(const TimeSeries& ts);

	vector<correlatedFeatures> getNormalModel(){
		return cf;
	}
    // you can add helper methods
    virtual vector<pair<pair<string, string>,float>> FindCorllations(const TimeSeries &ts);
    virtual bool is_corrlate(float val);
    virtual void setDetectors(correlatedFeatures *correlatedFeatures, vector<Point *> points, int size);
    vector<Point *> featuresToPoints(int size, vector<float> feature1, vector<float> feature2);
    void freePoints(vector<Point *> points);
    virtual bool checkOffset(Point point, int index);
    float getThreshold(){
        return m_threshold;
    }
    void setThreshold(float new_threshold){
        m_threshold = new_threshold;
    }
};



#endif /* SIMPLEANOMALYDETECTOR_H_ */
