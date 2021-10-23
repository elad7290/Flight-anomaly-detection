#include "CLI.h"

CLI::CLI(DefaultIO *dio) {
    m_dio = dio;
    hybrid = HybridAnomalyDetector();
    // set commands array
    commands[0] = new UploadCommand(m_dio,&train,&test);
    commands[1] = new SetCommand(m_dio, &hybrid);
    commands[2] = new AnomalyDetectionCommand(m_dio, &anomalies, &hybrid, &train, &test);
    commands[3] = new DisplayResultsCommand(m_dio, &anomalies);
    commands[4] = new AnalyzeCommand(m_dio, &anomalies, &test);
    commands[5] = new ExitCommand(m_dio, &exit);
}

void CLI::start() {
    while(!exit) {
        string s = "Welcome to the Anomaly Detection Server.\nPlease choose an option:\n";
        int size = commands.size();
        for (int i = 0; i < size; i++){
            s += to_string(i+1) + "." + commands[i]->getDescription() + "\n";
        }
        m_dio->write(s);
        int choice = stoi(m_dio->read()) -1;
        commands[choice]->execute();
    }
}


CLI::~CLI() {
    int size = commands.size();
    for (int i = 0; i < size; i++){
        free(commands[i]);
    }
}

