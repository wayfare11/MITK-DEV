#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"
#include "QmitkStdMultiWidget.h"
#include "mitkImage.h"
#include "mitkNodePredicateDataType.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkStandaloneDataStorage.h"
#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <itksys/SystemTools.hxx>
#include <mitkDataNode.h>
#include <mitkIOUtil.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkPlanarLine.h>
#include <usModuleRegistry.h>

// 槽函数声明
void OnDrawLineButtonClicked(mitk::StandaloneDataStorage::Pointer ds,
                             mitk::DataNode::Pointer selectedImageNode,
                             QComboBox *layerComboBox,
                             std::vector<mitk::DataNode::Pointer> &segmentationLayers,
                             std::map<mitk::DataNode::Pointer, std::vector<mitk::DataNode::Pointer>> &layerToLabelsMap);
void OnDrawCircleButtonClicked(
  mitk::StandaloneDataStorage::Pointer ds,
  mitk::DataNode::Pointer selectedImageNode,
  QComboBox *layerComboBox,
  std::vector<mitk::DataNode::Pointer> &segmentationLayers,
  std::map<mitk::DataNode::Pointer, std::vector<mitk::DataNode::Pointer>> &layerToLabelsMap);
void OnClearDrawingButtonClicked(mitk::StandaloneDataStorage::Pointer ds, mitk::DataNode::Pointer selectedImageNode);
void OnNewLayerButtonClicked(mitk::StandaloneDataStorage::Pointer ds,
                             QComboBox *layerComboBox,
                             std::vector<mitk::DataNode::Pointer> &segmentationLayers,
                             std::map<mitk::DataNode::Pointer, std::vector<mitk::DataNode::Pointer>> &layerToLabelsMap);
void OnLayerComboBoxIndexChanged(
  int index,
  mitk::StandaloneDataStorage::Pointer ds,
  std::vector<mitk::DataNode::Pointer> &segmentationLayers,
  QComboBox *labelComboBox,
  std::map<mitk::DataNode::Pointer, std::vector<mitk::DataNode::Pointer>> &layerToLabelsMap);
void OnCreateLabelButtonClicked(
  mitk::StandaloneDataStorage::Pointer ds,
  QComboBox *layerComboBox,
  std::vector<mitk::DataNode::Pointer> &segmentationLayers,
  QComboBox *labelComboBox,
  std::map<mitk::DataNode::Pointer, std::vector<mitk::DataNode::Pointer>> &layerToLabelsMap);

// 全局变量
int brushSize = 5;

class MyApplication : public QWidget
{
public:
  MyApplication(QWidget *parent = nullptr) : QWidget(parent)
  {
    setFixedSize(2400, 1600); // 设置窗口大小为2400x1600
    SetupWidgets();
  }

private:
  QComboBox *layerComboBox;
  QComboBox *labelComboBox;
  std::vector<mitk::DataNode::Pointer> segmentationLayers;
  std::map<mitk::DataNode::Pointer, std::vector<mitk::DataNode::Pointer>> layerToLabelsMap;

