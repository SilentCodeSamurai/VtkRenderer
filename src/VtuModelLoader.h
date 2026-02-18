#ifndef VTU_MODEL_LOADER_H
#define VTU_MODEL_LOADER_H

#include <QObject>
#include <QString>

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

#include <string>
#include <vector>

#include "NestedArrayInfo.h"

struct LoadedVtuModel {
  QString fileName;
  QString filePath;
  vtkSmartPointer<vtkUnstructuredGrid> grid;
  QVector<NestedArrayInfo> pointArrays;
};

class VtuModelLoader : public QObject {
  Q_OBJECT

public:
  explicit VtuModelLoader(QObject *parent = nullptr);
  void load(const QString &filePath);

  // Helper functions for component index mapping and name retrieval
  // These work with the componentNames structure created by load()
  static int comboIndexToVtkIndex(const NestedArrayInfo &array, int comboIndex);
  static int vtkIndexToComboIndex(const NestedArrayInfo &array, int vtkIndex);
  static QString getDisplayNameForVtkIndex(const NestedArrayInfo &array, int vtkIndex);
  static bool hasMagnitudeOption(const NestedArrayInfo &array);

signals:
  void modelLoaded(const LoadedVtuModel &model);
  void modelLoadingErrorOccured(const QString &errorMessage);
};

#endif
