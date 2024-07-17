#ifndef MONITORING_H
#define MONITORING_H

#include <QObject>
#include <QTimer>
#include <ecal/ecal.h>

class Monitoring : public QObject // Erben von QObject
{
    Q_OBJECT

public:
    Monitoring();
    ~Monitoring();

    // Getter-Methode für ProcessGraph.
    const eCAL::ProcessGraph::SProcessGraph& getProcessGraph() const; // Nicht nur SProcessGraph 

public slots: // Hinzufügen von slots
    void updateProcessGraph(); // Slot, um die Prozessinformationen zu aktualisieren.

private:
    QTimer *timer;           // Timer für periodische Updates.
    eCAL::Monitoring::SMonitoring monitoring;
    eCAL::ProcessGraph::SProcessGraph process_graph; // process_graph hinzufügen
    int counter = 0;
};

#endif // MONITORING_H