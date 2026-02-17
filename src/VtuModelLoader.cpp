#include "VtuModelLoader.h"

#include <vtkDataArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>

bool VtuModelLoader::load(const QString &filePath, QString *errorMessage) {
    clear();

    vtkNew<vtkXMLUnstructuredGridReader> reader;
    reader->SetFileName(filePath.toStdString().c_str());
    reader->Update();

    vtkUnstructuredGrid *output = reader->GetOutput();
    if (output == nullptr || output->GetNumberOfPoints() == 0) {
        if (errorMessage != nullptr) {
            *errorMessage = "Failed to read VTU file:\n" + filePath;
        }
        return false;
    }

    grid_ = vtkSmartPointer<vtkUnstructuredGrid>::New();
    grid_->ShallowCopy(output);

    vtkPointData *pointData = grid_->GetPointData();
    if (pointData != nullptr) {
        for (int i = 0; i < pointData->GetNumberOfArrays(); ++i) {
            vtkDataArray *arr = pointData->GetArray(i);
            if (arr == nullptr || arr->GetName() == nullptr) {
                continue;
            }
            pointArrays_.push_back({arr->GetName(), arr->GetNumberOfComponents()});
        }
    }

    return true;
}

void VtuModelLoader::clear() {
    grid_ = nullptr;
    pointArrays_.clear();
}