  void SetupWidgets()
  {
    // 注册类
    QmitkRegisterClasses();

    // 确保加载 MitkPlanarFigure 模块
    auto planarFigureModule = us::ModuleRegistry::GetModule("MitkPlanarFigure");
    if (!planarFigureModule)
    {
      std::cerr << "MitkPlanarFigure module not found." << std::endl;
      return;
    }

    // 创建顶层数据存储
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

    // 加载图像文件
    const char *filePath = "D:/WorkSpace/DataSet/01_medicaldecathlon/Task06_Lung/imagesTr/lung_001.nii.gz";
    mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(filePath, *ds);

    if (dataNodes->empty())
    {
      fprintf(stderr, "Could not open file %s \n\n", filePath);
      return;
    }

    // 获取加载的图像节点
    mitk::DataNode::Pointer imageNode = dataNodes->Begin()->Value();
    segmentationLayers.push_back(imageNode); // 将图像节点添加到分割图层列表

    // 创建顶层布局
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setMargin(0);
    vlayout->setSpacing(2);

    // 创建按钮并添加到布局中
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *drawLineButton = new QPushButton("Draw Line", this);
    buttonLayout->addWidget(drawLineButton);
    QPushButton *drawCircleButton = new QPushButton("Draw Circle", this);
    buttonLayout->addWidget(drawCircleButton);
    QPushButton *clearDrawingButton = new QPushButton("Clear Drawing", this); // 新的清空按钮
    buttonLayout->addWidget(clearDrawingButton);

    // 新建分割图层按钮和下拉框
    QPushButton *newLayerButton = new QPushButton("New Segmentation Layer", this);
    buttonLayout->addWidget(newLayerButton);
    layerComboBox = new QComboBox(this);
    buttonLayout->addWidget(layerComboBox);

    // 创建标签按钮和标签下拉框
    QPushButton *createLabelButton = new QPushButton("Create Label", this);
    buttonLayout->addWidget(createLabelButton);
    labelComboBox = new QComboBox(this);
    buttonLayout->addWidget(labelComboBox);

    vlayout->addLayout(buttonLayout);

    // 创建视图父窗口和水平布局
    QWidget *viewParent = new QWidget(this);
    vlayout->addWidget(viewParent);
    QHBoxLayout *hlayout = new QHBoxLayout(viewParent);
    hlayout->setMargin(0);

    // 创建和初始化 QmitkStdMultiWidget
    QmitkStdMultiWidget *multiWidget = new QmitkStdMultiWidget(viewParent);
    hlayout->addWidget(multiWidget);

    multiWidget->SetDataStorage(ds);
    multiWidget->InitializeMultiWidget();
    multiWidget->AddPlanesToDataStorage();

    // 连接按钮点击事件到槽函数
    QObject::connect(drawLineButton,
                     &QPushButton::clicked,
                     [=]()
                     { OnDrawLineButtonClicked(ds, imageNode, layerComboBox, segmentationLayers, layerToLabelsMap); });
    QObject::connect(
      drawCircleButton,
      &QPushButton::clicked,
      [=]() { OnDrawCircleButtonClicked(ds, imageNode, layerComboBox, segmentationLayers, layerToLabelsMap); });
    QObject::connect(clearDrawingButton, &QPushButton::clicked, [=]() { OnClearDrawingButtonClicked(ds, imageNode); });
    QObject::connect(newLayerButton,
                     &QPushButton::clicked,
                     [=]() { OnNewLayerButtonClicked(ds, layerComboBox, segmentationLayers, layerToLabelsMap); });

    // 连接下拉框选择事件到槽函数
    QObject::connect(layerComboBox,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [=](int index)
                     { OnLayerComboBoxIndexChanged(index, ds, segmentationLayers, labelComboBox, layerToLabelsMap); });

    // 连接创建标签按钮点击事件到槽函数
    QObject::connect(
      createLabelButton,
      &QPushButton::clicked,
      [=]() { OnCreateLabelButtonClicked(ds, layerComboBox, segmentationLayers, labelComboBox, layerToLabelsMap); });

    // 渲染管理器更新
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(ds);
  }
};

int main(int argc, char *argv[])
{
  QApplication qtapplication(argc, argv);

  MyApplication app;
  app.show();

  return qtapplication.exec();
}

