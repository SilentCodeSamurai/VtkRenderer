#include "MainWindow.h"
#include "VtuModelLoader.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkRenderWindow.h>

MainWindow::MainWindow(const QString &vtuFilePath, QWidget *parent)
    : QMainWindow(parent), modelLoader(this),
      fileFilter("VTU files (*.vtu);;All files (*.*)"),
      fileLabelPlaceholderText("📁 No VTU file selected") {
  setupVtk();
  setupUi();
  setupConnections();

  if (!vtuFilePath.isEmpty()) {
    modelLoader.load(vtuFilePath);
  }
}

/* SETUP */
void MainWindow::setupVtk() {
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

void MainWindow::setupUi() {
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
  rightPanel->setStyleSheet("QWidget {"
                            "   background-color: #13181f;"
                            "}");
  auto *rightLayout = new QVBoxLayout(rightPanel);
  rightLayout->setContentsMargins(8, 8, 8, 8);
  rightLayout->setSpacing(12);

  // File Picker Component
  QWidget *filePickerWidget = new QWidget(this);
  QVBoxLayout *filePickerLayout = new QVBoxLayout(filePickerWidget);
  filePickerLayout->setContentsMargins(0, 0, 0, 0);
  filePickerLayout->setSpacing(4);

  fileLabel = new QLabel(fileLabelPlaceholderText, this);
  fileLabel->setWordWrap(true);
  fileLabel->setStyleSheet("QLabel {"
                           "   color: #a7b4c2;"
                           "   background-color: #151d26;"
                           "   border: 1px solid #2a3a4b;"
                           "   border-radius: 0px;"
                           "   padding: 8px;"
                           "   font-family: monospace;"
                           "}");

  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->setSpacing(8);

  openFileButton = new QPushButton("📂 Open File", this);
  // Default state: no file opened yet → keep a colorful border even when idle
  openFileButton->setStyleSheet("QPushButton {"
                                "   background-color: #121820;"
                                "   color: #d9e7f5;"
                                "   border: 2px solid #00bcd4;"
                                "   border-radius: 0px;"
                                "   padding: 8px 16px;"
                                "   font-weight: 600;"
                                "}"
                                "QPushButton:hover {"
                                "   background-color: #0f2630;"
                                "   color: #64e8ff;"
                                "   border: 2px solid #00bcd4;"
                                "}"
                                "QPushButton:pressed {"
                                "   background-color: #093946;"
                                "   border: 2px solid #00bcd4;"
                                "}");

  closeFileButton = new QPushButton("✖ Close", this);
  closeFileButton->setStyleSheet("QPushButton {"
                                 "   background-color: #121820;"
                                 "   color: #d9e7f5;"
                                 "   border: 2px solid #2a3a4b;"
                                 "   border-radius: 0px;"
                                 "   padding: 8px 16px;"
                                 "   font-weight: 600;"
                                 "}"
                                 "QPushButton:hover {"
                                 "   background-color: #2a1713;"
                                 "   color: #ff9c87;"
                                 "   border: 2px solid #ff5a36;"
                                 "}"
                                 "QPushButton:pressed {"
                                 "   background-color: #3f1510;"
                                 "   border: 2px solid #ff5a36;"
                                 "}"
                                 "QPushButton:disabled {"
                                 "   background-color: #171d24;"
                                 "   border: 2px solid #35414e;"
                                 "   color: #607182;"
                                 "}");
  closeFileButton->setEnabled(false);

  buttonLayout->addWidget(openFileButton);
  buttonLayout->addWidget(closeFileButton);
  buttonLayout->addStretch();

  filePickerLayout->addWidget(fileLabel);
  filePickerLayout->addLayout(buttonLayout);
  filePickerWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  rightLayout->addWidget(filePickerWidget);

  // Separator
  QFrame *separator1 = new QFrame(this);
  separator1->setFrameShape(QFrame::HLine);
  separator1->setFrameShadow(QFrame::Sunken);
  rightLayout->addWidget(separator1);

  // Component Selector
  arrayComponentGroupBox = new QGroupBox("Data Selector", this);
  arrayComponentGroupBox->setStyleSheet("QGroupBox {"
                                        "   color: #d9e7f5;"
                                        "   border: 1px solid #3a4756;"
                                        "   border-radius: 0px;"
                                        "   margin-top: 10px;"
                                        "   padding-top: 8px;"
                                        "   background-color: #151d26;"
                                        "}"
                                        "QGroupBox::title {"
                                        "   subcontrol-origin: margin;"
                                        "   left: 8px;"
                                        "   padding: 0 4px;"
                                        "   color: #64e8ff;"
                                        "   font-weight: 600;"
                                        "}");
  arrayComponentGroupBox->setEnabled(false); // Disabled until file loaded
  QVBoxLayout *groupLayout = new QVBoxLayout(arrayComponentGroupBox);
  groupLayout->setSpacing(8);

  // Array row
  QHBoxLayout *arrayLayout = new QHBoxLayout();
  arrayLayout->setSpacing(8);

  arrayLabel = new QLabel("📊 Array:", this);
  arrayLabel->setMinimumWidth(80);
  arrayLabel->setStyleSheet("QLabel {"
                            "   color: #8fb0cf;"
                            "   font-weight: 600;"
                            "   background-color: transparent;"
                            "}");

  arrayCombo = new QComboBox(this);
  arrayCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  arrayCombo->setStyleSheet("QComboBox {"
                            "   border: 1px solid #3a4756;"
                            "   border-radius: 0px;"
                            "   padding: 6px;"
                            "   background-color: #10161d;"
                            "   color: #e6f3ff;"
                            "   selection-background-color: #00bcd4;"
                            "   selection-color: #04151d;"
                            "}"
                            "QComboBox::drop-down {"
                            "   border-left: 1px solid #3a4756;"
                            "   width: 22px;"
                            "   background-color: #141c24;"
                            "}"
                            "QComboBox:enabled:hover {"
                            "   border: 1px solid #00bcd4;"
                            "}"
                            "QComboBox:disabled {"
                            "   background-color: #1a2129;"
                            "   color: #5a6877;"
                            "}"
                            "QComboBox QAbstractItemView {"
                            "   background-color: #10161d;"
                            "   color: #e6f3ff;"
                            "   border: 1px solid #3a4756;"
                            "   selection-background-color: #00bcd4;"
                            "   selection-color: #04151d;"
                            "}");

  arrayLayout->addWidget(arrayLabel);
  arrayLayout->addWidget(arrayCombo);

  // Component row
  QHBoxLayout *componentLayout = new QHBoxLayout();
  componentLayout->setSpacing(8);

  componentLabel = new QLabel("🔧 Component:", this);
  componentLabel->setMinimumWidth(80);
  componentLabel->setStyleSheet("QLabel {"
                                "   color: #8fb0cf;"
                                "   font-weight: 600;"
                                "   background-color: transparent;"
                                "}");

  componentCombo = new QComboBox(this);
  componentCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  componentCombo->setStyleSheet("QComboBox {"
                                "   border: 1px solid #3a4756;"
                                "   border-radius: 0px;"
                                "   padding: 6px;"
                                "   background-color: #10161d;"
                                "   color: #e6f3ff;"
                                "   selection-background-color: #ff5a36;"
                                "   selection-color: #1a0a06;"
                                "}"
                                "QComboBox::drop-down {"
                                "   border-left: 1px solid #3a4756;"
                                "   width: 22px;"
                                "   background-color: #141c24;"
                                "}"
                                "QComboBox:enabled:hover {"
                                "   border: 1px solid #ff5a36;"
                                "}"
                                "QComboBox:disabled {"
                                "   background-color: #1a2129;"
                                "   color: #5a6877;"
                                "}"
                                "QComboBox QAbstractItemView {"
                                "   background-color: #10161d;"
                                "   color: #e6f3ff;"
                                "   border: 1px solid #3a4756;"
                                "   selection-background-color: #ff5a36;"
                                "   selection-color: #1a0a06;"
                                "}");

  componentLayout->addWidget(componentLabel);
  componentLayout->addWidget(componentCombo);

  groupLayout->addLayout(arrayLayout);
  groupLayout->addLayout(componentLayout);
  rightLayout->addWidget(arrayComponentGroupBox);

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

void MainWindow::setupConnections() {
  // File picker connections
  connect(openFileButton, &QPushButton::clicked, this,
          &MainWindow::onOpenFileClicked);
  connect(closeFileButton, &QPushButton::clicked, this,
          &MainWindow::onCloseFileClicked);

  // Model loader connections
  connect(&modelLoader, &VtuModelLoader::modelLoaded, this,
          &MainWindow::onModelLoaded);
  connect(&modelLoader, &VtuModelLoader::modelLoadingErrorOccured, this,
          &MainWindow::onModelLoadingErrorOccurred);

  // Array/Component selector connections
  connect(arrayCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &MainWindow::onArrayIndexChanged);
  connect(componentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MainWindow::onComponentIndexChanged);
}

/* INTERNAL SLOTS */
/* File Selection */
void MainWindow::onOpenFileClicked() {
  QString filePath =
      QFileDialog::getOpenFileName(this, "Select File", QString(), fileFilter);
  // User cancelled the dialog - this is normal, just return silently
  if (filePath.isEmpty()) {
    return;
  }
  // Validate file
  QFile file(filePath);
  if (!file.exists()) {
    QMessageBox::warning(this, "File not found",
                         QString("File not found: %1").arg(filePath));
    return;
  }
  // Load model(Loader will emit modelLoaded or modelLoadingErrorOccurred)
  modelLoader.load(filePath);
}

void MainWindow::onCloseFileClicked() { closeFile(); }

/* Model Loading */
void MainWindow::onModelLoaded(LoadedVtuModel *model,
                               const QString &modelFilePath) {
  // Validate model
  if (model == nullptr || model->grid == nullptr) {
    QMessageBox::warning(this, "No Model Loaded",
                         "No model loaded. Please open a valid VTU file.");
    return;
  }
  if (model->pointArraysInfo.empty()) {
    QMessageBox::warning(this, "No Point Arrays",
                         "No numeric point arrays found for coloring.");
    return;
  }

  // Close any previously opened file to avoid leaks
  closeFile();

  // Set attributes – MainWindow now owns the model and its file info
  this->openedVtuModel.reset(model);
  this->openedVtuModelFileInfo.reset(new QFileInfo(modelFilePath));

  // Set initial array and component indices
  int initialArrayIndex = 0;
  int initialComponentIndex = 0;
  if (!openedVtuModel->pointArraysInfo.isEmpty() &&
      VtuModelLoader::hasMagnitudeOption(
          openedVtuModel->pointArraysInfo[initialArrayIndex])) {
    initialComponentIndex = -1;
  }

  // Set array combobox items
  QVector<QString> arrayNames;
  for (const PointArrayInfo &arrayInfo : openedVtuModel->pointArraysInfo) {
    arrayNames.push_back(arrayInfo.name);
  }

  // Update Selector
  setArrayComboboxItems(arrayNames);
  setComponentComboboxItems(
      openedVtuModel->pointArraysInfo[initialArrayIndex].componentNames);
  setArrayComboboxIndex(initialArrayIndex);
  setComponentComboboxIndex(initialComponentIndex);
  setArrayComboboxEnabled(true);
  setComponentComboboxEnabled(true);
  arrayComponentGroupBox->setEnabled(true);

  // Update File Selection
  syncFileSelectionWithOpenedFile();

  // Update VTK
  syncModelActorWithOpenedModel();
  upadateSceneColoring(0, initialComponentIndex);
  rerenderVtkVisualizer();
}

void MainWindow::onModelLoadingErrorOccurred(const QString &errorMessage) {
  QMessageBox::warning(this, "Error Loading Model", errorMessage);
}

/* Array/Component Selector */
void MainWindow::onArrayIndexChanged(int arrayIndex) {
  if (openedVtuModel == nullptr) {
    return;
  }
  // Validate array index
  if (arrayIndex < 0 ||
      arrayIndex >= static_cast<int>(openedVtuModel->pointArraysInfo.size())) {
    QMessageBox::warning(this, "Invalid Array Index",
                         QString("Invalid array index: %1").arg(arrayIndex));
    return;
  }
  // Get array info
  const auto &arrayInfo = openedVtuModel->pointArraysInfo[arrayIndex];
  int initialComponentIndex =
      VtuModelLoader::hasMagnitudeOption(arrayInfo) ? -1 : 0;

  // Update Selector
  setArrayComboboxIndex(arrayIndex);
  setComponentComboboxItems(
      openedVtuModel->pointArraysInfo[arrayIndex].componentNames);
  setComponentComboboxIndex(initialComponentIndex);

  // Update VTK
  upadateSceneColoring(arrayIndex, initialComponentIndex);
  rerenderVtkVisualizer();
}

void MainWindow::onComponentIndexChanged(int comboIndex) {
  if (openedVtuModel == nullptr) {
    return;
  }
  // Validate array index
  int arrayIndex = arrayCombo->currentIndex();
  if (!(arrayIndex >= 0 &&
        arrayIndex < openedVtuModel->pointArraysInfo.size())) {
    return;
  }
  const auto &array = openedVtuModel->pointArraysInfo[arrayIndex];

  // Convert combo index to vtk index
  int vtkComponentIndex =
      VtuModelLoader::comboIndexToVtkIndex(array, comboIndex);

  // Update Selector
  setComponentComboboxIndex(vtkComponentIndex);

  // Update VTK
  upadateSceneColoring(arrayIndex, vtkComponentIndex);
  rerenderVtkVisualizer();
}

/* UI UPDATES */
/* Array/Component selector */
void MainWindow::setArrayComboboxItems(QVector<QString> items) {
  arrayCombo->clear();
  arrayCombo->addItems(items);
}

void MainWindow::setComponentComboboxItems(QVector<QString> items) {
  componentCombo->clear();
  componentCombo->addItems(items);
}

void MainWindow::setArrayComboboxIndex(int index) {
  if (!(index >= 0 && index < arrayCombo->count())) {
    return;
  }
  arrayCombo->blockSignals(true);
  arrayCombo->setCurrentIndex(index);
  arrayCombo->blockSignals(false);
}

void MainWindow::setComponentComboboxIndex(int vtkComponentIndex) {
  int arrayIndex = arrayCombo->currentIndex();
  if (!(arrayIndex >= 0 &&
        arrayIndex < openedVtuModel->pointArraysInfo.size())) {
    return;
  }
  const auto &arrayInfo = openedVtuModel->pointArraysInfo[arrayIndex];

  int comboIndex =
      VtuModelLoader::vtkIndexToComboIndex(arrayInfo, vtkComponentIndex);

  if (!(comboIndex >= 0 && comboIndex < componentCombo->count())) {
    return;
  }
  componentCombo->blockSignals(true);
  componentCombo->setCurrentIndex(comboIndex);
  componentCombo->blockSignals(false);
}

void MainWindow::setArrayComboboxEnabled(bool enabled) {
  arrayCombo->setEnabled(enabled);
}

void MainWindow::setComponentComboboxEnabled(bool enabled) {
  componentCombo->setEnabled(enabled);
}

void MainWindow::clearSelectorComboboxes() {
  arrayCombo->clear();
  componentCombo->clear();
}

/* File Selection */
void MainWindow::syncFileSelectionWithOpenedFile() {
  if (openedVtuModelFileInfo == nullptr) {
    fileLabel->setText(fileLabelPlaceholderText);
    fileLabel->setToolTip(QString());
    fileLabel->setStyleSheet("QLabel {"
                             "   color: #a7b4c2;"
                             "   background-color: #151d26;"
                             "   border: 1px solid #2a3a4b;"
                             "   border-radius: 0px;"
                             "   padding: 8px;"
                             "   font-family: monospace;"
                             "}");
    closeFileButton->setEnabled(false);

    // No file opened → keep the open button visually highlighted
    openFileButton->setStyleSheet("QPushButton {"
                                  "   background-color: #121820;"
                                  "   color: #d9e7f5;"
                                  "   border: 2px solid #00bcd4;"
                                  "   border-radius: 0px;"
                                  "   padding: 8px 16px;"
                                  "   font-weight: 600;"
                                  "}"
                                  "QPushButton:hover {"
                                  "   background-color: #0f2630;"
                                  "   color: #64e8ff;"
                                  "   border: 2px solid #00bcd4;"
                                  "}"
                                  "QPushButton:pressed {"
                                  "   background-color: #093946;"
                                  "   border: 2px solid #00bcd4;"
                                  "}");
  } else {
    fileLabel->setText("📄 " + openedVtuModelFileInfo->fileName());
    fileLabel->setToolTip(openedVtuModelFileInfo->filePath());
    fileLabel->setStyleSheet("QLabel {"
                             "   color: #a7b4c2;"
                             "   background-color: #151d26;"
                             "   border: 1px solid #2a3a4b;"
                             "   border-radius: 0px;"
                             "   padding: 8px;"
                             "   font-family: monospace;"
                             "}");
    closeFileButton->setEnabled(true);

    // File opened → remove idle highlight; keep it only on hover
    openFileButton->setStyleSheet("QPushButton {"
                                  "   background-color: #121820;"
                                  "   color: #d9e7f5;"
                                  "   border: 2px solid #2a3a4b;"
                                  "   border-radius: 0px;"
                                  "   padding: 8px 16px;"
                                  "   font-weight: 600;"
                                  "}"
                                  "QPushButton:hover {"
                                  "   background-color: #0f2630;"
                                  "   color: #64e8ff;"
                                  "   border: 2px solid #00bcd4;"
                                  "}"
                                  "QPushButton:pressed {"
                                  "   background-color: #093946;"
                                  "   border: 2px solid #00bcd4;"
                                  "}");
  }
}

/* VTK */
void MainWindow::setScalarBarVisibility(bool visible) {
  if (scalarBar == nullptr) {
    return;
  }
  scalarBar->SetVisibility(visible);
}

void MainWindow::syncModelActorWithOpenedModel() {
  // Remove and clear the current model actor
  if (modelActor != nullptr) {
    renderer->RemoveActor(modelActor);
    modelActor = nullptr;
  }
  // If model is nullptr - turn off model mapper scalar visibility
  if (openedVtuModel == nullptr) {
    if (modelMapper != nullptr) {
      modelMapper->ScalarVisibilityOff();
    }
    return;
  }
  // If model grid is nullptr or has no points - silently return
  if (openedVtuModel->grid == nullptr ||
      openedVtuModel->grid->GetNumberOfPoints() == 0) {
    return;
  }
  // If model mapper is nullptr - silently return
  if (modelMapper == nullptr) {
    return;
  }
  // Set model mapper input data and create a new model actor
  modelMapper->SetInputData(openedVtuModel->grid);
  modelActor = vtkSmartPointer<vtkActor>::New();
  modelActor->SetMapper(modelMapper);
  renderer->AddActor(modelActor);
  renderer->ResetCamera();
}

void MainWindow::upadateSceneColoring(int arrayIndex, int componentIndex) {
  if (openedVtuModel == nullptr) {
    return;
  }
  if (openedVtuModel->grid == nullptr) {
    return;
  }
  if (modelMapper == nullptr) {
    return;
  }
  if (scalarBar == nullptr) {
    return;
  }

  const auto &pointArrays = this->openedVtuModel->pointArraysInfo;
  if (arrayIndex < 0 ||
      arrayIndex >= static_cast<int>(openedVtuModel->pointArraysInfo.size())) {
    QMessageBox::warning(this, "Invalid Array Index",
                         QString("Invalid array index: %1").arg(arrayIndex));
    return;
  }

  vtkPointData *pointData = this->openedVtuModel->grid->GetPointData();
  if (pointData == nullptr) {
    QMessageBox::warning(this, "No Point Data",
                         "Model has no point data for coloring.");
    return;
  }

  const std::string arrayName =
      openedVtuModel->pointArraysInfo[arrayIndex].name.toStdString();
  vtkDataArray *arr = pointData->GetArray(arrayName.c_str());
  if (arr == nullptr) {
    QMessageBox::warning(this, "Array Unavailable",
                         QString("Array '%1' is unavailable in point data.")
                             .arg(QString::fromStdString(arrayName)));
    return;
  }

  // Configure mapper for scalar coloring
  modelMapper->SetInputData(this->openedVtuModel->grid);
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
}

void MainWindow::rerenderVtkVisualizer() {
  if (vtkVisualizer != nullptr) {
    vtkVisualizer->renderWindow()->Render();
  }
}

/* Helpers */
void MainWindow::closeFile() {
  // Clear attributes
  openedVtuModel.reset(nullptr);
  openedVtuModelFileInfo.reset(nullptr);

  // Update Selector
  clearSelectorComboboxes();
  setArrayComboboxEnabled(false);
  setComponentComboboxEnabled(false);
  arrayComponentGroupBox->setEnabled(false);

  // Update File Selection
  syncFileSelectionWithOpenedFile();

  // Update VTK
  syncModelActorWithOpenedModel();
  setScalarBarVisibility(false);
  rerenderVtkVisualizer();
}
