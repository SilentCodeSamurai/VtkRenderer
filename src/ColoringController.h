#ifndef COLORING_CONTROLLER_H
#define COLORING_CONTROLLER_H

#include "VtuModelLoader.h"

#include <QString>

#include <vtkSmartPointer.h>

class vtkDataSetMapper;
class vtkScalarBarActor;

class ColoringController {
public:
    ColoringController(vtkDataSetMapper *mapper, vtkScalarBarActor *scalarBar);

    void clear();
    bool apply(const VtuModelLoader &model, int arrayIndex, int component, QString *titleOut = nullptr);

private:
    vtkSmartPointer<vtkDataSetMapper> mapper_;
    vtkSmartPointer<vtkScalarBarActor> scalarBar_;
};

#endif
