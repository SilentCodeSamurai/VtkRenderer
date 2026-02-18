#include "App.h"
#include "VtuModelLoader.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkRenderWindow.h>

App::App(const QString &vtuFilePath, QWidget *parent)
    : QMainWindow(parent), modelLoader(this) {
  setupVtk();
  setupUi();
  setupConnections();

  if (!vtuFilePath.isEmpty()) {
    modelLoader.load(vtuFilePath);
  }
}

void App::setupVtk() {
  renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->SetBackground(0.12, 0.16, 0.20);

  modelMapper = vtkSmartPointer<vtkDataSetMapper>::New();
  scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
  scalarBar->SetNumberOfLabels(6);
  scalarBar->SetWidth(0.08);
  scalarBar->SetHeight(0.8);
  scalarBar->SetPosition(0.90, 0.10);
  scalarBar->SetVerticalTitleSeparation(18);
  scalarBar->SetVisibility(0);
  renderer->AddActor2D(scalarBar);
}

void App::setupUi() {
  setWindowTitle("VTK Renderer");
  resize(1200, 800);

  QWidget *central = new QWidget(this);
  setCentralWidget(central);

  auto *rootLayout = new QHBoxLayout(central);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(0);

  // VTK Widget
  vtkVisualizer = new QVTKOpenGLNativeWidget(this);
  vtkVisualizer->renderWindow()->AddRenderer(renderer);
  rootLayout->addWidget(vtkVisualizer, 1);

  // Right Panel
  auto *rightPanel = new QWidget(this);
  rightPanel->setMinimumWidth(300);
  auto *rightLayout = new QVBoxLayout(rightPanel);
  rightLayout->setContentsMargins(8, 8, 8, 8);
  rightLayout->setSpacing(12);

  // File Picker Component
  filePicker = new FilePickerWidget(this);
  filePicker->setFileFilter("VTU files (*.vtu);;All files (*.*)");
  filePicker->setPlaceholderText("📁 No VTU file selected");
  rightLayout->addWidget(filePicker);

  // Separator
  QFrame *separator1 = new QFrame(this);
  separator1->setFrameShape(QFrame::HLine);
  separator1->setFrameShadow(QFrame::Sunken);
  rightLayout->addWidget(separator1);

  // Component Selector
  arrayComponentSelector = new NestedSelectorWidget(this);
  arrayComponentSelector->setArrayLabel("📊 Array:");
  arrayComponentSelector->setComponentLabel("🔧 Component:");
  arrayComponentSelector->setEnabled(false); // Disabled until file loaded
  rightLayout->addWidget(arrayComponentSelector);

  // Separator
  QFrame *separator2 = new QFrame(this);
  separator2->setFrameShape(QFrame::HLine);
  separator2->setFrameShadow(QFrame::Sunken);
  rightLayout->addWidget(separator2);

  // Info label
  QLabel *infoLabel = new QLabel(
      "💡 Select an array and component to color the mesh.\n"
      "For multi-component arrays, 'Magnitude' shows all components.",
      this);
  infoLabel->setWordWrap(true);
  infoLabel->setStyleSheet("QLabel {"
                           "   color: #b3c4d6;"
                           "   font-size: 11px;"
                           "   padding: 6px;"
                           "   background-color: #151d26;"
                           "   border: 1px solid #3a4756;"
                           "   border-radius: 0px;"
                           "}");
  rightLayout->addWidget(infoLabel);

  rightLayout->addStretch(1);
  rootLayout->addWidget(rightPanel, 0);
}

void App::setupConnections() {
  connect(filePicker, &FilePickerWidget::fileSelectionChanged, &modelLoader,
          &VtuModelLoader::load);
  connect(filePicker, &FilePickerWidget::fileSelectionCleared, this,
          &App::onFileSelectionCleared);
  connect(filePicker, &FilePickerWidget::fileSelectionErrorOccurred, this,
          &App::onFileSelectionErrorOccurred);

  connect(&modelLoader, &VtuModelLoader::modelLoaded, this,
          &App::onModelLoaded);
  connect(&modelLoader, &VtuModelLoader::modelLoadingErrorOccured, this,
          &App::onModelLoadingErrorOccurred);

  connect(arrayComponentSelector, &NestedSelectorWidget::selectionChanged, this,
          &App::onSelectionChanged);
}

// Model
void App::updateModelActor() {
  if (modelActor != nullptr) {
    renderer->RemoveActor(modelActor);
    modelActor = nullptr;
  }
  if (model.grid == nullptr || model.grid->GetNumberOfPoints() == 0) {
    return;
  }
  modelMapper->SetInputData(model.grid);
  modelActor = vtkSmartPointer<vtkActor>::New();
  modelActor->SetMapper(modelMapper);
  renderer->AddActor(modelActor);
  renderer->ResetCamera();
}

