#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include <QString>

#include <vtkSmartPointer.h>

#include "ColoringController.h"
#include "VtuModelLoader.h"

class QLabel;
class QComboBox;
class QPushButton;
class QVTKOpenGLNativeWidget;

class vtkActor;
class vtkDataSetMapper;
class vtkRenderer;
class vtkScalarBarActor;
class vtkUnstructuredGrid;

class App : public QMainWindow {
public:
    explicit App(const QString &vtuFilePath = QString(), QWidget *parent = nullptr);

private:
    void clearLoadedData();
    void setNoFileState();
    void loadFromPath(const QString &filePath);
    void openFile();
    void closeFile();
    void updateComponentChoices(int arrayIndex);
    void applyColorSelection();

    QVTKOpenGLNativeWidget *vtkWidget_ = nullptr;
    QLabel *fileLabel_ = nullptr;
    QComboBox *arrayCombo_ = nullptr;
    QComboBox *componentCombo_ = nullptr;
    QPushButton *openFileButton_ = nullptr;
    QPushButton *closeFileButton_ = nullptr;
    vtkSmartPointer<vtkDataSetMapper> mapper_;
    vtkSmartPointer<vtkActor> actor_;
    vtkSmartPointer<vtkRenderer> renderer_;
    vtkSmartPointer<vtkScalarBarActor> scalarBar_;
    VtuModelLoader modelLoader_;
    ColoringController coloringController_{nullptr, nullptr};
};

#endif
