#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "mmdeviceapi.h"
#include "endpointvolume.h"
#include "Audioclient.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    qDebug()<<"1";
    HRESULT hr;
    IMMDeviceEnumerator* pDeviceEnumerator=nullptr;
    IMMDevice* pDevices = nullptr;
    IAudioEndpointVolume* pAudioVolume =nullptr;
    IAudioClient* pAudioClient = nullptr;

    try {

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_ALL,__uuidof(IMMDeviceEnumerator),(void**)&pDeviceEnumerator);
        if(FAILED(hr)) throw "CoCreateInstance";
        hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender,eMultimedia,&pDevices);
        if(FAILED(hr)) throw "GetDefaultAudioEndpoint";


        hr = pDevices->Activate(__uuidof(IAudioEndpointVolume),CLSCTX_ALL,nullptr,(void**)&pAudioVolume);
        if(FAILED(hr)) throw "pDevice->Active";

        DWORD dwHwSupportMask = 0;
        // 下面这行代码的意思是：通过掩码检测设备支持什么东西硬件调整属性
        hr = pAudioVolume->QueryHardwareSupport(&dwHwSupportMask);
        if (dwHwSupportMask & ENDPOINT_HARDWARE_SUPPORT_VOLUME)
            qDebug()<<"hwmask "<<dwHwSupportMask;

        if (dwHwSupportMask & ENDPOINT_HARDWARE_SUPPORT_VOLUME){
            //检查硬件设备的支持参数，这部分代码好像的返回结果感觉有些不太对
            float flevelMinDB(0.0);
            float flevelMaxDB(0.0);
            float fvolumeIncremenetDB(0.0);
            pAudioVolume->GetVolumeRange(&flevelMinDB,&flevelMaxDB,&fvolumeIncremenetDB);

            qDebug()<<"min:max:volue["<<flevelMinDB<<"_"<<flevelMaxDB<<"_"<<fvolumeIncremenetDB<<"]";
        }

        UINT nChannelCount(0);
        //硬件支持的最大声道数？
        pAudioVolume->GetChannelCount(&nChannelCount);

        qDebug()<<"#channels:"<<nChannelCount;

        hr = pDevices->Activate(__uuidof(IAudioClient),CLSCTX_ALL,nullptr,(void**)&pAudioClient);
        if(FAILED(hr)) throw "pDevice->Active";

        float fVolume;

       hr = pAudioVolume->GetMasterVolumeLevelScalar(&fVolume);

       if(FAILED(hr)) throw "SetMasterVolumeLevelScalar";

       pAudioClient->Release();
       pAudioVolume->Release();
       pDevices->Release();
       pDeviceEnumerator->Release();

       int  intVolume = fVolume*100+1;
       if(fVolume>100)
       {
           fVolume =100;
       }

       qDebug()<<"get size:"<<intVolume;

       // 这两行代码，好像并没有执行起来，应该是第二个参数的原因

       float fVol = 0.7f;
       pAudioVolume->SetMasterVolumeLevelScalar(fVol,NULL);
       //pAudioVolume->SetMute(false,NULL);

    } catch (...) {
       qDebug()<<"chick error";
    }

}
