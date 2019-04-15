#include <jni.h>
#include <stdlib.h>
#include <SuperpoweredFrequencyDomain.h>
#include <AndroidIO/SuperpoweredAndroidAudioIO.h>
#include <SuperpoweredAdvancedAudioPlayer.h>
#include <SuperpoweredSimple.h>
#include <SuperpoweredCPU.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <fstream>
#include <SuperpoweredRecorder.h>

extern "C" {
#include "Other/Timer.h"
#include "SpeechProcessing.h"
}



static SuperpoweredFrequencyDomain *frequencyDomain;
static float  *inputBufferFloat, *data;
static const float BANDGAINS[20] = {0.1000001, 0.2000001, 0.3000001, 0.4000001, 0.5000001, 0.6000001, 0.7000001, 0.8000001, 0.9000001, 1.000001, 1.100001, 1.200001, 1.300001, 1.400001, 1.500001, 1.600001, 1.700001, 1.800001, 1.900001, 2.000001};

//Changes made by Tahsin Ahmed Chowdhury for integration

static float *left, *right, *output;
SuperpoweredAndroidAudioIO *audioIO;
SuperpoweredAdvancedAudioPlayer *audioPlayer;
VADNoiseReductionCompression *vadNoiseRedCom;
Settings *settings;

//Variables for Timing
static size_t numOfFrames;
struct timespec begin, end;
static float timeTaken = 0.0f;
static uint64_t *timeBuffer;

bool storeIOData = false;
bool outputEnabled;

const char *filePathIn, *filePathOut, *NoiseClassifierFile, *UserBandGainsFile,*AudioFileName;
FILE* fileIDIn, *fileIDOut;


// This is called periodically by the media server.
static bool audioProcessing(void * __unused clientdata, short int *audioInputOutput, int numberOfSamples, int __unused samplerate) {

    //start(timer);
    clock_gettime(CLOCK_MONOTONIC_RAW, &begin);

    SuperpoweredShortIntToFloat(audioInputOutput, inputBufferFloat, (unsigned int)numberOfSamples); // Converting the 16-bit integer samples to 32-bit floating point.
    SuperpoweredDeInterleave(inputBufferFloat, left, right, (unsigned int)numberOfSamples);

    if(storeIOData) {//Storing input audio
        fwrite(audioInputOutput, sizeof(short int), numberOfSamples*2, fileIDIn);
    }

    output = (float*)calloc(numberOfSamples, sizeof(float));
    doNoiseReduction_Compression_withVAD(vadNoiseRedCom, left, output, settings);
    SuperpoweredInterleave(output, output, inputBufferFloat, (unsigned int) numberOfSamples);
    SuperpoweredFloatToShortInt(inputBufferFloat, audioInputOutput, (unsigned int)numberOfSamples);

    if(storeIOData) {//Storing processed/output audio
        fwrite(audioInputOutput, sizeof(short int), numberOfSamples*2, fileIDOut);
    }

    if(output!=NULL){
        free(output);
        output = NULL;
    }

    //stop(timer);

    clock_gettime(CLOCK_MONOTONIC_RAW,&end);
    memmove(timeBuffer, timeBuffer+1, 2*(numOfFrames - 1)* sizeof(timeBuffer));
    timeBuffer[numOfFrames - 1] = (end.tv_sec - begin.tv_sec) * 1000000000LL + (end.tv_nsec - begin.tv_nsec);
    timeTaken += (float)(timeBuffer[numOfFrames - 1])/(numOfFrames*1000000.0) - (float)(timeBuffer[0])/(numOfFrames*1000000.0);

    return true;
}



