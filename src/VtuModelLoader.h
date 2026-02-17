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

class VtuModelLoader {
public:
    bool load(const QString &filePath, QString *errorMessage = nullptr);
    void clear();

    vtkUnstructuredGrid *grid() const { return grid_; }
    const std::vector<FieldArrayInfo> &pointArrays() const { return pointArrays_; }

private:
    vtkSmartPointer<vtkUnstructuredGrid> grid_;
    std::vector<FieldArrayInfo> pointArrays_;
};

#endif
