#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>

#include "ModelManager.h"
#include "widgets/ComponentSelectorWidget/ComponentSelectorWidget.h"
#include "widgets/FilePickerWidget/FilePickerWidget.h"

class QVTKOpenGLNativeWidget;

class App : public QMainWindow
{
    Q_OBJECT

public:
    explicit App(const QString &vtuFilePath = QString(), QWidget *parent = nullptr);

private slots:
    void onModelLoaded();
    void onModelCleared();
    void onSelectionChanged(int arrayIndex, int componentIndex);
    void onFilePickerError(const QString &message);
    void onModelError(const QString &message);

private:
    void setupVtk();
    void setupUi();
    void setupConnections();
    void requestRender();
    void showSelectionError(const QString &message);

private:
    // UI Components
    FilePickerWidget *m_filePicker;
    ComponentSelectorWidget *m_selector;
    QVTKOpenGLNativeWidget *m_vtkWidget;

    // VTK Components
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkDataSetMapper> m_mapper;
    vtkSmartPointer<vtkScalarBarActor> m_scalarBar;

    // Model state
    ModelManager m_modelManager;
};

#endif // APP_H
