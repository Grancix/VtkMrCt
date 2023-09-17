#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkImageReslice.h>
#include <vtkImageMapper3D.h>
#include <vtkImageActor.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>

#include <QApplication>
#include <QDockWidget>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QSlider>

#include <cmath>
#include <cstdlib>
#include <random>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkNamedColors.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

using namespace std;

int main(int argc, char* argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.resize(1200, 900);

    QDockWidget controlDock;
    mainWindow.addDockWidget(Qt::LeftDockWidgetArea, &controlDock);

    QLabel controlDockTitle("");
    controlDock.setTitleBarWidget(&controlDockTitle);

    QPointer<QVBoxLayout> dockLayout = new QVBoxLayout();
    QWidget layoutContainer;
    layoutContainer.setLayout(dockLayout);
    controlDock.setWidget(&layoutContainer);

    QPointer<QVTKOpenGLNativeWidget> vtkRenderWidget = new QVTKOpenGLNativeWidget();
    mainWindow.setCentralWidget(vtkRenderWidget);

    vtkNew<vtkGenericOpenGLRenderWindow> window;
    vtkRenderWidget->setRenderWindow(window.Get());

    QSlider slider(Qt::Vertical);
    dockLayout->addWidget(&slider);

    slider.setMinimum(0);
    slider.setMaximum(147);
    slider.setValue(0);

    // File headers
    string input = "C:\\Users\\Grancu\\Desktop\\kulki\\mrct\\CT.rdata"; ///////////////change later
    ifstream stream(input + ".header");
    string line;

    std::ifstream fin(input, ios::in | ios::binary | ios::ate);

    int size = fin.tellg();
    cout << size;
    fin.seekg(0, ios::beg);

    char *buffer = new char[size];
    fin.read(buffer, size);
    fin.close();

    //Setup allocator for input file
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->SetDimensions(512, 512, 247);
    imageData->SetSpacing(0.447266, 0.447266, 0.625);
    imageData->SetOrigin(0, 0, 0);
    imageData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

    //Copy image data
    unsigned char* imageDataPointer = static_cast<unsigned char*>(imageData->GetScalarPointer());
    std::memcpy(imageDataPointer, buffer, size);

    //Mapping data
    vtkNew<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper;
    volumeMapper->SetInputData(imageData);

    //Volumes properties
    vtkNew<vtkVolumeProperty> volumeProperty;
    volumeProperty->SetInterpolationTypeToLinear();

    vtkNew<vtkVolume> volume;
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);

    vtkNew<vtkImageReslice> reslice;
    reslice->SetInputData(imageData);
    reslice->SetOutputDimensionality(2);
    reslice->SetResliceAxesDirectionCosines(1, 0, 0, 0, 1, 0, 0, 0, 1);
    reslice->SetInterpolationModeToLinear();

    reslice->SetResliceAxesOrigin(0, 0, 1);

    // Create a vtkLookupTable for color mapping
    vtkNew<vtkLookupTable> colorTable;
    colorTable->SetTableRange(0, 8000); // Adjust the range as needed
    colorTable->SetHueRange(0.0, 0.0);   // Adjust hue for desired color
    colorTable->SetSaturationRange(0, 0);
    colorTable->SetValueRange(0.0, 1.0);
    colorTable->Build();

    vtkNew<vtkImageMapToColors> colorMapper;
    colorMapper->SetInputConnection(reslice->GetOutputPort());
    colorMapper->SetLookupTable(colorTable.Get());

    // Create a vtkImageActor and set it up to display the slice with colors
    vtkNew<vtkImageActor> imageActor;
    imageActor->GetMapper()->SetInputConnection(colorMapper->GetOutputPort());

    vtkNew<vtkRenderer> renderer;
    window->AddRenderer(renderer);
    renderer->AddActor(imageActor);

    mainWindow.show();
    renderer->Render();

    QObject::connect(&slider, &QSlider::valueChanged, [&](int value) {
        reslice->SetResliceAxesOrigin(0, 0, value);
        renderer->GetRenderWindow()->Render();
        });

    return app.exec();
}
