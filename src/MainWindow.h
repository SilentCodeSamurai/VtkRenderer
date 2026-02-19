#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <qfileinfo.h>
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>

#include "PointArrayInfo.h"
#include "VtuModelLoader.h"

class QVTKOpenGLNativeWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(const QString &vtuFilePath = QString(),
                      QWidget *parent = nullptr);

private slots:
  /* INTERNAL SLOTS */
  /* File Selection */
  void onOpenFileClicked();
  void onCloseFileClicked();

  /* Model Loading */
  void onModelLoaded(LoadedVtuModel *model, const QString &modelFilePath);
  void onModelLoadingErrorOccurred(const QString &errorMessage);

  /* Array/Component Selector */
  void onArrayIndexChanged(int arrayIndex);
  void onComponentIndexChanged(int componentIndex);

private:
  /* SETUP */
  void setupVtk();
  void setupUi();
  void setupConnections();

  /* UI Upates */
  /* Array/Component selector */
  void setArrayComboboxItems(QVector<QString> items);
  void setComponentComboboxItems(QVector<QString> items);
  void setArrayComboboxIndex(int index);
  void setComponentComboboxIndex(int vtkComponentIndex);
  void setArrayComboboxEnabled(bool enabled);
  void setComponentComboboxEnabled(bool enabled);
  void clearSelectorComboboxes();

  /* File Selection */
  void syncFileSelectionWithOpenedFile();

  /* VTK */
  void setScalarBarVisibility(bool visible);
  void syncModelActorWithOpenedModel();
  void upadateSceneColoring(int arrayIndex, int componentIndex);
  void rerenderVtkVisualizer();

  /* Helpers */
  void closeFile();

private:
  /* UI COMPONENTS */
  /* File Picker */
  QLabel *fileLabel;
  QString fileLabelPlaceholderText;
  QPushButton *openFileButton;
  QPushButton *closeFileButton;
  QString fileFilter;

  /* Array/Component Selector */
  QGroupBox *arrayComponentGroupBox;
  QLabel *arrayLabel;
  QComboBox *arrayCombo;
  QLabel *componentLabel;
  QComboBox *componentCombo;

  /* VTK */
  QVTKOpenGLNativeWidget *vtkVisualizer;

  /* VTK COMPONENTS */
  vtkSmartPointer<vtkRenderer> renderer;
  vtkSmartPointer<vtkActor> modelActor;
  vtkSmartPointer<vtkDataSetMapper> modelMapper;
  vtkSmartPointer<vtkScalarBarActor> scalarBar;

  /* STATE */
  LoadedVtuModel *openedVtuModel;
  QFileInfo *openedVtuModelFileInfo;

  /* HELPERS */
  VtuModelLoader modelLoader;
};

#endif // MAINWINDOW_H
