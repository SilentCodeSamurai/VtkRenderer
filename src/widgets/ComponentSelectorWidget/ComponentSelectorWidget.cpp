#include "ComponentSelectorWidget.h"

ComponentSelectorWidget::ComponentSelectorWidget(QWidget *parent)
    : QWidget(parent), m_currentComponentCount(0), m_arrayLabelText("Array:"), m_componentLabelText("Component:"), m_autoSelectFirst(true)
{
    setupUi();
    setupConnections();
    updateEnableState();
}

void ComponentSelectorWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);

    m_groupBox = new QGroupBox("Data Selector", this);
    m_groupBox->setStyleSheet(
        "QGroupBox {"
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
    QVBoxLayout *groupLayout = new QVBoxLayout(m_groupBox);
    groupLayout->setSpacing(8);

    // Array row
    QHBoxLayout *arrayLayout = new QHBoxLayout();
    arrayLayout->setSpacing(8);

    m_arrayLabel = new QLabel(m_arrayLabelText, this);
    m_arrayLabel->setMinimumWidth(80);
    m_arrayLabel->setStyleSheet(
        "QLabel {"
        "   color: #8fb0cf;"
        "   font-weight: 600;"
        "}");

    m_arrayCombo = new QComboBox(this);
    m_arrayCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_arrayCombo->setStyleSheet(
        "QComboBox {"
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

    arrayLayout->addWidget(m_arrayLabel);
    arrayLayout->addWidget(m_arrayCombo);

    // Component row
    m_componentLayout = new QHBoxLayout();
    m_componentLayout->setSpacing(8);

    m_componentLabel = new QLabel(m_componentLabelText, this);
    m_componentLabel->setMinimumWidth(80);
    m_componentLabel->setStyleSheet(
        "QLabel {"
        "   color: #8fb0cf;"
        "   font-weight: 600;"
        "}");

    m_componentCombo = new QComboBox(this);
    m_componentCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_componentCombo->setStyleSheet(
        "QComboBox {"
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

    m_componentLayout->addWidget(m_componentLabel);
    m_componentLayout->addWidget(m_componentCombo);

    // Add to group
    groupLayout->addLayout(arrayLayout);
    groupLayout->addLayout(m_componentLayout);

    mainLayout->addWidget(m_groupBox);

    // Set size policy
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void ComponentSelectorWidget::setupConnections()
{
    connect(m_arrayCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ComponentSelectorWidget::onArrayIndexChanged);

    connect(m_componentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ComponentSelectorWidget::onComponentIndexChanged);
}

void ComponentSelectorWidget::onArrayIndexChanged(int index)
{
    updateComponentCombo(index);

    if (index >= 0 && index < m_arrays.size())
    {
        const auto &array = m_arrays[index];

        // Update component count
        if (m_currentComponentCount != array.componentCount)
        {
            m_currentComponentCount = array.componentCount;
        }

        // Auto-select first component if needed
        if (m_autoSelectFirst && m_componentCombo->count() > 0)
        {
            m_componentCombo->setCurrentIndex(0);
        }
    }

    emit selectionChanged(index, currentComponentIndex());
    updateEnableState();
}

void ComponentSelectorWidget::onComponentIndexChanged(int index)
{
    if (index >= 0 && index < m_componentCombo->count())
    {
        const int componentIdx = m_componentCombo->itemData(index).toInt();

        emit selectionChanged(currentArrayIndex(), componentIdx);
    }
}

void ComponentSelectorWidget::updateComponentCombo(int arrayIndex)
{
    m_componentCombo->clear();

    if (arrayIndex < 0 || arrayIndex >= m_arrays.size())
    {
        m_componentCombo->setEnabled(false);
        return;
    }

    const auto &array = m_arrays[arrayIndex];

    if (array.componentCount <= 0)
    {
        m_componentCombo->setEnabled(false);
        return;
    }

    // Add magnitude option for multi-component arrays
    if (array.componentCount > 1)
    {
        m_componentCombo->addItem("🌐 Magnitude (All)", -1);
    }

    // Add individual components
    for (int i = 0; i < array.componentCount; ++i)
    {
        QString displayName = (i < array.componentNames.size())
                                  ? array.componentNames[i]
                                  : QString("Component %1").arg(i);

        // Add icon based on component type
        if (array.componentCount == 3)
        {
            if (i == 0)
                displayName = "X Direction";
            else if (i == 1)
                displayName = "Y Direction";
            else if (i == 2)
                displayName = "Z Direction";
        }

        m_componentCombo->addItem(QString("📊 %1").arg(displayName), i);
    }

    m_componentCombo->setEnabled(true);
    m_currentComponentCount = array.componentCount;
}

void ComponentSelectorWidget::updateEnableState()
{
    const bool hasArrays = !m_arrays.isEmpty();
    m_arrayCombo->setEnabled(hasArrays);
    m_componentCombo->setEnabled(hasArrays && m_componentCombo->count() > 0);

    // Update group box title based on state
    if (hasArrays)
    {
        m_groupBox->setTitle(QString("Data Selector (%1 arrays)").arg(m_arrays.size()));
    }
    else
    {
        m_groupBox->setTitle("Data Selector (no data)");
    }
}

void ComponentSelectorWidget::setArrays(const QVector<ArrayInfo> &arrays)
{
    clearArrays();

    m_arrays = arrays;
    m_arrayCombo->clear();

    for (const auto &array : arrays)
    {
        m_arrayCombo->addItem(array.name);
    }

    if (m_autoSelectFirst && !arrays.isEmpty())
    {
        m_arrayCombo->setCurrentIndex(0);
    }

    updateEnableState();
}

void ComponentSelectorWidget::addArray(const ArrayInfo &array)
{
    m_arrays.append(array);
    m_arrayCombo->addItem(array.name);

    if (m_autoSelectFirst && m_arrays.size() == 1)
    {
        m_arrayCombo->setCurrentIndex(0);
    }

    updateEnableState();
}

void ComponentSelectorWidget::addArray(const QString &name, int componentCount)
{
    addArray(ArrayInfo(name, componentCount));
}

void ComponentSelectorWidget::clearArrays()
{
    m_arrays.clear();
    m_arrayCombo->clear();
    m_componentCombo->clear();
    m_currentComponentCount = 0;

    updateEnableState();
}

void ComponentSelectorWidget::setCurrentArray(int index)
{
    if (index >= 0 && index < m_arrayCombo->count())
    {
        m_arrayCombo->setCurrentIndex(index);
    }
}

void ComponentSelectorWidget::setCurrentComponent(int componentIndex)
{
    for (int i = 0; i < m_componentCombo->count(); ++i)
    {
        if (m_componentCombo->itemData(i).toInt() == componentIndex)
        {
            m_componentCombo->setCurrentIndex(i);
            break;
        }
    }
}

void ComponentSelectorWidget::selectArray(int index)
{
    setCurrentArray(index);
}

void ComponentSelectorWidget::selectComponent(int componentIndex)
{
    setCurrentComponent(componentIndex);
}

void ComponentSelectorWidget::selectMagnitude()
{
    setCurrentComponent(-1);
}

void ComponentSelectorWidget::clear()
{
    clearArrays();
}

void ComponentSelectorWidget::setArrayLabel(const QString &text)
{
    m_arrayLabelText = text;
    m_arrayLabel->setText(text);
}

void ComponentSelectorWidget::setComponentLabel(const QString &text)
{
    m_componentLabelText = text;
    m_componentLabel->setText(text);
}

void ComponentSelectorWidget::setEnabled(bool enabled)
{
    QWidget::setEnabled(enabled);
    m_groupBox->setEnabled(enabled);
    updateEnableState();
}

QString ComponentSelectorWidget::currentArrayName() const
{
    int idx = currentArrayIndex();
    return (idx >= 0 && idx < m_arrays.size()) ? m_arrays[idx].name : QString();
}

QString ComponentSelectorWidget::currentComponentName() const
{
    int idx = m_componentCombo->currentIndex();
    return (idx >= 0) ? m_componentCombo->itemText(idx) : QString();
}

// Fluent interface methods
ComponentSelectorWidget *ComponentSelectorWidget::withArrays(const QVector<ArrayInfo> &arrays)
{
    setArrays(arrays);
    return this;
}

ComponentSelectorWidget *ComponentSelectorWidget::withArray(const QString &name, int components)
{
    addArray(name, components);
    return this;
}

ComponentSelectorWidget *ComponentSelectorWidget::withLabels(const QString &arrayLabel, const QString &componentLabel)
{
    setArrayLabel(arrayLabel);
    setComponentLabel(componentLabel);
    return this;
}

ComponentSelectorWidget *ComponentSelectorWidget::withStyle(const QString &style)
{
    setStyleSheet(style);
    return this;
}
