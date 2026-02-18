#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>

#include "VtuModelLoader.h"
#include "widgets/FilePickerWidget/FilePickerWidget.h"
#include "widgets/NestedSelectorWidget/NestedSelectorWidget.h"

class QVTKOpenGLNativeWidget;

class App : public QMainWindow {
  Q_OBJECT

public:
  explicit App(const QString &vtuFilePath = QString(),
               QWidget *parent = nullptr);

private slots:
  void onModelLoaded(const LoadedVtuModel &model);
  void onModelLoadingErrorOccurred(const QString &errorMessage);

  void onFileSelectionChanged(const QString &filePath);
  void onFileSelectionCleared();
  void onFileSelectionErrorOccurred(const QString &message);

  void onSelectionChanged(int arrayIndex, int componentIndex);

private:
  void setupVtk();
  void setupUi();
  void setupConnections();
  void rerenderVtkWidget();

  void updateModelActor();
  void clearModelActor();
  void toggleScalarBarActorVisibility(bool shown);

  void clearComponentSelector();

signals:
  void arrayComponentSelectionChanged(int arrayIndex, int componentIndex);

private:
  // UI Components
  FilePickerWidget *filePicker;
  NestedSelectorWidget *arrayComponentSelector;
  QVTKOpenGLNativeWidget *vtkVisualizer;

  // VTK Components
  vtkSmartPointer<vtkRenderer> renderer;
  vtkSmartPointer<vtkActor> modelActor;
  vtkSmartPointer<vtkDataSetMapper> modelMapper;
  vtkSmartPointer<vtkScalarBarActor> scalarBar;

  // Model Loading
  VtuModelLoader modelLoader;
  LoadedVtuModel model;
};

#endif // APP_H
