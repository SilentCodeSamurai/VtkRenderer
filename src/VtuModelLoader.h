#ifndef VTU_MODEL_LOADER_H
#define VTU_MODEL_LOADER_H

#include <QObject>
#include <QString>

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

#include <string>
#include <vector>

#include "PointArrayInfo.h"

struct LoadedVtuModel {
  vtkSmartPointer<vtkUnstructuredGrid> grid;
  QVector<PointArrayInfo> pointArraysInfo;
};

class VtuModelLoader : public QObject {
  Q_OBJECT

public:
  explicit VtuModelLoader(QObject *parent = nullptr);
  void load(const QString &filePath);

  // Helper functions for component index mapping and name retrieval
  // These work with the componentNames structure created by load()
  static int comboIndexToVtkIndex(const PointArrayInfo &arrayInfo, int comboIndex);
  static int vtkIndexToComboIndex(const PointArrayInfo &arrayInfo, int vtkIndex);
  static QString getDisplayNameForVtkIndex(const PointArrayInfo &arrayInfo,
                                           int vtkIndex);
  static bool hasMagnitudeOption(const PointArrayInfo &arrayInfo);

signals:
  void modelLoaded(LoadedVtuModel *model, const QString &modelFilePath);
  void modelLoadingErrorOccured(const QString &errorMessage);
};

#endif
