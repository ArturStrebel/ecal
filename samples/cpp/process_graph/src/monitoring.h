#ifndef MONITORING_H
#define MONITORING_H

#include <QObject>
#include <QTimer>
#include <ecal/ecal.h>

class Monitoring : public QObject
{
    Q_OBJECT

public:
    Monitoring();
    ~Monitoring();

    const eCAL::ProcessGraph::SProcessGraph& getProcessGraph() const;

public slots:
    void updateProcessGraph();

private:
    QTimer *timer; // for periodic updates.
    eCAL::Monitoring::SMonitoring monitoring;
    eCAL::ProcessGraph::SProcessGraph process_graph;
};

#endif // MONITORING_H