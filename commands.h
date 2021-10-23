#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "HybridAnomalyDetector.h"
#include <cstring>
#include <cmath>
#include <sys/socket.h>
#include <sstream>

using namespace std;

class DefaultIO {
public:
    virtual string read() = 0;

    virtual void write(string text) = 0;

    virtual void write(float f) = 0;

    virtual void read(float *f) = 0;

    virtual ~DefaultIO() {}

    // you may add additional methods here
};

class StandardIO : public DefaultIO {
public:
    StandardIO() {

    }

    string read() override {
        string s;
        cin >> s;
        return s;
    }

    void write(string text) override {
        cout << text;
    }

    void write(float f) override {
        cout << f;
    }

    void read(float *f) override {
        cin >> *f;
    }

    ~StandardIO() {}
};

class SocketIO : public DefaultIO {
private:
    int m_fd;
public:
    SocketIO(int fd) {
        m_fd = fd;
    }

    string read() override {
        string buffer = "";
        char dummy = 0;
        //bzero(buffer, MSS);
        recv(m_fd, &dummy, sizeof(char), 0);
        while (dummy != '\n') {
            buffer += dummy;
            recv(m_fd, &dummy, sizeof(char), 0);
        }
        return buffer;
    }

    void read(float *f) override {
        string text = "";
        char dummy = 0;
        recv(m_fd, &dummy, sizeof(char), 0);
        while (dummy != '\n') {
            text += dummy;
            recv(m_fd, &dummy, sizeof(char), 0);
        }
        stringstream s(text);
        s >> *f;
    }

    void write(string text) override {
        send(m_fd, &text[0], strlen(&text[0]), 0);
    }

    void write(float f) override {
        ostringstream text;
        text << f;
        send(m_fd, text.str().c_str(), text.str().length(), 0);
    }


};


// you may edit this class
class Command {
protected:
    string description; // menu
    DefaultIO *dio;
public:
    Command(DefaultIO *dio) : dio(dio) {}

    virtual ~Command() {}

    virtual void execute() = 0;

    /******HELPERS******/

    string getDescription() {
        return description;
    }

    float precision(float val, int n) {
        val = val * pow(10,n);
        val = floor(val);
        val = val / pow(10,n);
        return val;
    }

    /******END HELPERS**/
};

// implement here your command classes
class UploadCommand : public Command {
private:
    TimeSeries *m_train;
    TimeSeries *m_test;
public:
    UploadCommand(DefaultIO *dio, TimeSeries *train, TimeSeries *test) : Command(dio) {
        description = "upload a time series csv file";
        m_train = train;
        m_test = test;
    }

    /******HELPERS******/

    map<string, vector<float>> readFromIO(DefaultIO *dio) {
        map<string, vector<float>> features;
        // find keys
        string str = dio->read();
        while (!str.empty()) {
            // parameter of feature
            string token = str.substr(0, str.find(","));
            str.replace(str.find(token), token.length() + 1, "");
            features[token] = vector<float>(0);
        }
        // find values
        str = dio->read();
        while (strcmp(&str[0], "done") != 0) {
            for (auto pair : features) {
                string token = str.substr(0, str.find(","));
                str.replace(str.find(token), token.length() + 1, "");
                features[pair.first].push_back(stof(token));
            }
            str = dio->read();
        }
        return features;
    }

    /******END HELPERS**/

    void execute() override {
        dio->write("Please upload your local train CSV file.\n");
        // making TimeSeries train
        *m_train = TimeSeries(readFromIO(dio));
        dio->write("Upload complete.\n");
        dio->write("Please upload your local test CSV file.\n");
        // making TimeSeries test
        *m_test = TimeSeries(readFromIO(dio));
        dio->write("Upload complete.\n");
    }
};

class SetCommand : public Command {
private:
    HybridAnomalyDetector *m_hybrid;
public:
    SetCommand(DefaultIO *dio, HybridAnomalyDetector *hybrid) : Command(dio) {
        description = "algorithm settings";
        m_hybrid = hybrid;
    }

    void execute() override {
        // write
        dio->write("The current correlation threshold is ");
        dio->write(precision(m_hybrid->getThreshold(),1));
        dio->write("\n");
        // end write
        bool flag = true;
        // excepting a valid new threshold
        while (flag) {
            dio->write("Type a new threshold\n");
            float new_threshold;
            dio->read(&new_threshold);
            if (new_threshold >= 0 && new_threshold <= 1) {
                m_hybrid->setThreshold(new_threshold);
                flag = false;
            } else {
                dio->write("please choose a value between 0 and 1.");
            }
        }
    }
};

class AnomalyDetectionCommand : public Command {
private:
    vector<AnomalyReport> *m_anomalies;
    HybridAnomalyDetector *m_hybrid;
    TimeSeries *m_train;
    TimeSeries *m_test;
public:
    AnomalyDetectionCommand(DefaultIO *dio, vector<AnomalyReport> *anomalies, HybridAnomalyDetector *hybrid,
                            TimeSeries *train, TimeSeries *test) : Command(dio) {
        description = "detect anomalies";
        m_anomalies = anomalies;
        m_hybrid = hybrid;
        m_train = train;
        m_test = test;
    }

    void execute() override {
        m_hybrid->learnNormal(*m_train);
        *m_anomalies = m_hybrid->detect(*m_test);
        dio->write("anomaly detection complete.\n");
    }
};

class DisplayResultsCommand : public Command {
private:
    vector<AnomalyReport> *m_anomalies;
public:
    DisplayResultsCommand(DefaultIO *dio, vector<AnomalyReport> *anomalies) : Command(dio) {
        description = "display results";
        m_anomalies = anomalies;
    }

