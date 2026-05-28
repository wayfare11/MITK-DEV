#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QSplitter>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <QmitkRegisterClasses.h>
#include <QmitkRenderWindowWidget.h>
#include <QmitkStdMultiWidget.h>

#include <mitkException.h>
#include <mitkFileReaderRegistry.h>
#include <mitkInteractionSchemeSwitcher.h>
#include <mitkIOMimeTypes.h>
#include <mitkIOUtil.h>
#include <mitkItkImageIO.h>
#include <mitkRenderingManager.h>
#include <mitkStandaloneDataStorage.h>

#include <itkNrrdImageIO.h>
#include <itkNrrdImageIOFactory.h>

#include <iostream>
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>

namespace
{
QString DefaultLiverPath()
{
  return QString::fromWCharArray(
    L"C:\\Users\\WB-wangyu\\Desktop\\\u533b\u5b66\u5f71\u50cf\u6570\u636e\\liver.nrrd");
}

QString DefaultLungPath()
{
  return QString::fromWCharArray(
    L"C:\\Users\\WB-wangyu\\Desktop\\\u533b\u5b66\u5f71\u50cf\u6570\u636e\\lung1111111.nrrd");
}

std::string ToMitkPath(const QString& path)
{
  const QByteArray pathBytes = QFileInfo(path).absoluteFilePath().toLocal8Bit();
  return std::string(pathBytes.constData(), static_cast<size_t>(pathBytes.size()));
}

std::runtime_error MakeError(const QString& message)
{
  const QByteArray bytes = message.toLocal8Bit();
  return std::runtime_error(std::string(bytes.constData(), static_cast<size_t>(bytes.size())));
}

bool ContainsNonAscii(const QString& path)
{
  for (const auto ch : path)
  {
    if (ch.unicode() > 127)
    {
      return true;
    }
  }

  return false;
}

QString PrepareMitkLoadPath(const QString& imagePath, std::unique_ptr<QTemporaryDir>& tempDir)
{
  const QString absolutePath = QFileInfo(imagePath).absoluteFilePath();
  if (!ContainsNonAscii(absolutePath))
  {
    return absolutePath;
  }

  const QString tempTemplate = QDir(QCoreApplication::applicationDirPath()).filePath("mitk-load-XXXXXX");
  tempDir = std::make_unique<QTemporaryDir>(tempTemplate);
  if (!tempDir->isValid())
  {
    throw MakeError(QString("Could not create temporary MITK load directory: %1").arg(tempTemplate));
  }

  const QString tempPath = QDir(tempDir->path()).filePath(QFileInfo(absolutePath).fileName());
  if (!QFile::copy(absolutePath, tempPath))
  {
    throw MakeError(QString("Could not copy image to temporary ASCII path: %1").arg(tempPath));
  }

  return tempPath;
}

void RegisterNrrdImageIO()
{
  static std::unique_ptr<mitk::ItkImageIO> nrrdImageIO;
  if (!nrrdImageIO)
  {
    itk::NrrdImageIOFactory::RegisterOneFactory();
    nrrdImageIO = std::make_unique<mitk::ItkImageIO>(itk::NrrdImageIO::New().GetPointer());
  }
}

void PrintNrrdReadersForCheck(const QString& imagePath)
{
  std::unique_ptr<QTemporaryDir> tempDir;
  const QString loadPath = PrepareMitkLoadPath(imagePath, tempDir);

  std::cerr << "Prepared load path: " << ToMitkPath(loadPath) << std::endl;

  mitk::FileReaderRegistry registry;
  const auto mimeType = mitk::FileReaderRegistry::GetMimeTypeForFile(ToMitkPath(loadPath));
  auto references = mitk::FileReaderRegistry::GetReferences(mimeType);
  std::cerr << "NRRD reader references: " << references.size() << std::endl;

  auto readers = registry.GetReaders(mimeType);
  std::cerr << "NRRD readers: " << readers.size() << std::endl;
  for (auto* reader : readers)
  {
    reader->SetInput(ToMitkPath(loadPath));
    std::cerr << "  confidence=" << static_cast<int>(reader->GetConfidenceLevel())
              << std::endl;
  }
  registry.UngetReaders(readers);
}

void LoadImage(const QString& imagePath, mitk::StandaloneDataStorage* dataStorage)
{
  if (!QFileInfo::exists(imagePath))
  {
    throw MakeError(QString("File does not exist: %1").arg(imagePath));
  }

  try
  {
    std::unique_ptr<QTemporaryDir> tempDir;
    const QString loadPath = PrepareMitkLoadPath(imagePath, tempDir);
    auto nodes = mitk::IOUtil::Load(ToMitkPath(loadPath), *dataStorage);
    if (nodes.IsNull() || nodes->empty())
    {
      throw MakeError(QString("No data node was loaded: %1").arg(imagePath));
    }
  }
  catch (const mitk::Exception& e)
  {
    throw MakeError(QString("MITK failed to load %1\n%2").arg(imagePath, QString::fromLocal8Bit(e.what())));
  }
}

class ImagePane final : public QWidget
{
public:
  ImagePane(const QString& title, const QString& imagePath, const QString& widgetName, QWidget* parent = nullptr)
    : QWidget(parent),
      m_DataStorage(mitk::StandaloneDataStorage::New()),
      m_MultiWidget(new QmitkStdMultiWidget(this, Qt::WindowFlags(), widgetName))
  {
    SetupUi(title, imagePath);
    LoadImage(imagePath, m_DataStorage);
    InitializeViewer();
  }

