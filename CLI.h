#ifndef CLI_H_
#define CLI_H_

#include "commands.h"

#define SIZE 6

using namespace std;

class CLI {
	DefaultIO* m_dio;
	// you can add data members
    vector<Command*> commands = vector<Command*>(SIZE);
	TimeSeries train;
	TimeSeries test;
	HybridAnomalyDetector hybrid;
    vector<AnomalyReport> anomalies;

	bool exit = false;
public:
	CLI(DefaultIO* d_dio);
	void start();
	virtual ~CLI();
};

#endif /* CLI_H_ */
