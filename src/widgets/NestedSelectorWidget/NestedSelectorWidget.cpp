#include "NestedSelectorWidget.h"
#include "../../VtuModelLoader.h"

NestedSelectorWidget::NestedSelectorWidget(QWidget *parent)
    : QWidget(parent), arrayLabelText("Array:"),
      componentLabelText("Component:"), groupBoxTitle("Data Selector") {
  setupUi();
  setupConnections();
clear();
}

void NestedSelectorWidget::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(8);

  groupBox = new QGroupBox(groupBoxTitle, this);
  groupBox->setStyleSheet("QGroupBox {"
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
  QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);
  groupLayout->setSpacing(8);

  // Array row
  QHBoxLayout *arrayLayout = new QHBoxLayout();
  arrayLayout->setSpacing(8);

  arrayLabel = new QLabel(arrayLabelText, this);
  arrayLabel->setMinimumWidth(80);
  arrayLabel->setStyleSheet("QLabel {"
                            "   color: #8fb0cf;"
                            "   font-weight: 600;"
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
                            "QComboBox:hover {"
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
  componentLayout = new QHBoxLayout();
  componentLayout->setSpacing(8);

  componentLabel = new QLabel(componentLabelText, this);
  componentLabel->setMinimumWidth(80);
  componentLabel->setStyleSheet("QLabel {"
                                "   color: #8fb0cf;"
                                "   font-weight: 600;"
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
                                "QComboBox:hover {"
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

  // Add to group
  groupLayout->addLayout(arrayLayout);
  groupLayout->addLayout(componentLayout);

  mainLayout->addWidget(groupBox);

  // Set size policy
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void NestedSelectorWidget::setupConnections() {
  connect(arrayCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &NestedSelectorWidget::onArrayIndexChanged);

  connect(componentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &NestedSelectorWidget::onComponentIndexChanged);
}

void NestedSelectorWidget::setGroupBoxTitle(const QString &title) {
  groupBoxTitle = title;
  groupBox->setTitle(title);
}

void NestedSelectorWidget::setArrayLabel(const QString &label) {
  arrayLabelText = label;
  arrayLabel->setText(label);
}

void NestedSelectorWidget::setComponentLabel(const QString &label) {
  componentLabelText = label;
  componentLabel->setText(label);
}

void NestedSelectorWidget::setEnabled(bool enabled) {
  QWidget::setEnabled(enabled);
  groupBox->setEnabled(enabled);
}

void NestedSelectorWidget::clear() {
  arrayCombo->clear();
  componentCombo->clear();
}

void NestedSelectorWidget::setNestedArrays(
    const QVector<NestedArrayInfo> &arrays) {
  if (arrayCombo == nullptr || componentCombo == nullptr) {
    return;
  }
  this->arrays = arrays;
  updateArrayComboItems();
  if (!arrays.isEmpty()) {
    updateComponentComboItems(0);
  }
}

void NestedSelectorWidget::setCurrentArrayIndex(int index) {
  if (index >= 0 && index < arrayCombo->count()) {
    arrayCombo->blockSignals(true);
    arrayCombo->setCurrentIndex(index);
    arrayCombo->blockSignals(false);
    updateComponentComboItems(index);
  }
}

void NestedSelectorWidget::setCurrentComponentIndex(int vtkComponentIndex) {
  int arrayIndex = arrayCombo->currentIndex();
  if (arrayIndex >= 0 && arrayIndex < arrays.size()) {
    const auto &array = arrays[arrayIndex];
    int comboIndex = VtuModelLoader::vtkIndexToComboIndex(array, vtkComponentIndex);
    if (comboIndex >= 0 && comboIndex < componentCombo->count()) {
      componentCombo->blockSignals(true);
      componentCombo->setCurrentIndex(comboIndex);
      componentCombo->blockSignals(false);
    }
  }
}

void NestedSelectorWidget::onArrayIndexChanged(int index) {
  if (index >= 0 && index < arrays.size()) {
    // Update component combobox for the new array
    updateComponentComboItems(index);
    // Reset to first component (Magnitude for multi-component, or component 0
    const auto &array = arrays[index];
    int initialVtkIndex = VtuModelLoader::hasMagnitudeOption(array) ? -1 : 0;
    setCurrentComponentIndex(initialVtkIndex);
    emit selectionChanged(index, initialVtkIndex);
  }
}

void NestedSelectorWidget::onComponentIndexChanged(int comboIndex) {
  int arrayIndex = arrayCombo->currentIndex();
  if (arrayIndex >= 0 && arrayIndex < arrays.size()) {
    const auto &array = arrays[arrayIndex];
    int vtkComponentIndex = VtuModelLoader::comboIndexToVtkIndex(array, comboIndex);
    emit selectionChanged(arrayIndex, vtkComponentIndex);
  }
}

void NestedSelectorWidget::updateArrayComboItems() {
  if (arrayCombo == nullptr) {
    return;
  }
  arrayCombo->blockSignals(true);
  arrayCombo->clear();
  for (const NestedArrayInfo &array : arrays) {
    arrayCombo->addItem(array.name);
  }
  arrayCombo->blockSignals(false);
}

void NestedSelectorWidget::updateComponentComboItems(int arrayIndex) {
  if (componentCombo == nullptr) {
    return;
  }
  componentCombo->blockSignals(true);
  componentCombo->clear();
  if (arrayIndex < 0 || arrayIndex >= arrays.size()) {
    componentCombo->blockSignals(false);
    return;
  }
  for (const QString &component : arrays[arrayIndex].componentNames) {
    componentCombo->addItem(component);
  }
  componentCombo->blockSignals(false);
}
