#include "App.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>
#include <QFrame>

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkRenderWindow.h>

App::App(const QString &vtuFilePath, QWidget *parent)
    : QMainWindow(parent)
    , m_modelManager(this)
{
    setupVtk();
    setupUi();
    setupConnections();

    if (!vtuFilePath.isEmpty())
    {
        m_filePicker->setFile(vtuFilePath);
    }
}

void App::setupVtk()
{
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(0.12, 0.16, 0.20);

    m_mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    m_scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
    m_scalarBar->SetNumberOfLabels(6);
    m_scalarBar->SetWidth(0.08);
    m_scalarBar->SetHeight(0.8);
    m_scalarBar->SetPosition(0.90, 0.10);
    m_scalarBar->SetVerticalTitleSeparation(18);
    m_scalarBar->SetVisibility(0);
    m_renderer->AddActor2D(m_scalarBar);
}

void App::setupUi()
{
    setWindowTitle("VTK Renderer");
    resize(1200, 800);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // VTK Widget
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    m_vtkWidget->renderWindow()->AddRenderer(m_renderer);
    rootLayout->addWidget(m_vtkWidget, 1);

    // Right Panel
    auto *rightPanel = new QWidget(this);
    rightPanel->setMinimumWidth(300);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(8, 8, 8, 8);
    rightLayout->setSpacing(12);

    // File Picker Component
    m_filePicker = new FilePickerWidget(this);
    m_filePicker->setFileFilter("VTU files (*.vtu);;All files (*.*)");
    m_filePicker->setPlaceholderText("📁 No VTU file selected");
    rightLayout->addWidget(m_filePicker);

    // Separator
    QFrame *separator1 = new QFrame(this);
    separator1->setFrameShape(QFrame::HLine);
    separator1->setFrameShadow(QFrame::Sunken);
    rightLayout->addWidget(separator1);

    // Component Selector
    m_selector = new ComponentSelectorWidget(this);
    m_selector->withLabels("📊 Array:", "🔧 Component:");
    m_selector->setEnabled(false); // Disabled until file loaded
    rightLayout->addWidget(m_selector);

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
    infoLabel->setStyleSheet(
        "QLabel {"
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

void App::setupConnections()
{
    connect(m_filePicker, &FilePickerWidget::fileOpened,
            &m_modelManager, &ModelManager::loadFromPath);
    connect(m_filePicker, &FilePickerWidget::fileClosed,
            &m_modelManager, &ModelManager::clearModel);
    connect(m_filePicker, &FilePickerWidget::error,
            this, &App::onFilePickerError);

    connect(m_selector, &ComponentSelectorWidget::selectionChanged,
            this, &App::onSelectionChanged);

    connect(&m_modelManager, &ModelManager::modelLoaded,
            this, &App::onModelLoaded);
    connect(&m_modelManager, &ModelManager::modelCleared,
            this, &App::onModelCleared);
    connect(&m_modelManager, &ModelManager::errorOccurred,
            this, &App::onModelError);
}

void App::onModelLoaded()
{
    QVector<ArrayInfo> arrays;
    for (const auto &arr : m_modelManager.pointArrays())
    {
        arrays.append(ArrayInfo(
            QString::fromStdString(arr.name),
            arr.components));
    }
    m_selector->setArrays(arrays);
    m_selector->setEnabled(!arrays.isEmpty());

    if (m_actor != nullptr)
    {
        m_renderer->RemoveActor(m_actor);
        m_actor = nullptr;
    }

    m_mapper->SetInputData(m_modelManager.grid());
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
    m_renderer->AddActor(m_actor);
    m_renderer->ResetCamera();

    if (!arrays.isEmpty())
    {
        onSelectionChanged(m_selector->currentArrayIndex(), m_selector->currentComponentIndex());
    }
    else
    {
        requestRender();
    }

    if (arrays.isEmpty())
    {
        QMessageBox::warning(this, "No Point Arrays",
                             "No numeric point arrays found for coloring.");
    }
}

void App::onModelCleared()
{
    if (m_actor != nullptr)
    {
        m_renderer->RemoveActor(m_actor);
        m_actor = nullptr;
    }

    m_selector->clear();
    m_selector->setEnabled(false);

    if (m_mapper != nullptr)
    {
        m_mapper->ScalarVisibilityOff();
    }
    if (m_scalarBar != nullptr)
    {
        m_scalarBar->SetVisibility(0);
    }
    requestRender();
}

void App::onSelectionChanged(int arrayIndex, int componentIndex)
{
    if (!m_modelManager.hasModel() || m_mapper == nullptr || m_scalarBar == nullptr)
    {
        return;
    }

    const auto &pointArrays = m_modelManager.pointArrays();
    if (arrayIndex < 0 || arrayIndex >= static_cast<int>(pointArrays.size()))
    {
        showSelectionError(QString("Invalid array index: %1").arg(arrayIndex));
        return;
    }

    vtkPointData *pointData = m_modelManager.grid()->GetPointData();
    if (pointData == nullptr)
    {
        showSelectionError("Model has no point data for coloring.");
        return;
    }

    const std::string &arrayName = pointArrays[arrayIndex].name;
    vtkDataArray *arr = pointData->GetArray(arrayName.c_str());
    if (arr == nullptr)
    {
        showSelectionError(
            QString("Array '%1' is unavailable in point data.")
                .arg(QString::fromStdString(arrayName)));
        return;
    }

    m_mapper->SetInputData(m_modelManager.grid());
    m_mapper->SetScalarModeToUsePointFieldData();
    m_mapper->SetColorModeToMapScalars();
    m_mapper->ScalarVisibilityOn();
    m_mapper->ColorByArrayComponent(arrayName.c_str(), componentIndex);

    double range[2] = {0.0, 1.0};
    arr->GetRange(range, componentIndex);
    m_mapper->SetScalarRange(range);

    const QString componentText = (componentIndex < 0)
        ? "Magnitude"
        : QString("Component %1").arg(componentIndex);
    const QString title = QString::fromStdString(arrayName) + "\n" + componentText;
    m_scalarBar->SetLookupTable(m_mapper->GetLookupTable());
    m_scalarBar->SetTitle(title.toLocal8Bit().constData());
    m_scalarBar->SetVisibility(1);

    requestRender();
}

void App::requestRender()
{
    if (m_vtkWidget != nullptr)
    {
        m_vtkWidget->renderWindow()->Render();
    }
}

void App::onFilePickerError(const QString &message)
{
    QMessageBox::critical(this, "File Error", message);
}

void App::onModelError(const QString &message)
{
    QMessageBox::critical(this, "Load Error", message);
}

void App::showSelectionError(const QString &message)
{
    QMessageBox::warning(this, "Selection Error", message);
}
