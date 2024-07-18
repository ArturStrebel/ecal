#pragma once

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

signals:
    void updateTopicTree();

private:
    bool topicTreeHasChanged();
    QTimer *timer;
    eCAL::Monitoring::SMonitoring monitoring;
    eCAL::ProcessGraph::SProcessGraph process_graph; 
    std::vector<eCAL::ProcessGraph::STopicTreeItem> previousTopicTree;
};