// Main entry to Native Portion of the app, initalizes module variables
extern "C" JNIEXPORT void Java_com_superpowered_IntegratedApp_MainActivity_FrequencyDomain(JNIEnv *javaEnvironment,
                                                                                           jobject __unused obj,
                                                                                           jint samplerate,
                                                                                           jint buffersize,
                                                                                           jboolean storeAudioFlag,
                                                                                           jstring fileIn,
                                                                                           jstring fileOut,
                                                                                           jstring noiseClassifierFile,
                                                                                           jstring userBandGainsFile) {

    NoiseClassifierFile = javaEnvironment->GetStringUTFChars(noiseClassifierFile,JNI_FALSE); // hybrid noise classifier path
    UserBandGainsFile = javaEnvironment->GetStringUTFChars(userBandGainsFile,JNI_FALSE);
    vadNoiseRedCom = initVAD_NoiseReduction_Compression(settings, NoiseClassifierFile, UserBandGainsFile);
    //timer = newTimer();

    inputBufferFloat = (float *)malloc(buffersize * sizeof(float) * 2 + 128);
    left = (float *)malloc(buffersize * sizeof(float) + 128);
    right = (float *)malloc(buffersize * sizeof(float) + 128);
    data  = (float*)malloc(sizeof(float)*1600);

    storeIOData=storeAudioFlag;
    if (storeIOData) {
        filePathIn = javaEnvironment->GetStringUTFChars(fileIn,JNI_FALSE);
        filePathOut = javaEnvironment->GetStringUTFChars(fileOut,JNI_FALSE);
        fileIDIn = createWAV(filePathIn, samplerate, 2);
        fileIDOut = createWAV(filePathOut, samplerate, 2);
    }

    //calculates number of frames within the gui update interval to provide average processing time
    numOfFrames = (size_t)(samplerate*settings->guiUpdateInterval/buffersize);
    timeBuffer = (uint64_t *)calloc(numOfFrames, sizeof(uint64_t));
    timeTaken = 0.0f;

    SuperpoweredCPU::setSustainedPerformanceMode(true);
    audioIO = new SuperpoweredAndroidAudioIO(samplerate, buffersize, true, true, audioProcessing, javaEnvironment, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2); // Start audio input/output.
}

//####################################### Read File Changes - Start #################################################


static void playerEventCallback(void * __unused clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    switch (event) {
        case SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess: audioPlayer->play(false); break;
        case SuperpoweredAdvancedAudioPlayerEvent_LoadError: __android_log_print(ANDROID_LOG_DEBUG,
                                                                                 "NCAndroid",
                                                                                 "Open error: %s",
                                                                                 (char *)value); break;
        case SuperpoweredAdvancedAudioPlayerEvent_EOF: audioPlayer->togglePlayback(); break;
        default:;
    };
}

// This is called periodically by the media server.
static bool audioFileProcessing(void * __unused clientdata, short int *audioInputOutput, int numberOfSamples, int __unused samplerate) {

    //start(timer);
    clock_gettime(CLOCK_MONOTONIC_RAW, &begin);
    if (audioPlayer->process(inputBufferFloat, false, (unsigned int) numberOfSamples)) {
        //SuperpoweredShortIntToFloat(audioInputOutput, inputBufferFloat, (unsigned int)numberOfSamples); // Converting the 16-bit integer samples to 32-bit floating point.
        SuperpoweredFloatToShortInt(inputBufferFloat, audioInputOutput,
                                    (unsigned int) numberOfSamples);
        SuperpoweredDeInterleave(inputBufferFloat, left, right, (unsigned int) numberOfSamples);

        if (storeIOData) {//Storing input audio
            fwrite(audioInputOutput, sizeof(short int), numberOfSamples * 2, fileIDIn);
        }

        output = (float *) calloc(numberOfSamples, sizeof(float));
        doNoiseReduction_Compression_withVAD(vadNoiseRedCom, left, output, settings);
        SuperpoweredInterleave(output, output, inputBufferFloat, (unsigned int) numberOfSamples);
        SuperpoweredFloatToShortInt(inputBufferFloat, audioInputOutput,
                                    (unsigned int) numberOfSamples);

        if (storeIOData) {//Storing processed/output audio
            fwrite(audioInputOutput, sizeof(short int), numberOfSamples * 2, fileIDOut);
        }

        if (output != NULL) {
            free(output);
            output = NULL;
        }

        //stop(timer);

        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        memmove(timeBuffer, timeBuffer + 1, 2 * (numOfFrames - 1) * sizeof(timeBuffer));
        timeBuffer[numOfFrames - 1] =
                (end.tv_sec - begin.tv_sec) * 1000000000LL + (end.tv_nsec - begin.tv_nsec);
        timeTaken += (float) (timeBuffer[numOfFrames - 1]) / (numOfFrames * 1000000.0) -
                     (float) (timeBuffer[0]) / (numOfFrames * 1000000.0);

        return true;
    }else return false;
}