// 槽函数实现
void OnDrawLineButtonClicked(mitk::StandaloneDataStorage::Pointer ds,
                             mitk::DataNode::Pointer selectedImageNode,
                             QComboBox *layerComboBox,
                             std::vector<mitk::DataNode::Pointer> &segmentationLayers,
                             std::map<mitk::DataNode::Pointer, std::vector<mitk::DataNode::Pointer>> &layerToLabelsMap)
{
  // 创建一个新的PlanarLine对象
  mitk::PlanarLine::Pointer line = mitk::PlanarLine::New();

  // 创建一个新的DataNode并设置其数据为PlanarLine对象
  mitk::DataNode::Pointer lineNode = mitk::DataNode::New();
  lineNode->SetData(line);
  lineNode->SetName("Line");

  // 设置显示属性
  lineNode->SetProperty("planarfigure.default.line.width", mitk::FloatProperty::New(brushSize));
  lineNode->SetProperty("planarfigure.drawcontrolpoints", mitk::BoolProperty::New(true));
  lineNode->SetProperty("planarfigure.drawname", mitk::BoolProperty::New(false)); // 隐藏 "Line" 标签
  lineNode->SetProperty("planarfigure.drawquantities", mitk::BoolProperty::New(true));

  // 将线节点添加到当前选择的图层
  int currentLayerIndex = layerComboBox->currentIndex();
  if (currentLayerIndex >= 0 && currentLayerIndex < segmentationLayers.size())
  {
    mitk::DataNode::Pointer currentLayerNode = segmentationLayers[currentLayerIndex];
    mitk::StandaloneDataStorage::SetOfObjects::Pointer parents = mitk::StandaloneDataStorage::SetOfObjects::New();
    parents->InsertElement(0, currentLayerNode);
    ds->Add(lineNode, parents);

    // 将新创建的线条节点添加到对应图层的标签列表中
    layerToLabelsMap[currentLayerNode].push_back(lineNode);
  }
  else
  {
    ds->Add(lineNode);
  }

  // 创建并设置交互器
  mitk::PlanarFigureInteractor::Pointer interactor = mitk::PlanarFigureInteractor::New();
  auto planarFigureModule = us::ModuleRegistry::GetModule("MitkPlanarFigure");

  interactor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule);
  interactor->SetEventConfig("PlanarFigureConfig.xml", planarFigureModule);
  interactor->SetDataNode(lineNode);

  // 更新渲染
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void OnDrawCircleButtonClicked(
  mitk::StandaloneDataStorage::Pointer ds,
  mitk::DataNode::Pointer selectedImageNode,
  QComboBox *layerComboBox,
  std::vector<mitk::DataNode::Pointer> &segmentationLayers,
  std::map<mitk::DataNode::Pointer, std::vector<mitk::DataNode::Pointer>> &layerToLabelsMap)
{
  // 创建一个新的PlanarCircle对象
  mitk::PlanarCircle::Pointer circle = mitk::PlanarCircle::New();

  // 创建一个新的DataNode并设置其数据为PlanarCircle对象
  mitk::DataNode::Pointer circleNode = mitk::DataNode::New();
  circleNode->SetData(circle);
  circleNode->SetName("Circle");

  // 设置显示属性
  circleNode->SetProperty("planarfigure.default.line.width", mitk::FloatProperty::New(brushSize));
  circleNode->SetProperty("planarfigure.drawcontrolpoints", mitk::BoolProperty::New(true));
  circleNode->SetProperty("planarfigure.drawname", mitk::BoolProperty::New(false));      // 隐藏 "Circle" 标签
  circleNode->SetProperty("planarfigure.drawquantities", mitk::BoolProperty::New(true)); // 显示面积等信息

  // 将圆节点添加到当前选择的图层
  int currentLayerIndex = layerComboBox->currentIndex();
  if (currentLayerIndex >= 0 && currentLayerIndex < segmentationLayers.size())
  {
    mitk::DataNode::Pointer currentLayerNode = segmentationLayers[currentLayerIndex];
    mitk::StandaloneDataStorage::SetOfObjects::Pointer parents = mitk::StandaloneDataStorage::SetOfObjects::New();
    parents->InsertElement(0, currentLayerNode);
    ds->Add(circleNode, parents);

    // 将新创建的圆形节点添加到对应图层的标签列表中
    layerToLabelsMap[currentLayerNode].push_back(circleNode);
  }
  else
  {
    ds->Add(circleNode);
  }

  // 创建并设置交互器
  mitk::PlanarFigureInteractor::Pointer interactor = mitk::PlanarFigureInteractor::New();
  auto planarFigureModule = us::ModuleRegistry::GetModule("MitkPlanarFigure");

  interactor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule);
  interactor->SetEventConfig("PlanarFigureConfig.xml", planarFigureModule);
  interactor->SetDataNode(circleNode);

  // 更新渲染
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void OnClearDrawingButtonClicked(mitk::StandaloneDataStorage::Pointer ds, mitk::DataNode::Pointer selectedImageNode)
{
  // 获取所有PlanarFigure类型的节点
  mitk::StandaloneDataStorage::SetOfObjects::ConstPointer allNodes = ds->GetAll();
  for (auto it = allNodes->Begin(); it != allNodes->End(); ++it)
  {
    mitk::DataNode::Pointer node = it->Value();
    if (dynamic_cast<mitk::PlanarFigure *>(node->GetData()))
    {
      ds->Remove(node);
    }
  }

  // 更新渲染
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void OnNewLayerButtonClicked(mitk::StandaloneDataStorage::Pointer ds,
                             QComboBox *layerComboBox,
                             std::vector<mitk::DataNode::Pointer> &segmentationLayers,
                             std::map<mitk::DataNode::Pointer, std::vector<mitk::DataNode::Pointer>> &layerToLabelsMap)
{
  // 检查是否有图像节点
  if (segmentationLayers.empty())
  {
    std::cerr << "No image node available to get geometry from." << std::endl;
    return;
  }

  // 获取第一个图像节点的几何信息
  mitk::DataNode::Pointer firstImageNode = segmentationLayers.front();
  mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(firstImageNode->GetData());
  if (image.IsNull())
  {
    std::cerr << "First node is not an image." << std::endl;
    return;
  }

  mitk::BaseGeometry::Pointer geometry = image->GetGeometry();

  // 创建一个新的空图像作为分割图层
  mitk::Image::Pointer segmentation = mitk::Image::New();
  segmentation->Initialize(mitk::MakeScalarPixelType<unsigned char>(), *geometry);

  // 为新建的分割图层生成名称
  std::string layerName = "seg_" + std::to_string(segmentationLayers.size());

  // 创建一个新的DataNode并设置其数据为分割图层
  mitk::DataNode::Pointer segmentationNode = mitk::DataNode::New();
  segmentationNode->SetData(segmentation);
  segmentationNode->SetName(layerName);

  // 将分割图层节点添加到数据存储中
  ds->Add(segmentationNode);

  // 添加到分割图层列表并更新下拉框
  segmentationLayers.push_back(segmentationNode);
  layerComboBox->addItem(QString::fromStdString(layerName));

  // 初始化该图层的标签列表
  layerToLabelsMap[segmentationNode] = std::vector<mitk::DataNode::Pointer>();

  // 打印调试信息
  std::cout << "Added new segmentation layer: " << layerName << std::endl;
  std::cout << "Total segmentation layers: " << segmentationLayers.size() << std::endl;

  // 更新渲染
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void OnLayerComboBoxIndexChanged(
  int index,
  mitk::StandaloneDataStorage::Pointer ds,
  std::vector<mitk::DataNode::Pointer> &segmentationLayers,
  QComboBox *labelComboBox,
  std::map<mitk::DataNode::Pointer, std::vector<mitk::DataNode::Pointer>> &layerToLabelsMap)
{
  if (index >= 0 && index < segmentationLayers.size())
  {
    mitk::DataNode::Pointer selectedLayer = segmentationLayers[index];

    // 获取所有PlanarFigure类型的节点
    mitk::StandaloneDataStorage::SetOfObjects::ConstPointer allNodes = ds->GetAll();
    std::vector<mitk::DataNode::Pointer> planarFigureNodes;

    // 保存所有PlanarFigure节点
    for (auto it = allNodes->Begin(); it != allNodes->End(); ++it)
    {
      mitk::DataNode::Pointer node = it->Value();
      if (dynamic_cast<mitk::PlanarFigure *>(node->GetData()))
      {
        planarFigureNodes.push_back(node);
      }
    }

    // 清除数据存储中的所有PlanarFigure节点
    for (auto node : planarFigureNodes)
    {
      ds->Remove(node);
    }

    // 初始化视图
    mitk::RenderingManager::GetInstance()->InitializeViews(
      selectedLayer->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
    std::cout << "Reinitialized view for layer: " << selectedLayer->GetName() << std::endl;

    // 重新添加保存的PlanarFigure节点
    for (auto node : planarFigureNodes)
    {
      ds->Add(node);
    }

    // 更新标签下拉框
    labelComboBox->clear();
    if (layerToLabelsMap.find(selectedLayer) != layerToLabelsMap.end())
    {
      for (auto labelNode : layerToLabelsMap[selectedLayer])
      {
        ds->Add(labelNode);
        labelComboBox->addItem(QString::fromStdString(labelNode->GetName()));
      }
    }

    // 更新渲染
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void OnCreateLabelButtonClicked(
  mitk::StandaloneDataStorage::Pointer ds,
  QComboBox *layerComboBox,
  std::vector<mitk::DataNode::Pointer> &segmentationLayers,
  QComboBox *labelComboBox,
  std::map<mitk::DataNode::Pointer, std::vector<mitk::DataNode::Pointer>> &layerToLabelsMap)
{
  // 获取当前选择的图层
  int currentLayerIndex = layerComboBox->currentIndex();
  if (currentLayerIndex < 0 || currentLayerIndex >= segmentationLayers.size())
  {
    std::cerr << "No valid segmentation layer selected." << std::endl;
    return;
  }

  mitk::DataNode::Pointer currentLayerNode = segmentationLayers[currentLayerIndex];

  // 打开颜色选择对话框
  QColor color = QColorDialog::getColor(Qt::white, nullptr, "Select Label Color");
  if (!color.isValid())
  {
    std::cerr << "No color selected." << std::endl;
    return;
  }

  // 创建一个新的PlanarFigure对象作为标签
  mitk::PlanarCircle::Pointer label = mitk::PlanarCircle::New();

  // 创建一个新的DataNode并设置其数据为标签
  mitk::DataNode::Pointer labelNode = mitk::DataNode::New();
  labelNode->SetData(label);

  // 生成标签名称
  int labelCount = labelComboBox->count() + 1;
  std::string labelName = "label_" + std::to_string(labelCount);
  labelNode->SetName(labelName);

  // 设置标签颜色
  mitk::Color mitkColor;
  mitkColor.SetRed(color.redF());
  mitkColor.SetGreen(color.greenF());
  mitkColor.SetBlue(color.blueF());
  labelNode->SetColor(mitkColor);

  // 将标签节点添加到当前选择的图层
  mitk::StandaloneDataStorage::SetOfObjects::Pointer parents = mitk::StandaloneDataStorage::SetOfObjects::New();
  parents->InsertElement(0, currentLayerNode);
  ds->Add(labelNode, parents);

  // 添加到标签下拉框
  labelComboBox->addItem(QString::fromStdString(labelName));

  // 将新创建的标签节点添加到对应图层的标签列表中
  layerToLabelsMap[currentLayerNode].push_back(labelNode);

  // 创建并设置交互器
  mitk::PlanarFigureInteractor::Pointer interactor = mitk::PlanarFigureInteractor::New();
  auto planarFigureModule = us::ModuleRegistry::GetModule("MitkPlanarFigure");

  interactor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule);
  interactor->SetEventConfig("PlanarFigureConfig.xml", planarFigureModule);
  interactor->SetDataNode(labelNode);

  // 更新渲染
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