  void ResetViews()
  {
    const auto nodes = m_DataStorage->GetAll();
    const auto geometry = m_DataStorage->ComputeBoundingGeometry3D(nodes);
    m_MultiWidget->InitializeViews(geometry.GetPointer(), true);
    m_MultiWidget->ResetCrosshair();
    m_MultiWidget->SetWidgetPlaneMode(mitk::InteractionSchemeSwitcher::MITKStandard);
    m_MultiWidget->SetCrosshairVisibility(true);
    m_MultiWidget->SetWidgetPlanesVisibility(true);

    for (const auto& [name, renderWindowWidget] : m_MultiWidget->GetRenderWindowWidgets())
    {
      if (renderWindowWidget)
      {
        renderWindowWidget->OnResetGeometry();
      }
    }

    m_MultiWidget->Fit();
    m_MultiWidget->ForceImmediateUpdateAll();
    m_MultiWidget->RequestUpdateAll();
  }

private:
  void SetupUi(const QString& title, const QString& imagePath)
  {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);

    auto* titleLabel = new QLabel(QString("%1 - %2").arg(title, QFileInfo(imagePath).fileName()), this);
    titleLabel->setFrameStyle(QFrame::NoFrame);
    layout->addWidget(titleLabel);
    layout->addWidget(m_MultiWidget, 1);
  }

  void InitializeViewer()
  {
    m_MultiWidget->SetDataStorage(m_DataStorage);
    m_MultiWidget->InitializeMultiWidget();
    m_MultiWidget->AddPlanesToDataStorage();
    ResetViews();
  }

  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  QmitkStdMultiWidget* m_MultiWidget;
};

QStringList ResolveImagePaths(const QStringList& arguments)
{
  QStringList paths;
  paths << (arguments.size() > 1 ? arguments.at(1) : DefaultLiverPath());
  paths << (arguments.size() > 2 ? arguments.at(2) : DefaultLungPath());
  return paths;
}

QMainWindow* CreateMainWindow(const QStringList& imagePaths)
{
  auto* window = new QMainWindow;
  auto* splitter = new QSplitter(Qt::Horizontal, window);
  std::vector<ImagePane*> panes;

  panes.push_back(new ImagePane("Liver", imagePaths.at(0), "liverMultiWidget", splitter));
  panes.push_back(new ImagePane("Lung", imagePaths.at(1), "lungMultiWidget", splitter));

  splitter->addWidget(panes.at(0));
  splitter->addWidget(panes.at(1));
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 1);

  window->setCentralWidget(splitter);
  window->setWindowTitle("MITK Dual Four-View Viewer");
  window->resize(1600, 900);

  QTimer::singleShot(0, window, [panes]()
  {
    for (auto* pane : panes)
      pane->ResetViews();
  });

  QTimer::singleShot(250, window, [panes]()
  {
    for (auto* pane : panes)
      pane->ResetViews();
  });

  return window;
}
}

int main(int argc, char* argv[])
{
  QApplication application(argc, argv);
  QApplication::setApplicationName("Dual MITK Four Views");

  QmitkRegisterClasses();
  RegisterNrrdImageIO();

  QStringList arguments = application.arguments();
  const bool checkLoadOnly = arguments.removeAll("--check-load") > 0;

  try
  {
    const QStringList imagePaths = ResolveImagePaths(arguments);

    if (checkLoadOnly)
    {
      PrintNrrdReadersForCheck(imagePaths.at(0));
      mitk::StandaloneDataStorage::Pointer liverStorage = mitk::StandaloneDataStorage::New();
      mitk::StandaloneDataStorage::Pointer lungStorage = mitk::StandaloneDataStorage::New();
      LoadImage(imagePaths.at(0), liverStorage);
      LoadImage(imagePaths.at(1), lungStorage);
      return 0;
    }

    QMainWindow* window = CreateMainWindow(imagePaths);
    window->show();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return application.exec();
  }
  catch (const std::exception& e)
  {
    if (checkLoadOnly)
    {
      std::cerr << e.what() << std::endl;
      return 2;
    }

    QMessageBox::critical(nullptr, "Load failed", QString::fromLocal8Bit(e.what()));
    return 2;
  }
}
