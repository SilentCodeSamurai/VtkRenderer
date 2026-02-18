#ifndef VTU_MODEL_LOADER_H
#define VTU_MODEL_LOADER_H

#include <QString>

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

#include <string>
#include <vector>

struct FieldArrayInfo {
    std::string name;
    int components = 0;
};

struct LoadedVtuModel {
    vtkSmartPointer<vtkUnstructuredGrid> grid;
    std::vector<FieldArrayInfo> pointArrays;
};

class VtuModelLoader {
public:
    static bool load(
        const QString &filePath,
        LoadedVtuModel *outModel,
        QString *errorMessage = nullptr);
};

#endif