// Main entry to Native Portion of the app, initalizes module variables
extern "C" JNIEXPORT void Java_com_superpowered_IntegratedApp_MainActivity_ReadFile(JNIEnv *javaEnvironment,
                                                                                    jobject __unused obj,
                                                                                    jint samplerate,
                                                                                    jint buffersize,
                                                                                    jboolean storeAudioFlag,
                                                                                    jstring fileIn,
                                                                                    jstring fileOut,
                                                                                    jstring noiseClassifierFile,
                                                                                    jstring userBandGainsFile,
                                                                                    jstring audioFileName) {

    NoiseClassifierFile = javaEnvironment->GetStringUTFChars(noiseClassifierFile,JNI_FALSE); // hybrid noise classifier path
    UserBandGainsFile = javaEnvironment->GetStringUTFChars(userBandGainsFile,JNI_FALSE);
    vadNoiseRedCom = initVAD_NoiseReduction_Compression(settings, NoiseClassifierFile, UserBandGainsFile);
    //timer = newTimer();
    inputBufferFloat = (float *)malloc(buffersize * sizeof(float) * 2 + 128);
    left = (float *)malloc(buffersize * sizeof(float) + 128);
    right = (float *)malloc(buffersize * sizeof(float) + 128);

    storeIOData=storeAudioFlag;
    if (storeIOData) {
        filePathIn = javaEnvironment->GetStringUTFChars(fileIn,JNI_FALSE);
        filePathOut = javaEnvironment->GetStringUTFChars(fileOut,JNI_FALSE);
        fileIDIn = createWAV(filePathIn, samplerate, 2);
        fileIDOut = createWAV(filePathOut, samplerate, 2);
    }

    //calculates number of frames within the gui update interval to provide average processing time
    numOfFrames = (size_t)(samplerate*settings->guiUpdateInterval/buffersize);
    timeBuffer = (uint64_t *)calloc(numOfFrames, sizeof(uint64_t));
    timeTaken = 0.0f;

    SuperpoweredCPU::setSustainedPerformanceMode(true);
    outputEnabled = true;
    audioPlayer = new SuperpoweredAdvancedAudioPlayer(NULL, playerEventCallback, (unsigned int)samplerate, 0);
    audioIO = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioFileProcessing, javaEnvironment, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2); // Start audio input/output.
    AudioFileName = javaEnvironment->GetStringUTFChars(audioFileName, JNI_FALSE);
    audioPlayer->open(AudioFileName);
}



//####################################### Read File Changes - End #################################################


// ################################## Main Activity functions Start ###############################################
// Stops audio media and detroys variables from allocated memory
extern "C" JNIEXPORT
void Java_com_superpowered_IntegratedApp_MainActivity_StopAudio(JNIEnv* javaEnvironment, jobject __unused obj, jstring fileIn, jstring fileOut){

    if(inputBufferFloat != NULL){
        if(audioIO!= nullptr) {
            delete audioIO;
        }
        destroyVAD_NoiseReduction_Compression(&vadNoiseRedCom);
        free(inputBufferFloat);
        if(left!=NULL); {
            free(left);
            left=NULL;
        }
        if(right!=NULL); {
            free(right);
            right=NULL;
        }
        //destroy(&timer);
        inputBufferFloat = NULL;
        if(storeIOData) {
            closeWAV(fileIDIn);
            closeWAV(fileIDOut);
            javaEnvironment->ReleaseStringUTFChars(fileIn, filePathIn);
            javaEnvironment->ReleaseStringUTFChars(fileOut, filePathOut);
        }
    }
}