void App::clearModelActor() {
  if (modelActor != nullptr) {
    renderer->RemoveActor(modelActor);
    modelActor = nullptr;
  }
  if (modelMapper != nullptr) {
    modelMapper->ScalarVisibilityOff();
  }
}

// Scalar Bar
void App::toggleScalarBarActorVisibility(bool shown) {
  if (scalarBar != nullptr) {
    scalarBar->SetVisibility(shown ? 1 : 0);
  }
}

// File Picker
void App::onFileSelectionChanged(const QString &filePath) {
  modelLoader.load(filePath);
}

void App::onFileSelectionCleared() {
  model.grid = nullptr;
  model.pointArrays.clear();
  model.fileName = QString();
  model.filePath = QString();

  clearModelActor();
  toggleScalarBarActorVisibility(false);
  arrayComponentSelector->clear();
  filePicker->clear();
  rerenderVtkWidget();
}

void App::onFileSelectionErrorOccurred(const QString &errorMessage) {
  QMessageBox::warning(this, "Error Loading File", errorMessage);
}

void App::onModelLoaded(const LoadedVtuModel &model) {
  if (model.pointArrays.empty()) {
    QMessageBox::warning(this, "No Point Arrays",
                         "No numeric point arrays found for coloring.");
    return;
  }

  this->model = model;

  if (arrayComponentSelector == nullptr) {
    return;
  }

  arrayComponentSelector->setNestedArrays(model.pointArrays);
  arrayComponentSelector->setCurrentArrayIndex(0);
  arrayComponentSelector->setEnabled(true);

  updateModelActor();
  // Determine initial component index: -1 for Magnitude if multi-component,
  // else 0
  int initialComponentIndex = 0;
  if (!model.pointArrays.isEmpty() &&
      VtuModelLoader::hasMagnitudeOption(model.pointArrays[0])) {
    initialComponentIndex = -1;
  }
  // Set the component selector to the initial index
  arrayComponentSelector->setCurrentComponentIndex(initialComponentIndex);
  filePicker->setSelectedFile(model.fileName);

  onSelectionChanged(0, initialComponentIndex);
}

void App::onModelLoadingErrorOccurred(const QString &errorMessage) {
  QMessageBox::warning(this, "Error Loading Model", errorMessage);
}

void App::onSelectionChanged(int arrayIndex, int componentIndex) {
  if (model.grid == nullptr || modelMapper == nullptr || scalarBar == nullptr) {
    return;
  }

  const auto &pointArrays = this->model.pointArrays;
  if (arrayIndex < 0 || arrayIndex >= static_cast<int>(pointArrays.size())) {
    QMessageBox::warning(this, "Invalid Array Index",
                         QString("Invalid array index: %1").arg(arrayIndex));
    return;
  }

  vtkPointData *pointData = this->model.grid->GetPointData();
  if (pointData == nullptr) {
    QMessageBox::warning(this, "No Point Data",
                         "Model has no point data for coloring.");
    return;
  }

  const std::string arrayName = pointArrays[arrayIndex].name.toStdString();
  vtkDataArray *arr = pointData->GetArray(arrayName.c_str());
  if (arr == nullptr) {
    QMessageBox::warning(this, "Array Unavailable",
                         QString("Array '%1' is unavailable in point data.")
                             .arg(QString::fromStdString(arrayName)));
    return;
  }

  // Configure mapper for scalar coloring
  modelMapper->SetInputData(this->model.grid);
  modelMapper->SetScalarModeToUsePointFieldData();
  modelMapper->SetColorModeToMapScalars();
  modelMapper->ScalarVisibilityOn();
  modelMapper->ColorByArrayComponent(arrayName.c_str(), componentIndex);

  // Set scalar range
  double range[2] = {0.0, 1.0};
  arr->GetRange(range, componentIndex);
  modelMapper->SetScalarRange(range);

  // Configure scalar bar - use parsed component names from model
  const auto &array = pointArrays[arrayIndex];
  QString componentText =
      VtuModelLoader::getDisplayNameForVtkIndex(array, componentIndex);
  const QString title = array.name + "\n" + componentText;
  scalarBar->SetLookupTable(modelMapper->GetLookupTable());
  scalarBar->SetTitle(title.toLocal8Bit().constData());
  scalarBar->SetVisibility(1);

  // Render
  rerenderVtkWidget();
}

void App::rerenderVtkWidget() {
  if (vtkVisualizer != nullptr) {
    vtkVisualizer->renderWindow()->Render();
  }
}