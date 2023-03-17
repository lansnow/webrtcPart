#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "mmdeviceapi.h"
#include "endpointvolume.h"
#include "Audioclient.h"

#include "audiopolicy.h" // IAudioSessionManager 这个服务相关

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
    /*
        以下两个对象都是通过 pDevices->Activate获取的
    */
    IAudioSessionManager* pManger = NULL;    // session层的设置只针对当前应用
    IAudioEndpointVolume* pAudioVolume =nullptr; // 这个对象的设置是对所有的应用都生效
    IAudioClient* pAudioClient = nullptr; //该接口属于WASAPI，能够在音频应用跟音频引擎中创建音频流(共享模式)
    // 还可以在应用程序跟硬件缓冲之间创建音频流(独占模式)
    try {

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_ALL,__uuidof(IMMDeviceEnumerator),(void**)&pDeviceEnumerator);
        if(FAILED(hr)) throw "CoCreateInstance";


        // 以下这句是获取音频默认的输出设备
        // 这里的第一个参数，是会返回对应的采集设备跟播放设备的，即参数1的变化，返回的设备有可能是耳机，也
        // 可能是麦客风
        hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender,eMultimedia,&pDevices);
        if(FAILED(hr)) throw "GetDefaultAudioEndpoint";


        /*以下是WASSIP相关测试*/

        // 从设备中获取IAudioSessionManager对象
        hr = pDevices->Activate(__uuidof(IAudioSessionManager),CLSCTX_ALL,nullptr,(void**)&pManger);
        if(FAILED(hr)) throw "pDevice->Active IAudioSessionManager";
        /*
            以下代码是对IAudioSessionManager对象进行的操作
        */
        // 这部分其实是webrtc里的 InitSpeaker操作，返回一个WASAPI的操作句柄
        ISimpleAudioVolume* _ptrRenderSimpleVolue=NULL; //这个对象是用来改变声音信息的，使用提基于WASAPI那个服务进行的
        // 上面那行的目的是为了在外部能够改变声音大小，如果设备是麦，那就是采集的声音大小
        // 如果是音响设置，则可以调用播放声音的大小
        int ret = pManger->GetSimpleAudioVolume(NULL,false,&_ptrRenderSimpleVolue);
        if(ret!=0 || _ptrRenderSimpleVolue==NULL){
            qDebug()<<"fail";

        }else{
            qDebug()<<"can do next";
        }

        // 外放声音里，stereo 文体声 mono 单声道
        // 根据声道的不同，需要对缓存buffer进行大小调整
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
        /*
            IAudioClient几个重要的API：
            Initialize：初始化，判断使用模式、驱动数据方式（一般事件方式）、访问周期、buffer大小。这里的参数就是一些配置
            GetBufferSize:获取缓冲区大小，
            SetEventHandle:这个是事件驱动是 必须的事件回调
            GetService: 拿到 扬声器跟采集 两个设备
        */

        float fVolume;

       hr = pAudioVolume->GetMasterVolumeLevelScalar(&fVolume);

       if(FAILED(hr)) throw "SetMasterVolumeLevelScalar";

       // pAudioClient->Release();
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
        WAVEFORMATEX* pWfoxOut=NULL;
        WAVEFORMAT Wfx; //这个是输出参数
        hr = pAudioClient->GetMixFormat(&pWfoxOut);
        qDebug()<<"hr:"<<hr;
        if(hr>=0){
            qDebug()<<"nChannels:"<<pWfoxOut->nChannels;
        }

        Wfx.wFormatTag = WAVE_FORMAT_PCM; //这里是具体的输出参数设置


       //float fVol = 0.7f;
       //pAudioVolume->SetMasterVolumeLevelScalar(fVol,NULL);
       //pAudioVolume->SetMute(false,NULL);

    } catch (...) {
       qDebug()<<"chick error";
    }

}
