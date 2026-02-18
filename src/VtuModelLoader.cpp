#include "VtuModelLoader.h"

#include <QFile>
#include <QMap>
#include <QXmlStreamReader>

#include <vtkDataArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>

VtuModelLoader::VtuModelLoader(QObject *parent) : QObject(parent) {}

void VtuModelLoader::load(const QString &filePath) {
  LoadedVtuModel outModel;

  vtkNew<vtkXMLUnstructuredGridReader> reader;
  reader->SetFileName(filePath.toStdString().c_str());
  reader->Update();

  vtkUnstructuredGrid *output = reader->GetOutput();
  if (output == nullptr) {
    emit modelLoadingErrorOccured("Failed to read VTU file (no output):\n" +
                                  filePath);
    return;
  }

  const vtkIdType numPoints = output->GetNumberOfPoints();
  if (numPoints == 0) {
    emit modelLoadingErrorOccured("Failed to read VTU file (no points):\n" +
                                  filePath);
    return;
  }

  outModel.grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  outModel.grid->ShallowCopy(output);

  // Parse XML to extract component names directly
  QMap<QString, QVector<QString>> arrayComponentNames;
  QFile file(filePath);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
      QXmlStreamReader::TokenType token = xml.readNext();
      if (token == QXmlStreamReader::StartElement && xml.name() == QString("DataArray")) {
        QXmlStreamAttributes attrs = xml.attributes();
        QString arrayName = attrs.value("Name").toString();
        int numComponents = attrs.value("NumberOfComponents").toInt();
        
        if (!arrayName.isEmpty() && numComponents > 0) {
          QVector<QString> componentNames;
          // Extract ComponentName0, ComponentName1, etc.
          for (int i = 0; i < numComponents; ++i) {
            QString attrName = QString("ComponentName%1").arg(i);
            QString componentName = attrs.value(attrName).toString();
            if (!componentName.isEmpty()) {
              componentNames.push_back(componentName);
            }
          }
          if (!componentNames.isEmpty()) {
            arrayComponentNames[arrayName] = componentNames;
          }
        }
      }
    }
    file.close();
  }

  vtkPointData *pointData = outModel.grid->GetPointData();
  if (pointData != nullptr) {
    const int numArrays = pointData->GetNumberOfArrays();
    for (int i = 0; i < numArrays; ++i) {
      vtkDataArray *arr = pointData->GetArray(i);
      if (arr == nullptr || arr->GetName() == nullptr) {
        continue;
      }
      int numberOfComponents = arr->GetNumberOfComponents();
      QString arrayName = QString::fromStdString(arr->GetName());

      QVector<QString> componentNames;
      
      // For multi-component arrays, add "Magnitude" as first option (maps to componentIndex -1)
      if (numberOfComponents > 1) {
        componentNames.push_back("Magnitude");
      }
      
      // Use component names from XML if available, otherwise try VTK, otherwise default
      QVector<QString> xmlComponentNames = arrayComponentNames.value(arrayName);
      for (int j = 0; j < numberOfComponents; ++j) {
        QString componentName;
        if (j < xmlComponentNames.size()) {
          // Use name from XML
          componentName = xmlComponentNames[j];
        } else {
          // Try VTK's GetComponentName
          const char *vtkComponentName = arr->GetComponentName(j);
          if (vtkComponentName != nullptr) {
            componentName = QString::fromStdString(vtkComponentName);
          } else {
            // Default naming
            if (numberOfComponents == 1) {
              componentName = "Magnitude";
            } else {
              componentName = QString("Component %1").arg(j);
            }
          }
        }
        componentNames.push_back(componentName);
      }
      outModel.pointArrays.push_back(
          NestedArrayInfo(arrayName, QVector<QString>::fromVector(componentNames)));
    }
    outModel.fileName = file.fileName();
    outModel.filePath = filePath;
  }

  emit modelLoaded(outModel);
}

// Helper functions for component index mapping and name retrieval
int VtuModelLoader::comboIndexToVtkIndex(const NestedArrayInfo &array, int comboIndex) {
  if (hasMagnitudeOption(array)) {
    if (comboIndex == 0) {
      return -1; // Magnitude
    } else {
      return comboIndex - 1; // Adjust for Magnitude offset
    }
  } else {
    return (comboIndex >= 0) ? comboIndex : 0;
  }
}

int VtuModelLoader::vtkIndexToComboIndex(const NestedArrayInfo &array, int vtkIndex) {
  if (hasMagnitudeOption(array)) {
    if (vtkIndex == -1) {
      return 0; // Magnitude
    } else if (vtkIndex >= 0) {
      return vtkIndex + 1; // Adjust for Magnitude offset
    }
  } else {
    return (vtkIndex >= 0) ? vtkIndex : 0;
  }
  return 0;
}

QString VtuModelLoader::getDisplayNameForVtkIndex(const NestedArrayInfo &array, int vtkIndex) {
  if (vtkIndex == -1) {
    return "Magnitude";
  }
  
  int nameIndex = vtkIndex;
  if (hasMagnitudeOption(array)) {
    // Magnitude is at index 0, actual components start at index 1
    nameIndex = vtkIndex + 1;
  }
  
  if (nameIndex >= 0 && nameIndex < array.componentNames.size()) {
    return array.componentNames[nameIndex];
  }
  
  return QString("Component %1").arg(vtkIndex);
}

bool VtuModelLoader::hasMagnitudeOption(const NestedArrayInfo &array) {
  return array.componentNames.size() > 1 && array.componentNames[0] == "Magnitude";
}
