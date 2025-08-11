#ifndef SAMPLE_MANAGER_H
#define SAMPLE_MANAGER_H

#include <QObject>
#include "sample.h"

class SampleManager : public QObject
{
    Q_OBJECT
public:
    explicit SampleManager(QObject *parent = nullptr);

    const QMap<QString, std::shared_ptr<const Sample>>& getSamples() const;
    std::shared_ptr<const Sample> getSample(const QString &name) const;
    bool addSample(std::shared_ptr<const Sample> sample);
    bool updateSample(const QString &oldName, std::shared_ptr<const Sample> newSample);
    bool removeSample(const QString &name);
    bool sampleExists(const QString &name) const;

    QStringList getSampleNames() const;
    int getSampleCount() const;
    void clear();

signals:
    void samplesChanged();
    void sampleAdded(const QString &name);
    void sampleUpdated(const QString &oldName, const QString &newName);
    void sampleRemoved(const QString &name);

private:
    QMap<QString, std::shared_ptr<const Sample>> samples;

    SampleManager(const SampleManager&) = delete;
    SampleManager& operator=(const SampleManager&) = delete;

    void loadSamples();
    bool saveSamples();
    QString getSamplesFilePath() const;
};

#endif // SAMPLE_MANAGER_H
