#include "ColoringController.h"

#include <vtkDataArray.h>
#include <vtkDataSetMapper.h>
#include <vtkPointData.h>
#include <vtkScalarBarActor.h>
#include <vtkUnstructuredGrid.h>

ColoringController::ColoringController(vtkDataSetMapper *mapper, vtkScalarBarActor *scalarBar)
    : mapper_(mapper), scalarBar_(scalarBar) {}

void ColoringController::clear() {
    if (mapper_ != nullptr) {
        mapper_->ScalarVisibilityOff();
    }
    if (scalarBar_ != nullptr) {
        scalarBar_->SetVisibility(0);
    }
}

bool ColoringController::apply(
    const VtuModelLoader &model,
    int arrayIndex,
    int component,
    QString *titleOut) {
    if (mapper_ == nullptr || scalarBar_ == nullptr || model.grid() == nullptr) {
        return false;
    }

    const auto &arrays = model.pointArrays();
    if (arrayIndex < 0 || arrayIndex >= static_cast<int>(arrays.size())) {
        return false;
    }

    vtkPointData *pointData = model.grid()->GetPointData();
    if (pointData == nullptr) {
        return false;
    }

    const std::string &arrayName = arrays[arrayIndex].name;
    vtkDataArray *arr = pointData->GetArray(arrayName.c_str());
    if (arr == nullptr) {
        return false;
    }

    mapper_->SetInputData(model.grid());
    mapper_->SetScalarModeToUsePointFieldData();
    mapper_->SetColorModeToMapScalars();
    mapper_->ScalarVisibilityOn();
    mapper_->ColorByArrayComponent(arrayName.c_str(), component);

    double range[2] = {0.0, 1.0};
    arr->GetRange(range, component);
    mapper_->SetScalarRange(range);

    QString componentText;
    if (component < 0) {
        componentText = "Magnitude";
    } else {
        componentText = QString("Component %1").arg(component);
    }

    const QString title = QString::fromStdString(arrayName) + "\n" + componentText;
    scalarBar_->SetLookupTable(mapper_->GetLookupTable());
    scalarBar_->SetTitle(title.toLocal8Bit().constData());
    scalarBar_->SetVisibility(1);

    if (titleOut != nullptr) {
        *titleOut = title;
    }

    return true;
}
