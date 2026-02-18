#include "ModelManager.h"

#include <vtkUnstructuredGrid.h>

ModelManager::ModelManager(QObject *parent)
    : QObject(parent) {}

void ModelManager::loadFromPath(const QString &filePath) {
    LoadedVtuModel loadedModel;
    QString errorText;
    if (!VtuModelLoader::load(filePath, &loadedModel, &errorText)) {
        emit errorOccurred(errorText);
        clearModel();
        return;
    }

    m_model = loadedModel;
    emit modelLoaded();
}

bool ModelManager::hasModel() const {
    return m_model.grid != nullptr;
}

vtkUnstructuredGrid *ModelManager::grid() const {
    return m_model.grid;
}

const std::vector<FieldArrayInfo> &ModelManager::pointArrays() const {
    return m_model.pointArrays;
}

void ModelManager::clearModel() {
    m_model.grid = nullptr;
    m_model.pointArrays.clear();
    emit modelCleared();
}

