#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include "VtuModelLoader.h"

#include <QObject>
#include <QString>

class vtkUnstructuredGrid;

class ModelManager : public QObject {
    Q_OBJECT

public:
    explicit ModelManager(QObject *parent = nullptr);

    bool hasModel() const;
    vtkUnstructuredGrid *grid() const;
    const std::vector<FieldArrayInfo> &pointArrays() const;

signals:
    void modelLoaded();
    void modelCleared();
    void errorOccurred(const QString &message);

public slots:
    void loadFromPath(const QString &filePath);
    void clearModel();

private:
    LoadedVtuModel m_model;
};

#endif // MODEL_MANAGER_H