extern "C" JNIEXPORT void Java_com_superpowered_IntegratedApp_MainActivity_loadDataForReadFile(JNIEnv * __unused env,
                                                                                               jobject __unused instance) {

    data = (float *) malloc(sizeof(float) * 1600);
    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 40; j++) {
            data[40 * i + j] = 0;
        }
    }

}

// settings is loaded by MainActivity onCreate() method
extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_MainActivity_loadSettings(JNIEnv* __unused env, jobject __unused instance) {

    settings = newSettings();
}

// settings is destroyed by MainActivity onDestroy() method
extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_MainActivity_destroySettings(JNIEnv* __unused env, jobject __unused instance) {

    destroySettings(settings);
}

//Returns the Sampling Frequency to display in noise reduction settings view
extern "C" JNIEXPORT int
Java_com_superpowered_IntegratedApp_MainActivity_getFs(JNIEnv* __unused env, jobject __unused instance) {

    if (settings != NULL) {
        return settings->fs;
    } else {
        return 0;
    }
}

extern "C" JNIEXPORT int
Java_com_superpowered_IntegratedApp_MainActivity_getStepSize(JNIEnv* __unused env, jobject __unused instance) {

    if (settings != NULL) {
        return settings->stepSize;
    } else {
        return 0;
    }

}

//This returns gui update time in Miliseconds in main activity
extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_MainActivity_getGuiUpdateRate(JNIEnv* __unused env, jobject __unused instance) {

    if (settings != NULL ) {
        return (settings->guiUpdateInterval)*1000.0f;
    } else {
        return 0.0f;
    }

}

extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_MainActivity_updateSettingsAmplification(JNIEnv* __unused env,
                                                                             jobject __unused instance,
                                                                             jfloat ampValue) {
    if (settings != NULL ) {
        settings->amplification = ampValue;
    }
}

extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_MainActivity_noiseReductionOn(JNIEnv* __unused env, jobject __unused instance,
                                                                  jboolean on) {

    if (settings != NULL ) {
        settings->noiseReductionOutputType = on;
    }

}

extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_MainActivity_compressionOn(JNIEnv*  __unused env, jobject __unused instance,
                                                               jboolean on) {

    if (settings != NULL ) {
        settings->compressionOutputType = on;
    }

}

extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_MainActivity_setAudioPlay(JNIEnv* __unused env, jobject __unused instance, jint on) {

    if (settings != NULL ) {
        settings->playAudio = on;
    }

}

extern "C" JNIEXPORT int
Java_com_superpowered_IntegratedApp_MainActivity_getDetectedClass(JNIEnv* __unused env, jobject __unused instance) {

    if(settings != NULL){
        return settings->classLabel;
    } else {
        return -1;
    }
}

extern "C"
JNIEXPORT jfloatArray Java_com_superpowered_IntegratedApp_MainActivity_GetMelImage(JNIEnv * __unused env, jobject __unused instance){
    jfloatArray result;
    result = env->NewFloatArray(1600);

    if (vadNoiseRedCom!=NULL) {
        for (int i = 0; i < 40; i++) {
            for (int j = 0; j < 40; j++) {
                data[40 * i + j] = vadNoiseRedCom->melSpectrogram->melSpectrogramImage[i][j];
            }
        }
    }
    env->SetFloatArrayRegion(result, 0, 1600, data);
    return result;

}

extern "C"
JNIEXPORT void JNICALL Java_com_superpowered_IntegratedApp_MainActivity_setDetectedClass(JNIEnv* __unused env, jobject __unused instance, jfloat detectedClass) {
    if (settings!=NULL)
    {
        settings->classLabel = (int)round(detectedClass);
    }
}

extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_MainActivity_getdbPower(JNIEnv* __unused env,
                                                            jobject __unused instance) {

    if(settings != NULL){
        return settings->dbpower;
    } else {
        return 0.0f;
    }

}

extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_MainActivity_setQuietAdjustment(JNIEnv* __unused env,
                                                                    jobject __unused instance,
                                                                    jint quietAdjValue) {
    if (settings != NULL ) {
        settings->quiet = quietAdjValue;
    }
}

