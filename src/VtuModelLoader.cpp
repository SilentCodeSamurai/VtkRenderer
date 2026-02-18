#include "VtuModelLoader.h"

#include <vtkDataArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>

bool VtuModelLoader::load(
    const QString &filePath,
    LoadedVtuModel *outModel,
    QString *errorMessage) {
    if (outModel == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = "Internal error: output model pointer is null.";
        }
        return false;
    }

    outModel->grid = nullptr;
    outModel->pointArrays.clear();

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

    outModel->grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    outModel->grid->ShallowCopy(output);

    vtkPointData *pointData = outModel->grid->GetPointData();
    if (pointData != nullptr) {
        for (int i = 0; i < pointData->GetNumberOfArrays(); ++i) {
            vtkDataArray *arr = pointData->GetArray(i);
            if (arr == nullptr || arr->GetName() == nullptr) {
                continue;
            }
            outModel->pointArrays.push_back({arr->GetName(), arr->GetNumberOfComponents()});
        }
    }

    return true;
}
