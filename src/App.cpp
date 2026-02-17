#include "App.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVTKOpenGLNativeWidget.h>
#include <QVBoxLayout>
#include <QWidget>

#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>

App::App(const QString &vtuFilePath, QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("VtkRenderer");
    resize(1200, 800);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(8);

    vtkWidget_ = new QVTKOpenGLNativeWidget(this);
    rootLayout->addWidget(vtkWidget_, 1);

    auto *rightPanel = new QWidget(this);
    rightPanel->setMinimumWidth(260);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(8, 8, 8, 8);
    rightLayout->setSpacing(8);

    fileLabel_ = new QLabel(this);
    fileLabel_->setWordWrap(true);
    rightLayout->addWidget(fileLabel_);

    auto *formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignLeft);

    arrayCombo_ = new QComboBox(this);
    componentCombo_ = new QComboBox(this);
    formLayout->addRow("Array:", arrayCombo_);
    formLayout->addRow("Component:", componentCombo_);

    auto *buttonsLayout = new QHBoxLayout();
    openFileButton_ = new QPushButton("Open file", this);
    closeFileButton_ = new QPushButton("Close file", this);
    buttonsLayout->addWidget(openFileButton_);
    buttonsLayout->addWidget(closeFileButton_);

    rightLayout->addLayout(formLayout);
    rightLayout->addLayout(buttonsLayout);
    rightLayout->addStretch(1);
    rootLayout->addWidget(rightPanel, 0);

    renderer_ = vtkSmartPointer<vtkRenderer>::New();
    renderer_->SetBackground(0.12, 0.16, 0.20);
    vtkWidget_->renderWindow()->AddRenderer(renderer_);

    scalarBar_ = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBar_->SetNumberOfLabels(6);
    scalarBar_->SetWidth(0.08);
    scalarBar_->SetHeight(0.8);
    scalarBar_->SetPosition(0.90, 0.10);
    scalarBar_->SetVerticalTitleSeparation(18);
    renderer_->AddActor2D(scalarBar_);
    mapper_ = vtkSmartPointer<vtkDataSetMapper>::New();
    coloringController_ = ColoringController(mapper_, scalarBar_);

    connect(arrayCombo_, &QComboBox::currentIndexChanged, this, [this](int idx) {
        updateComponentChoices(idx);
        applyColorSelection();
    });
    connect(componentCombo_, &QComboBox::currentIndexChanged, this, [this](int) {
        applyColorSelection();
    });
    connect(openFileButton_, &QPushButton::clicked, this, [this]() { openFile(); });
    connect(closeFileButton_, &QPushButton::clicked, this, [this]() { closeFile(); });

    setNoFileState();
    if (!vtuFilePath.isEmpty()) {
        loadFromPath(vtuFilePath);
    }
}

void App::clearLoadedData() {
    if (actor_ != nullptr) {
        renderer_->RemoveActor(actor_);
        actor_ = nullptr;
    }
    modelLoader_.clear();
    arrayCombo_->clear();
    componentCombo_->clear();
    coloringController_.clear();
}

void App::setNoFileState() {
    clearLoadedData();
    fileLabel_->setText("No file selected");
    arrayCombo_->setEnabled(false);
    componentCombo_->setEnabled(false);
    closeFileButton_->setEnabled(false);
    vtkWidget_->renderWindow()->Render();
}

void App::loadFromPath(const QString &filePath) {
    clearLoadedData();
    QString errorText;
    if (!modelLoader_.load(filePath, &errorText)) {
        QMessageBox::critical(this, "Load Error", errorText);
        setNoFileState();
        return;
    }
    mapper_->SetInputData(modelLoader_.grid());

    actor_ = vtkSmartPointer<vtkActor>::New();
    actor_->SetMapper(mapper_);
    renderer_->AddActor(actor_);

    for (const auto &arr : modelLoader_.pointArrays()) {
        arrayCombo_->addItem(arr.name.c_str());
    }

    fileLabel_->setText("File: " + QFileInfo(filePath).fileName());
    closeFileButton_->setEnabled(true);
    const bool hasArrays = (arrayCombo_->count() > 0);
    arrayCombo_->setEnabled(hasArrays);
    componentCombo_->setEnabled(hasArrays);
    if (!hasArrays) {
        QMessageBox::warning(this, "No Point Arrays", "No numeric point arrays found for coloring.");
        coloringController_.clear();
    }
    if (arrayCombo_->count() > 0) {
        arrayCombo_->setCurrentIndex(0);
        updateComponentChoices(0);
        applyColorSelection();
    }
    renderer_->ResetCamera();
    vtkWidget_->renderWindow()->Render();
}

void App::openFile() {
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open VTU File",
        QString(),
        "VTU files (*.vtu);;All files (*.*)");
    if (filePath.isEmpty()) {
        return;
    }
    loadFromPath(filePath);
}

void App::closeFile() { setNoFileState(); }

void App::updateComponentChoices(int arrayIndex) {
    componentCombo_->clear();
    const auto &arrays = modelLoader_.pointArrays();
    if (arrayIndex < 0 || arrayIndex >= static_cast<int>(arrays.size())) {
        componentCombo_->setEnabled(false);
        return;
    }

    const int nComp = arrays[arrayIndex].components;
    if (nComp <= 0) {
        componentCombo_->setEnabled(false);
        return;
    }

    if (nComp > 1) {
        componentCombo_->addItem("Magnitude", -1);
    }
    for (int c = 0; c < nComp; ++c) {
        componentCombo_->addItem(QString("Component %1").arg(c), c);
    }
    componentCombo_->setEnabled(true);
}

void App::applyColorSelection() {
    if (mapper_ == nullptr || modelLoader_.grid() == nullptr) {
        return;
    }
    const int arrayIndex = arrayCombo_->currentIndex();
    const auto &arrays = modelLoader_.pointArrays();
    if (arrayIndex < 0 || arrayIndex >= static_cast<int>(arrays.size())) {
        return;
    }

    const int component = componentCombo_->currentData().toInt();
    if (coloringController_.apply(modelLoader_, arrayIndex, component)) {
        vtkWidget_->renderWindow()->Render();
    }
}
