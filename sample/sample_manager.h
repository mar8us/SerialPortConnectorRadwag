#ifndef SAMPLE_MANAGER_H
#define SAMPLE_MANAGER_H

#include <QObject>
#include "sample.h"

class SampleManager : public QObject
{
    Q_OBJECT
public:
    explicit SampleManager(QObject *parent = nullptr);

    const QMap<QString, Sample> &getSamples() const;
    void setSamples(const QMap<QString, Sample> &newSamples);

    Sample getSample(const QString &number) const;
    bool sampleExists(const QString &number) const;
    bool addSample(const Sample &sample);
    bool updateSample(const Sample &sample);
    bool removeSample(const QString &number);

    bool reloadSamples();

signals:
    void samplesChanged();
    void sampleAdded(const QString &number);
    void sampleUpdated(const QString &number);
    void sampleRemoved(const QString &number);

private:
    QMap<QString, Sample> samples;

    SampleManager(const SampleManager&) = delete;
    SampleManager& operator=(const SampleManager&) = delete;

    void loadSamples();
    bool saveSamples();
    QString getSamplesFilePath() const;
};

#endif // SAMPLE_MANAGER_H