extern "C" JNIEXPORT int Java_com_superpowered_IntegratedApp_MainActivity_getClusterLabel(JNIEnv * __unused env, jobject __unused instance) {

    if(vadNoiseRedCom != NULL) {
        return getClusterLabel(vadNoiseRedCom);
    }
    else {
        return 0;
    }
}


extern "C" JNIEXPORT int Java_com_superpowered_IntegratedApp_MainActivity_getTotalClusters(JNIEnv * __unused env, jobject __unused instance) {

    if(vadNoiseRedCom != NULL) {
        return getTotalClusters(vadNoiseRedCom);
    }
    else {
        return 0;
    }
}

extern "C" JNIEXPORT int Java_com_superpowered_IntegratedApp_MainActivity_NoisySpeechVADClassDecisionBuffer(JNIEnv * __unused env,
                                                                                                            jobject __unused instance,
                                                                                                            jint VADLabel) {

    if(settings != NULL) {
        if (settings->VADSmoothBuffer == settings->VADBufferLength){ // buffer is full
            settings->VADSmoothBuffer = 0;
            VADLabel = VADLabel; // either "Noise" or "Quiet" VAD class
        }
        else {
            settings->VADSmoothBuffer = settings->VADSmoothBuffer + 1; // buffer is filling; increasing counter by one
            VADLabel = 2; // speech+Noise VAD class
        }
    }

}


extern "C" JNIEXPORT int Java_com_superpowered_IntegratedApp_MainActivity_setFinalVADLabel(JNIEnv * __unused env,
                                                                                           jobject __unused instance,
                                                                                           jint VADFinalClass) {

    if(settings != NULL) {
        settings->finalVADLabel = VADFinalClass;
    }

}

//Returns frames processing time to noise reduction settings view
extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_MainActivity_getExecutionTime(JNIEnv* __unused env,
                                                                  jobject  __unused instance) {

//    if(timer!=NULL){
//        return getMS(timer);
//    } else {
//        return 0.0f;
//    }
    return timeTaken;
}

// ################################## Noise Reduction functions Start ###############################################
//Returns frames processing time to noise reduction settings view
extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_getExecutionTime(JNIEnv* __unused env,
                                                                            jobject  __unused instance) {

//    if(timer!=NULL){
//        return getMS(timer);
//    } else {
//        return 0.0f;
//    }
    return timeTaken;
}


//Returns the Sampling Frequency to display in noise reduction settings view
extern "C" JNIEXPORT int
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_getFs(JNIEnv* __unused env, jobject __unused instance) {

    if (settings != NULL) {
        return settings->fs;
    } else {
        return 0;
    }

}


extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_getWindowSize(JNIEnv* __unused env,
                                                                         jobject __unused instance) {

    if (settings != NULL ) {
        return (1.0f*settings->frameSize/(1.0f*settings->fs))*1000.0f;
    } else {
        return 0.0f;
    }
}

extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_getOverlapSize(JNIEnv* __unused env,
                                                                          jobject __unused instance) {

    if (settings != NULL ) {
        return (1.0f*settings->stepSize/(1.0f*settings->fs))*1000.0f;
    } else {
        return 0.0f;
    }

}

extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_getNoiseUpdateRate(JNIEnv* __unused env,
                                                                              jobject __unused instance) {

    if (settings != NULL ) {
        return settings->noiseEstimateTime;
    } else {
        return 0.0f;
    }

}

extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_getGuiUpdateRate(JNIEnv* __unused env,
                                                                            jobject __unused instance) {

    if (settings != NULL ) {
        return settings->guiUpdateInterval;
    } else {
        return 0.0f;
    }

}


extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_getSPLCalibration(JNIEnv* __unused env,
                                                                             jobject __unused instance) {

    if (settings != NULL ) {
        return settings->calibration;
    } else {
        return 0.0f;
    }

}


extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_updateSettingsSPLCalibration(JNIEnv* __unused env,
                                                                                        jobject __unused instance,
                                                                                        jfloat calibration) {

    if (settings != NULL ) {
        settings->calibration = calibration;
    }

}

extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_updateSettingsGUIUpdateRate(JNIEnv* __unused env,
                                                                                       jobject __unused instance,
                                                                                       jfloat guiUpdateRate) {

    if (settings != NULL ) {
        settings->guiUpdateInterval = guiUpdateRate;
    }

}

extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_updateSettingsNoiseUpdateRate(
        JNIEnv* __unused  env, jobject __unused instance, jfloat noiseUpdateRate) {

    if (settings != NULL ) {
        settings->noiseEstimateTime = noiseUpdateRate;
    }

}

extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_getdbPower(JNIEnv* __unused env,
                                                                      jobject __unused instance) {

    if(settings != NULL){
        return settings->dbpower;
    } else {
        return 0.0f;
    }

}




extern "C" JNIEXPORT int
Java_com_superpowered_IntegratedApp_NoiseReductionSettings_getPlayAudioStatus(JNIEnv* __unused env,
                                                                              jobject __unused instance) {

    if (settings != NULL){
        return settings->playAudio;
    }
    else {
        return 0;
    }


}
//################################### Noise Classifier functions ######################################

extern "C" JNIEXPORT int
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getPlayAudioStatus(JNIEnv* __unused env,
                                                                                   jobject __unused instance) {

    if (settings != NULL){
        return settings->playAudio;
    }
    else {
        return 0;
    }
}


extern "C" JNIEXPORT int Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getClusterLabel(JNIEnv * __unused env, jobject __unused instance) {

    if(vadNoiseRedCom != NULL) {
        return getClusterLabel(vadNoiseRedCom);
    }
    else {
        return 0;
    }
}

extern "C" JNIEXPORT int Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getPreviousClusterLabel(JNIEnv * __unused env, jobject __unused instance) {

    if(vadNoiseRedCom != NULL) {
        return getPreviousClusterLabel(vadNoiseRedCom);
    }
    else {
        return 0;
    }
}

extern "C" JNIEXPORT int Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getTotalClusters(JNIEnv * __unused env, jobject __unused instance) {

    if(vadNoiseRedCom != NULL) {
        return getTotalClusters(vadNoiseRedCom);
    }
    else {
        return 0;
    }
}


extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_updateUserBandGains(JNIEnv* __unused env,
                                                                                    jobject __unused instance,
                                                                                    jint index,
                                                                                    jint data) {
    if (settings != NULL ) {
        settings->userBandGains[index] = BANDGAINS[data];
        // settings->userBandGains[index] = ceilf(data * 10) / 10;
    }
}

extern "C" JNIEXPORT jfloatArray
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getUserBandGains(JNIEnv* __unused env,
                                                                                 jobject __unused instance,
                                                                                 jint detectedClass) {
    jfloatArray dataArray;
    dataArray = env->NewFloatArray(5);
    if (settings != NULL ) {
        env->SetFloatArrayRegion(dataArray, 0, 5, settings->userBandGains);
        return dataArray;
    }
}




extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_setSaveData(JNIEnv* __unused env, jobject __unused instance,
                                                                            jboolean on) {

    if (settings != NULL ) {
        settings->saveData = on;
    }

}

extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_setHybridMode(JNIEnv* __unused env, jobject __unused instance,
                                                                              jboolean on) {

    if (settings != NULL ) {
        settings->hybridMode = on;
    }

}


extern "C" JNIEXPORT int
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getSaveData(JNIEnv* __unused env, jobject __unused instance) {

    if (settings != NULL ) {
        return settings->saveData;
    }else{
        return 0;
    }

}

extern "C" JNIEXPORT int
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getHybridMode(JNIEnv* __unused env, jobject __unused instance) {

    if (settings != NULL ) {
        return settings->hybridMode;
    }else{
        return 0;
    }

}

extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getGuiUpdateRate(JNIEnv* __unused env,
                                                                                 jobject __unused instance) {

    if (settings != NULL ) {
        return settings->guiUpdateInterval;
    } else {
        return 0.0f;
    }

}


extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getVigilance1(JNIEnv* __unused env,
                                                                              jobject __unused instance) {

    if (settings != NULL ) {
        return settings->vigilance1;
    } else {
        return 0.0f;
    }

}


extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_updateSettingVigilance1(JNIEnv* __unused env,
                                                                                        jobject __unused instance,
                                                                                        jfloat vigilance1) {

    if (settings != NULL ) {
        settings->vigilance1 = vigilance1;
    }

}

extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getVigilance2(JNIEnv* __unused env,
                                                                              jobject __unused instance) {

    if (settings != NULL ) {
        return settings->vigilance2;
    } else {
        return 0.0f;
    }

}


extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_updateSettingVigilance2(JNIEnv* __unused env,
                                                                                        jobject __unused instance,
                                                                                        jfloat vigilance2) {

    if (settings != NULL ) {
        settings->vigilance2 = vigilance2;
    }

}

extern "C" JNIEXPORT int
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getFeatAvgBuffVal(JNIEnv* __unused env,
                                                                                  jobject __unused instance) {

    if (settings != NULL ) {
        return settings->FeatAvgBufferLength;
    } else {
        return 0.0f;
    }

}


extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_updateSettingFeatAvgBuffVal(JNIEnv* __unused env,
                                                                                            jobject __unused instance,
                                                                                            jint FeatAvgBufferLength) {

    if (settings != NULL ) {
        settings->FeatAvgBufferLength = FeatAvgBufferLength;
    }

}


extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getNewClusterCreaBuffVal(JNIEnv* __unused env,
                                                                                         jobject __unused instance) {

    if (settings != NULL ) {
        return settings->NewClusterCreationBufferTime;
    } else {
        return 0.0f;
    }

}


extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_updateSettingNewClusterCreaBuffVal(JNIEnv* __unused env,
                                                                                                   jobject __unused instance,
                                                                                                   jfloat NewClusterCreationBufferSize) {

    if (settings != NULL ) {

        settings->NewClusterCreationBufferTime = floor((NewClusterCreationBufferSize * 1000)/ ((settings->frameSize/2) * settings->FeatAvgBufferLength)); //convert from seconds to integer size
    }

}



extern "C" JNIEXPORT float
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_getDecisionSmoothingBuffVal (JNIEnv* __unused env,
                                                                                             jobject __unused instance) {

    if (settings != NULL ) {
        return settings->DecisionSmoothingBufferTime;
    } else {
        return 0.0f;
    }

}


extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_NoiseClassificationSettings_updateSettingDecisionSmoothingBuffVal(JNIEnv* __unused env,
                                                                                                      jobject __unused instance,
                                                                                                      jfloat DecisionSmoothingBufferSize) {

    if (settings != NULL ) {
        settings->DecisionSmoothingBufferTime = DecisionSmoothingBufferSize ;
    }

}


// ################################## Compression functions Start ###############################################
extern "C" JNIEXPORT jfloatArray
Java_com_superpowered_IntegratedApp_CompressionSettings_getCompressionSettingsData(JNIEnv *env,
                                                                                   jobject __unused instance) {

    jfloatArray dataArray;
    dataArray = env->NewFloatArray(20);
    env->SetFloatArrayRegion(dataArray, 0, 20, dataIn);
    return dataArray;
}

extern "C" JNIEXPORT void
Java_com_superpowered_IntegratedApp_CompressionSettings_updateCompressionSettingsData(JNIEnv* __unused env,
                                                                                      jobject __unused instance,
                                                                                      jint index,
                                                                                      jint data) {
    dataIn[index] = (float)data;
    switch(index)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            dRC1_not_empty = (boolean_T) false;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            dRC2_not_empty = (boolean_T) false;
            break;

        case 8:
        case 9:
        case 10:
        case 11:
            dRC3_not_empty = (boolean_T) false;
            break;

        case 12:
        case 13:
        case 14:
        case 15:
            dRC4_not_empty = (boolean_T) false;
            break;

        case 16:
        case 17:
        case 18:
        case 19:
            dRC5_not_empty = (boolean_T) false;
            break;

        default:
            break;
    }

}