    void execute() override {
        string s;
        for (AnomalyReport a: *m_anomalies) {
            s = to_string(a.timeStep) + "\t" + a.description + "\n";
            dio->write(s);
        }
        dio->write("Done.\n");
    }
};

class AnalyzeCommand : public Command {
private:
    vector<AnomalyReport> *m_anomalies;
    TimeSeries *m_ts;
public:
    AnalyzeCommand(DefaultIO *dio, vector<AnomalyReport> *anomalies ,TimeSeries *ts) : Command(dio) {
        description = "upload anomalies and analyze results";
        m_anomalies = anomalies;
        m_ts = ts;
    }

    /******HELPERS******/

    vector<pair<long, long>> to_serial(vector<long> anomalies) {
        vector<pair<long, long>> serial;
        int size = anomalies.size();
        // check anomalies vector not empty
        if (size == 0) {
            return serial;
        }
        // get first pair
        serial.push_back(make_pair(anomalies[0], anomalies[0]));
        for (int i = 1; i < size; i++) {
            int last = serial.size() - 1;
            if (anomalies[i] == serial[last].second + 1) {
                serial[last].second = anomalies[i];
            } else {
                serial.push_back(make_pair(anomalies[i], anomalies[i]));
            }
        }
        return serial;
    }

    vector<pair<long, long>> find_serial_anomalies(DefaultIO *dio) {
        vector<pair<long, long>> serial;
        string str = dio->read();
        while (strcmp(&str[0], "done") != 0) {
            // separate to 2 edges
            string token1 = str.substr(0, str.find(","));
            str.replace(str.find(token1), token1.length() + 1, "");
            string token2 = str.substr(0, str.find(","));
            // insert edges
            serial.push_back(make_pair(stol(token1), stol(token2)));
            //next
            str = dio->read();
        }
        return serial;
    }

    vector<pair<long, long>> find_serial_anomalies(vector<AnomalyReport> *anomalies) {
        map<string, vector<long>> anomalies_separate;
        // anomalies separated to pairs and their anomalies
        for (AnomalyReport anomaly: *anomalies) {
            anomalies_separate[anomaly.description].push_back(anomaly.timeStep);
        }
        // serial shown anomalies
        map<string, vector<pair<long, long>>> anomalies_serial;
        for (auto pair: anomalies_separate) {
            anomalies_serial[pair.first] = to_serial(pair.second);
        }
        // unite all ranges
        vector<pair<long, long>> united;
        for (auto pair: anomalies_serial) {
            united.insert(united.end(), pair.second.begin(), pair.second.end());
        }
        return united;
    }

    vector<pair<long, long>> find_cut(vector<pair<long, long>> given_anomalies_serial, vector<pair<long, long>> my_anomalies_serial) {
        int given_size = given_anomalies_serial.size();
        int my_size = my_anomalies_serial.size();
        vector<pair<long, long>> cuts;
        for (int i = 0; i < given_size; ++i) {
            for (int j = 0; j < my_size; ++j) {
                // if there is a cut
                if ((my_anomalies_serial[j].first >= given_anomalies_serial[i].first &&
                     my_anomalies_serial[j].first <= given_anomalies_serial[i].second) ||
                    (my_anomalies_serial[j].second >= given_anomalies_serial[i].first &&
                     my_anomalies_serial[j].second <= given_anomalies_serial[i].second)) {
                    cuts.push_back(my_anomalies_serial[j]);
                    continue;
                }
                if ((given_anomalies_serial[i].first >= my_anomalies_serial[j].first &&
                     given_anomalies_serial[i].first <= my_anomalies_serial[j].second) ||
                    given_anomalies_serial[i].second >= my_anomalies_serial[j].first &&
                    given_anomalies_serial[i].second <= my_anomalies_serial[j].second) {
                    cuts.push_back(my_anomalies_serial[j]);
                    continue;
                }
            }
        }
        return cuts;
    }

    long number_of_time_steps(vector<pair<long, long>> serial) {
        long sum = 0;
        for(auto pair: serial) {
            sum+= (pair.second - pair.first) + 1;
        }
        return sum;
    }

    /******END HELPERS**/

    void execute() override {
        // if there no samples
        if (m_ts->GetFeaturesNames().size() == 0){
            return;
        }

        dio->write("Please upload your local anomalies file.\n");
        // their serial shown anomalies
        vector<pair<long, long>> given_anomalies_serial = find_serial_anomalies(dio);
        dio->write("Upload complete.\n");
        // my serial shown anomalies
        vector<pair<long, long>> my_anomalies_serial = find_serial_anomalies(m_anomalies);
        // find cuts
        vector<pair<long, long>> cuts = find_cut(given_anomalies_serial, my_anomalies_serial);

        float TP = cuts.size();
        float P = given_anomalies_serial.size();
        // write
        dio->write("True Positive Rate: ");
        dio->write(precision(TP/P, 3));
        dio->write("\n");

        float n = m_ts->NumberOfSampales(m_ts->GetFeaturesNames()[0]);
        float N = n - number_of_time_steps(given_anomalies_serial);
        float FP = my_anomalies_serial.size() - cuts.size();
        // write
        dio->write("False Positive Rate: ");
        dio->write(precision(FP/N, 3));
        dio->write("\n");
    }
};

class ExitCommand : public Command {
private:
    bool *m_exit;
public:
    ExitCommand(DefaultIO *dio, bool *exit) : Command(dio) {
        description = "exit";
        m_exit = exit;
    }

    void execute() override {
        *m_exit = true;
    }
};

#endif /* COMMANDS_H_ */
